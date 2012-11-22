/*
    This file is part of the TOBI SignalServer.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU General Public License Usage
    Alternatively, this file may be used under the terms of the GNU
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file gpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/gpl.html.

    In case of GNU General Public License Usage ,the TOBI SignalServer
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI SignalServer. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: SignalServer@tobi-project.org
*/

#include "signalserver/signalserver.h"

#include <fstream>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>

#include "tia/tia_server.h"
#include "tia/constants.h"
#include "tia/data_packet_interface.h"

#include "hardware/hw_access.h"
#include "config/xml_parser.h"
#include "config/xml_tags.h"

#include "tobiid/IDMessage.hpp"
#include "libtid/tid_server.h"

#include "filewriter/file_writer.h"



namespace tobiss
{
//-----------------------------------------------------------------------------

SignalServer::SignalServer(XMLParser& config_parser, bool use_new_tia)
  : hw_access_(0), tia_server_(0), tia_io_service_thread_(0),hw_access_io_service_thread_(0),
    config_parser_(config_parser),
    stop_reading_(false),
    master_blocksize_( 0 ),
    master_samplingrate_( 0 ),
    packet_(0), event_source_(0), file_writer_(0), write_file_(false), use_continous_saving_(0),
    last_block_nr_(0),
    tid_server_(0)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION <<  std::endl;
  #endif

  tia_server_ = new tia::TiAServer(tia_io_service_, use_new_tia);
  hw_access_ = new HWAccess(hw_access_io_service_, config_parser_);

  master_blocksize_ =  hw_access_->getMastersBlocksize();
  master_samplingrate_ = hw_access_->getMastersSamplingRate();

  tia_server_->setMasterBlocksize( master_blocksize_ );
  tia_server_->setMasterSamplingRate( master_samplingrate_ );
  tia_server_->setAcquiredSignalTypes( hw_access_->getAcquiredSignalTypes() );
  tia_server_->setBlockSizesPerSignalType( hw_access_->getBlockSizesPerSignalType() );

  sampling_rate_per_sig_type_ = hw_access_->getSamplingRatePerSignalType();
  tia_server_->setSamplingRatePerSignalType(sampling_rate_per_sig_type_);

  channels_per_sig_type_ = hw_access_->getChannelNames();
  tia_server_->setChannelNames(channels_per_sig_type_);

  subject_info_ = config_parser_ .parseSubject();
  server_settings_ = config_parser_.parseServerSettings();

  tia_server_->initialize(subject_info_, server_settings_);

  packet_ = tia_server_->getEmptyDataPacket();
  packet_->reset();

  tid_server_ = new TiD::TiDServer();
  tid_server_->bind ( boost::lexical_cast<unsigned int>(server_settings_[xmltags::tid_port]));
  tid_server_->reserveNrOfMsgs(2048);
  tid_server_->start();

  last_timestamp_ = boost::chrono::high_resolution_clock::now();
  current_timestamp_ = boost::chrono::high_resolution_clock::now();

  if(server_settings_[xmltags::store_data] == "1")
  {
    file_writer_ = new FileWriter( server_settings_[xmltags::filetype] );

    if(server_settings_.find(xmltags::filepath) != server_settings_.end())
      file_writer_->setFilepath( server_settings_[xmltags::filepath] );

    if(server_settings_.find(xmltags::append_to_filename) != server_settings_.end())
      file_writer_->setNewFileAppendString( server_settings_[xmltags::append_to_filename] );

    if(server_settings_.find(xmltags::file_exists) != server_settings_.end())
    {
      if( server_settings_[xmltags::file_exists] == xmltags::file_exists_new_file )
        file_writer_->setBehaviourIfFileExists( FileWriter::NewFile );
      else
        file_writer_->setBehaviourIfFileExists( FileWriter::OverWrite );

    }
    else
      file_writer_->setBehaviourIfFileExists( FileWriter::NewFile );

    std::map<uint32_t, std::vector<std::string> >::iterator it(channels_per_sig_type_.begin());
    tia::Constants cst;

    std::string label;
    for(uint32_t m = 0 ; it != channels_per_sig_type_.end(); it++, m++)
      for(uint32_t n = 0; n < it->second.size(); n++)
      {
        label = cst.getSignalName(it->first) + ":" + it->second[n];
        file_writer_->addNewChannel(label, FileWriterDataTypes::FLOAT,
                                    boost::numeric_cast<double>(sampling_rate_per_sig_type_[m]));
      }

    if(server_settings_.find(xmltags::continous_saving) != server_settings_.end())
    {
      write_file_ = true;
      use_continous_saving_ = true;

      file_writer_->setFilename( server_settings_[xmltags::filename] );
      file_writer_->open();
    }
  }

  hw_access_->startDataAcquisition();
  hw_access_io_service_thread_ = new boost::thread(boost::bind(&boost::asio::io_service::run,
                                                               &hw_access_io_service_));
  tia_io_service_thread_ = new boost::thread(boost::bind(&boost::asio::io_service::run,
                                                               &tia_io_service_));

  #ifdef WIN32
    SetPriorityClass(tia_io_service_thread_->native_handle(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(tia_io_service_thread_->native_handle(), THREAD_PRIORITY_TIME_CRITICAL );

    SetPriorityClass(hw_access_io_service_thread_->native_handle(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(hw_access_io_service_thread_->native_handle(), THREAD_PRIORITY_TIME_CRITICAL );
  #endif
}

//-----------------------------------------------------------------------------

SignalServer::~SignalServer()
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION <<  std::endl;
  #endif

  if( !stop_reading_ )
    stop();

  if(tia_io_service_thread_)
    delete(tia_io_service_thread_);

  if(tia_server_)
    delete(tia_server_);

  if(hw_access_io_service_thread_)
    delete(hw_access_io_service_thread_);

  if(hw_access_)
    delete(hw_access_);

  if(tid_server_)
    delete(tid_server_);
  if(file_writer_)
    delete file_writer_;
}

//-----------------------------------------------------------------------------

void SignalServer::stop()
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION <<  std::endl;
  #endif

  stop_reading_ = true;

  hw_access_io_service_.stop();
  hw_access_io_service_thread_->interrupt();
  hw_access_io_service_thread_->join();

  tia_io_service_.stop();
  tia_io_service_thread_->interrupt();
  tia_io_service_thread_->join();

  tid_server_->stop();

  hw_access_->stopDataAcquisition();
  write_file_ = 0;
  if(file_writer_)
    file_writer_->close();
}

//-----------------------------------------------------------------------------

void SignalServer::readPackets()
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION <<  std::endl;
  #endif

  std::vector<IDMessage> msgs;
  msgs.reserve(100);

  while (!stop_reading_)
  {
    last_timestamp_ = current_timestamp_;
    hw_access_->fillDataPacket(packet_);
    current_timestamp_ = boost::chrono::high_resolution_clock::now();

    tid_server_->update(packet_->getTimestamp(),packet_->getPacketID());

    tia_server_->sendDataPacket();

    if(tid_server_->newMessagesAvailable())
      tid_server_->getLastMessages(msgs);


    if(file_writer_)
      storeData(packet_, &msgs);

    msgs.clear();
  }
}

//-----------------------------------------------------------------------------

std::vector<std::string> SignalServer::getPossibleHardwareNames()
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  return(HWAccess::getPossibleHardwareNames());
}

//---------------------------------------------------------------------------------------------

void SignalServer::storeData(tia::DataPacket* packet, std::vector<IDMessage>* msgs)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  if(!use_continous_saving_)
    processStoreFileTiDMsgs(msgs);

  if(write_file_)
  {
    uint32_t nr_values = 0;
    uint32_t nr_blocks = 0;
    uint32_t nr_channels = 0;
    std::vector<double> v;
    uint32_t ch_start = 0;

    for(std::map<uint32_t, std::vector<std::string> >::iterator it(channels_per_sig_type_.begin());
      it != channels_per_sig_type_.end();  it++)
    {
      try
      {
        v = packet_->getSingleDataBlock(it->first);
        nr_values = packet_->getNrOfSamples(it->first);
        nr_channels = packet_->getNrOfChannels(it->first);
        nr_blocks = nr_values/nr_channels;

        for(uint32_t n = 0; n < nr_values/nr_blocks; n++)
          for(uint32_t m = 0; m < nr_blocks; m++)
            file_writer_->addSample<double>(ch_start + n, v[ (n*nr_blocks) + m]);

      }
      catch( std::exception& e )
      {
        std::cerr << "  Caught exception from File writer: " << e.what() << std::endl;
        //        throw;
        write_file_ = 0;
        file_writer_->close();
      }
      ch_start += it->second.size();
    }

    last_block_nr_ += master_blocksize_;

    std::vector<IDMessage> messages(*msgs);
    for(unsigned int n = 0; n < messages.size(); n++)
    {
      if(messages[n].GetFamilyType() == IDMessage::FamilyBiosig)
      {

        IDMessage* cur_msg = &(messages[n]);

        if(cur_msg->GetBlockIdx() != IDMessage::BlockIdxUnset)
          file_writer_->addEvent(cur_msg->GetBlockIdx()*master_blocksize_, cur_msg->GetEvent() );
        else
        {
          TCTimestamp ts(cur_msg->absolute);
          boost::chrono::high_resolution_clock::time_point msg_time(
                boost::chrono::seconds(ts.timestamp.tv_sec) +
                boost::chrono::microseconds(ts.timestamp.tv_usec) );

          boost::chrono::duration<double> diff = current_timestamp_ - last_timestamp_;
          boost::chrono::duration<double> sample_time = diff/double(master_blocksize_);

          boost::chrono::duration<double> ev_diff = current_timestamp_ - msg_time;
          boost::chrono::duration<double> shift_tmp = ev_diff/sample_time.count();

          unsigned int shift = round(shift_tmp.count());


          file_writer_->addEvent(last_block_nr_ - shift, cur_msg->GetEvent() );
        }
      }
    }
  }
}

//---------------------------------------------------------------------------------------------

void SignalServer::processStoreFileTiDMsgs(std::vector<IDMessage>* msgs)
{

  std::vector<IDMessage> messages(*msgs);
  IDMessage msg;

  for(std::vector<IDMessage>::iterator it(messages.begin()); it != messages.end(); it++ )
  {
    msg = *it;
    //std::cout << " -->  " << msg.GetDescription() << "; " << msg.GetFamily() << " : " << msg.GetFamilyType() << std::endl << std::flush;

    if( (msg.GetFamilyType() == IDMessage::FamilyCustom) && (msg.GetDescription() == "StopRecording") )
    {
      //std::cerr << "  *** StopRecMsg received!" << std::endl << std::flush;

      if(!file_writer_->isopen())
        std::cerr << "  *** Error: StopRecording Event received  --  not recording right now!" << std::endl;
      else
      {
        write_file_ = false;
        file_writer_->close();
      }
    }

    if( (msg.GetFamilyType() == IDMessage::FamilyCustom) && (msg.GetDescription() == "StartRecording") )
    {
      //std::cerr << "  ***  StartRecMsg received!" << std::endl << std::flush;

      if(file_writer_->isopen())
        std::cerr << "  *** Error: StartRecording Event received  --  already recording right now!" << std::endl;
      else
      {
        try
        {
          file_writer_->setFilename( server_settings_[xmltags::filename] );
          file_writer_->open();
          write_file_ = true;
          last_block_nr_ = 0;
        }
        catch( std::exception& e )
        {
          std::cerr << "  Error during opening -- caught exception from File writer: " << e.what() << std::endl;
          write_file_ = 0;
          file_writer_->close();
        }
      }
    }
  }
}

//---------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------









#ifdef OBSOLOLETE_CODE

void SignalServer::fustyReadPackets()
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION <<  std::endl;
  #endif

  #ifdef USE_TID_SERVER
    std::vector<IDMessage> msgs;
    msgs.reserve(100);
    std::ofstream events_file;
  #endif

  boost::uint64_t sample_count = 0;


  while (!stop_reading_)
  {
    sample_count += master_blocksize_;

    hw_access_->fillDataPacket(packet_);
    tia_server_->sendDataPacket();

//    #ifdef USE_TID_SERVER
    if(tid_server_->newMessagesAvailable())
      tid_server_->getLastMessages(msgs);
//    #endif

//    #ifdef USE_GDF_SAVER
    if(write_file_)
    {
      uint32_t nr_values = 0;
      uint32_t nr_blocks = 0;
      uint32_t nr_channels = 0;
      std::vector<double> v;
      uint32_t ch_start = 0;

      for(std::map<uint32_t, std::vector<std::string> >::iterator it(channels_per_sig_type_.begin());
        it != channels_per_sig_type_.end();  it++)
      {
        try
        {
          v = packet_->getSingleDataBlock(it->first);
          nr_values = packet_->getNrOfSamples(it->first);
          nr_channels = packet_->getNrOfChannels(it->first);
          nr_blocks = nr_values/nr_channels;

          for(uint32_t n = 0; n < nr_values/nr_blocks; n++)
            for(uint32_t m = 0; m < nr_blocks; m++)
            {
              gdf_writer_->addSamplePhys(ch_start + n, v[ (n*nr_blocks) + m]);
            }
        }
        catch( std::exception& e )
        {
          std::cerr << "  Caught exception from GDF writer: " << e.what() << std::endl;
          throw;
        }
        ch_start += it->second.size();
      }

//      #ifdef USE_TID_SERVER
      for(unsigned int n = 0; n < msgs.size(); n++)
      {
        if(msgs[n].GetFamilyType() == IDMessage::FamilyBiosig)
        {
          if(msgs[n].GetBlockIdx() >0)
          {
            std::cout << "Current sample count: " << sample_count << std::endl;
            gdf_writer_->addEvent(msgs[n].GetBlockIdx()*master_blocksize_, msgs[n].GetEvent() );
          }
          else
          {
            gdf_writer_->addEvent(sample_count, msgs[n].GetEvent() );
            std::cout << "Used BlockNr: " << sample_count << std::endl;
          }
          msgs[n].Dump();
          std::string timestamp;
          msgs[n].absolute.Get(&timestamp);
          std::cout << " + Timestamp: " << timestamp << std::endl;
        }
        else
        {
          if(!events_file.is_open())
          {
            std::cerr << "Received a 'non-gdf' event -- saving into .txt file 'events_file.txt'." << std::endl;
            events_file.open("events_file.txt");
            boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());
            events_file << std::endl << t << std::endl;
            events_file << "FamilyType:Family:Position:Event:Description:AbsTimestamp:RelTimeStamp" << std::endl;
          }
          msgs[n].Dump();
          events_file << msgs[n].GetFamilyType() << ":";
          events_file << msgs[n].GetFamily() << ":";
          events_file << msgs[n].GetBlockIdx()*master_blocksize_ << ":";
          events_file << msgs[n].GetEvent() << ":";
          events_file << msgs[n].GetDescription() << ":";
          events_file << "0" << ":";
          events_file << "0" << std::endl;
        }
      }
//      #endif
    }
//    #endif

//    #ifdef USE_TID_SERVER
      if(msgs.size())
        msgs.clear();
//    #endif
  }

//  #ifdef USE_TID_SERVER
    if(!events_file.is_open())
      events_file.close();
//  #endif

}

//-----------------------------------------------------------------------------
#ifdef USE_GDF_SAVER
void SignalServer::initGdf()
{
  std::map<uint32_t, std::vector<std::string> >::iterator it(channels_per_sig_type_.begin());

  for(uint32_t m = 0 ; it != channels_per_sig_type_.end(); it++, m++)
  {
    for(uint32_t n = 0; n < it->second.size(); n++)
    {
      int ind = gdf_writer_->getFirstFreeSignalIndex();
      gdf_writer_->createSignal(ind);
      gdf_writer_->getSignalHeader(ind).set_label( cst.getSignalName(it->first) + ":" + it->second[n] );
      gdf_writer_->getSignalHeader(ind).set_datatype( gdf::FLOAT32 );
      gdf_writer_->getSignalHeader(ind).set_samplerate(sampling_rate_per_sig_type_[m]);
      gdf_writer_->getSignalHeader(ind).set_digmin( -250000 );
      gdf_writer_->getSignalHeader(ind).set_digmax( 250000 );
      gdf_writer_->getSignalHeader(ind).set_physmin( -250000 );
      gdf_writer_->getSignalHeader(ind).set_physmax( 250000 );
    }
  }

  gdf_writer_->getHeaderAccess().setRecordDuration( 2* master_samplingrate_, 2*master_samplingrate_ );

  gdf_writer_->setEventMode( 1 );     // FIXME: 1 ... EventMode 1 (gdf)
  gdf_writer_->setEventSamplingRate( master_samplingrate_ );

  double systime = boost::numeric_cast<double>( time( NULL ) );
  double tmptime = ( systime/(3600.0*24.0) + 719529.0 ) * pow(2.0,32);
  gdf_writer_->getMainHeader( ).set_recording_start( boost::numeric_cast<gdf::uint64>(tmptime) );

  std::cout << "  ** Saving data to file: " << server_settings_[tia::Constants::ss_filename] << ".gdf" << std::endl;

  boost::filesystem::path p( server_settings_[tia::Constants::ss_filename] + ".gdf" );

  if(exists(p) && server_settings_[tia::Constants::ss_file_overwrite] == "1")
  {
    if(is_regular_file(p))
    {
      std::cout << "  **** WARNING **** Old file is overwritten now!" << std::endl;
      boost::filesystem3::remove(p);
    }
    else
      throw(std::invalid_argument( p.string() + "exists, but is not a regular file!"));
  }

  gdf_writer_->open( server_settings_[tia::Constants::ss_filename] + ".gdf" );
}
#endif

#endif

//-----------------------------------------------------------------------------

}  // tobiss


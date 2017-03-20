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
#include <boost/math/special_functions.hpp>

#include "tia/tia_server.h"
#include "tia/constants.h"
#include "tia/data_packet_interface.h"

#include "hardware/hw_access.h"
#include "config/xml_parser.h"
#include "config/xml_tags.h"

#include "tobiid/IDMessage.hpp"
#include "libtid/tid_server.h"

#include "tobiic/ICSerializerRapid.hpp"
#include "tobicore/TCException.hpp"

#include "filewriter/file_writer.h"

#include <sys/timeb.h>

namespace tobiss
{

static const int SOCKET_BUFFER_SIZE = 65536;

//-----------------------------------------------------------------------------

SignalServer::SignalServer(XMLParser& config_parser, bool use_new_tia)
  : hw_access_(0), tia_server_(0), tia_io_service_thread_(0),hw_access_io_service_thread_(0),
    config_parser_(config_parser),
    stop_reading_(false),
    master_blocksize_( 0 ),
    master_samplingrate_( 0 ),
    packet_(0), event_source_(0), file_writer_(0), write_file_(false), use_continous_saving_(false),
    last_block_nr_(0),
    tid_server_(0),
    tic_thread_(0), tic_socket_(0)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl << std::flush;
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


  // TiD
  if(server_settings_.find(xmltags::tid_use) != server_settings_.end())
  {
    if( config_parser.equalsYesOrNo( server_settings_[xmltags::tid_use]))
    {
      tid_server_ = new TiD::TiDServer();
      unsigned int port = boost::lexical_cast<unsigned int>(server_settings_[xmltags::tid_port]);
      tid_server_->bind ( port );
      tid_server_->reserveNrOfMsgs(2048);

      bool zero_delay = false;
      if(server_settings_.find(xmltags::tid_assume_zero_network_delay) != server_settings_.end())
        zero_delay = config_parser.equalsYesOrNo( server_settings_[xmltags::tid_assume_zero_network_delay]);

      tid_server_->assumeZeroNetworkDelay( zero_delay );
      tid_server_->start();

      std::cout << std::endl << " * TiD Server successfully listening on port " << port;
      if(zero_delay)
        std::cout << "  --  NOTICE: Assuming zero network delay! ";
      std::cout << std::endl;
    }
  }
  else
    tid_server_ = 0;
  //----------------------------


  // TiC
  if(server_settings_.find(xmltags::tic_use) != server_settings_.end())
  {
    if( config_parser.equalsYesOrNo( server_settings_[xmltags::tic_use]))
    {
      unsigned short port = boost::lexical_cast<unsigned int>(server_settings_[xmltags::tic_port]);
      std::string ip = server_settings_[xmltags::tic_ip];

      std::string classes = server_settings_[xmltags::tic_classifier];

      size_t del = classes.find_first_of(";");
      size_t pos = 0;
      while(del != std::string::npos)
      {
        std::string substr = classes.substr(0, del);
        classes.erase(0,del+1);

        pos = substr.find_first_of(":");
        tic_classes_.push_back( std::make_pair(substr.substr(0,pos), substr.substr(pos+1,std::string::npos)) );
        del = classes.find_first_of(";");
      }
      pos = classes.find_first_of(":");
      tic_classes_.push_back( std::make_pair(classes.substr(0,pos), classes.substr(pos+1,std::string::npos)) );

      tic_values_.resize(tic_classes_.size());

      tic_socket_ = new boost::asio::ip::tcp::socket(tic_io_service_);
      tic_thread_ = new boost::thread(boost::bind( &SignalServer::runTiCClient, this, ip, port));

      std::cout << std::endl << " * TiC client waiting to connect to " << ip << ":" << port;
    }
  }
  else
    tic_socket_ = 0;
  //----------------------------

  timeval tv;
  gettimeofday(&tv, NULL);

  last_timestamp_ = boost::chrono::system_clock::time_point(
                      boost::chrono::seconds(tv.tv_sec) + boost::chrono::microseconds(tv.tv_usec) );

  current_timestamp_ = last_timestamp_;

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

    double max_fs = 0;
    std::string label;
    for(uint32_t m = 0 ; it != channels_per_sig_type_.end(); it++, m++)
      for(uint32_t n = 0; n < it->second.size(); n++)
      {
        label = cst.getSignalName(it->first) + ":" + it->second[n];
        double fs = boost::numeric_cast<double>(sampling_rate_per_sig_type_[m]);
        file_writer_->addNewChannel(label, FileWriterDataTypes::FLOAT, fs);
        max_fs = std::max(max_fs, fs);
      }
    file_writer_->setEventSamplingRate(max_fs);

    for(uint32_t m = 0 ; m < tic_classes_.size(); m++)
    {
      label = tic_classes_[m].first + ":" + tic_classes_[m].second;
      file_writer_->addNewChannel(label, FileWriterDataTypes::FLOAT, max_fs);
    }

    if(server_settings_.find(xmltags::continous_saving) != server_settings_.end())
    {
      if( server_settings_[xmltags::continous_saving] == "1" )
      {
          write_file_ = true;
          use_continous_saving_ = true;

          file_writer_->setFilename( server_settings_[xmltags::filename] );
          file_writer_->open();
      }
    }

    std::cout << std::endl << " * Storing data " << std::endl;

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

  if(tid_server_)
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

    timeval tv;
    gettimeofday(&tv, NULL);
    current_timestamp_ = boost::chrono::system_clock::time_point(
                          boost::chrono::seconds(tv.tv_sec) + boost::chrono::microseconds(tv.tv_usec) );

    //current_timestamp_ = boost::chrono::system_clock::now();

    if(tid_server_)
    {
      tid_server_->update(packet_->getTimestamp(),packet_->getPacketID());

      if(tid_server_->newMessagesAvailable())
        tid_server_->getLastMessages(msgs);
    }

    tia_server_->sendDataPacket();

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
        v = packet->getSingleDataBlock(it->first);
        nr_values = packet->getNrOfSamples(it->first);
        nr_channels = packet->getNrOfChannels(it->first);
        nr_blocks = nr_values/nr_channels;

        for(uint32_t n = 0; n < nr_values/nr_blocks; n++)
          for(uint32_t m = 0; m < nr_blocks; m++)
            file_writer_->addSample<double>(ch_start + n, v[ (n*nr_blocks) + m]);

        tic_mutex_.lock();

        //std::cout << "Nr values: " << nr_values << ";  " << "Nr TiC msgs: " << tic_values_.size() << std::endl;

        for(uint32_t m = 0 ; m < tic_values_.size(); m++)
        {
          for(std::list<double>::iterator it = tic_values_[m].begin();
              it != tic_values_[m].end(); it++)
          {
            file_writer_->addSample<double>(nr_channels + m, *it );
          }
          tic_values_[m].clear();
        }
        tic_mutex_.unlock();
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
      if(messages[n].GetFamily() == IDMessage::TxtFamilyCustom)
      {

        IDMessage* cur_msg = &(messages[n]);

        if(cur_msg->GetBlockIdx() != IDMessage::BlockIdxUnset)
          file_writer_->addEvent(cur_msg->GetBlockIdx()*master_blocksize_, cur_msg->GetEvent() );
        else
        {
          TCTimestamp ts(cur_msg->absolute);
          boost::chrono::system_clock::time_point msg_time(
                boost::chrono::seconds(ts.timestamp.tv_sec) +
                boost::chrono::microseconds(ts.timestamp.tv_usec) );

          boost::chrono::duration<double> diff = current_timestamp_ - last_timestamp_;
          boost::chrono::duration<double> sample_time = diff/double(master_blocksize_);

          boost::chrono::duration<double> ev_diff = current_timestamp_ - msg_time;
          boost::chrono::duration<double> shift_tmp = ev_diff/sample_time.count();

          //cur_msg->Dump();
          //std::cout << ts.timestamp.tv_sec << std::endl << std::flush;
          //std::cout << ts.timestamp.tv_usec << std::endl << std::flush;
          //std::cout << msg_time << "; "<< std::endl << std::flush;
          //std::cout << current_timestamp_ << "; "<< std::endl << std::flush;
          //std::cout << diff << "; "<< std::endl << std::flush;
          //std::cout << sample_time << "; "<< std::endl << std::flush;
          //std::cout << ev_diff << "; "<< std::endl << std::flush;
          //std::cout << shift_tmp << "; "<< std::endl << std::flush;
          //std::cout << boost::math::round(shift_tmp.count())<< std::endl << std::flush;
          //std::cout << "------" << std::endl << std::flush;
          //std::cout << std::endl << std::flush;

          int shift = 0;

          try
          {
            shift = boost::numeric_cast<int>( boost::math::round(shift_tmp.count()) );
          }
          catch(boost::numeric::bad_numeric_cast& e)
          {
            std::cerr << BOOST_CURRENT_FUNCTION << std::endl;
            std::cerr << "  Error: " << e.what() << " -- Shift of: " << boost::math::round(shift_tmp.count()) << std::endl;
          }

          file_writer_->addEvent(last_block_nr_ - shift, cur_msg->GetEvent() );
          // file_writer_->addEvent(last_block_nr_, cur_msg->GetEvent() );
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

    if( (msg.GetFamily() == IDMessage::TxtFamilyCustom) && (msg.GetDescription() == "StopRecording") )
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

    if( (msg.GetFamily() == IDMessage::TxtFamilyCustom) && (msg.GetDescription() == "StartRecording") )
    {
      std::cerr << "  ***  StartRecMsg received!" << std::endl << std::flush;

      if(file_writer_->isopen())
        std::cerr << "  *** Error: StartRecording Event received  --  already recording right now!" << std::endl;
      else
      {
        try
        {
          file_writer_->setFilename( server_settings_[xmltags::filename] );
          file_writer_->open();
          tic_mutex_.lock();

          std::cerr << "  ***  Clearing TiC Values!" << std::endl << std::flush;
          for(unsigned int x = 0; x < tic_values_.size(); x++)
//            for(std::list<double>::iterator it = tic_values_[x].begin();
//                it != tic_values_[x].end(); it++)
            {
              tic_values_[x].clear();
            }

          write_file_ = true;

          std::cerr << "  ***  TiC Values cleared!" << std::endl << std::flush;

          tic_mutex_.unlock();
          last_block_nr_ = 0;
        }
        catch( std::exception& e )
        {
          std::cerr << "  Error during opening -- caught exception from File writer: " << e.what() << std::endl;
          tic_mutex_.lock();
          write_file_ = 0;
          for(unsigned int x = 0; x < tic_values_.size(); x++)
//            for(std::list<double>::iterator it = tic_values_[x].begin();
//                it != tic_values_[x].end(); it++)
            {
              tic_values_[x].clear();
            }

          tic_mutex_.unlock();
          file_writer_->close();
        }
      }
    }
  }
}

//---------------------------------------------------------------------------------------------

void SignalServer::runTiCClient(std::string ip, unsigned short port)
{
  boost::system::error_code ec;
  boost::asio::ip::tcp::endpoint peer(boost::asio::ip::address::from_string(ip),port );
  boost::asio::streambuf b;
  std::string delimiter("</tobiic>");
  ICSerializerRapid recv_serializer;
  ICMessage msg;

  std::string xml_str;
  std::string tmp_str;
  std::string str_buffer;

  bool running = 1;
  std::cout << "  ***  Trying to connect to TiC Server ..." << std::endl << std::flush;

  while(running)
  {

    tic_socket_->connect(peer, ec);
    if(ec)
    {
      //std::cerr << "TiCClient::connect -- " << ec.message() << std::endl;
      boost::this_thread::sleep(boost::posix_time::millisec(1));
      continue;
    }
    else
      std::cout << "  ***  TiC Client connected ..." << std::endl << std::flush;
    ec.clear();

    boost::asio::socket_base::send_buffer_size send_buffer_option(SOCKET_BUFFER_SIZE);
    tic_socket_->set_option(send_buffer_option);
    boost::asio::socket_base::receive_buffer_size recv_buffer_option(SOCKET_BUFFER_SIZE);
    tic_socket_->set_option(recv_buffer_option);
    boost::asio::ip::tcp::no_delay delay(true);
    tic_socket_->set_option(delay);
    boost::asio::socket_base::linger linger(false, 0);
    tic_socket_->set_option(linger);

    while(running)
    {
      std::istream is(&b);
      boost::asio::read_until(*tic_socket_, b, delimiter, ec);

      if(ec)
      {
        //        for(unsigned int x = 0; x < tic_values_.size(); x++)
        //        {
        //          for(std::list<double>::iterator it = tic_values_[x].begin();
        //              it != tic_values_[x].end(); it++)
        //          {
        //            std::cout << *it << ", ";
        //          }

        //          std::cout << std::endl;
        //        }

        tic_socket_->cancel();
        tic_socket_->close();
        ec.clear();
        std::cout << "  ***  Trying to connect to TiC Server ..." << std::endl << std::flush;
        break;
      }

      if(str_buffer.size())
      {
        std::getline(is, tmp_str);
        str_buffer.append(tmp_str);
      }
      else
        std::getline(is, str_buffer);

      size_t pos = str_buffer.find(delimiter);
      while(pos != std::string::npos)
      {
        xml_str = str_buffer.substr(0, pos+delimiter.size() );
        str_buffer.erase(0, pos+delimiter.size());

        recv_serializer.SetMessage(&msg);
        recv_serializer.Deserialize(&xml_str);

        double val = 0;
        for(unsigned int x = 0; x < tic_classes_.size(); x++)
        {
          try
          {
            val = msg.GetValue(tic_classes_[x].first, tic_classes_[x].second);
          }
          catch(TCException& e)
          {
            val = 0;
          }

          tic_mutex_.lock();
          if(write_file_)
            tic_values_[x].push_back(val);
          tic_mutex_.unlock();
        }

        pos = str_buffer.find(delimiter);
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


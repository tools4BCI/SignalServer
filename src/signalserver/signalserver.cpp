#include "signalserver/signalserver.h"

#include <fstream>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>

#include "tia/tia_server.h"
#include "hardware/hw_access.h"
#include "config/xml_parser.h"

#include <tobiid/IDMessage.hpp>
#include "TiDlib/tid_server.h"


#include "libgdf/GDF/Writer.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

SignalServer::SignalServer(HWAccess& hw_access, TiAServer& tia_server, XMLParser& config_parser)
  : hw_access_(hw_access), tia_server_(tia_server), config_parser_(config_parser),
    tid_server_(0), gdf_writer_(0), stop_reading_(false), write_file_(false),
    master_blocksize_( hw_access.getMastersBlocksize() ),
    master_samplingrate_( hw_access.getMastersSamplingRate() )
{

  tia_server_.setMasterBlocksize( master_blocksize_ );
  tia_server_.setMasterSamplingRate( master_samplingrate_ );
  tia_server_.setAcquiredSignalTypes(hw_access_.getAcquiredSignalTypes());
  tia_server_.setBlockSizesPerSignalType(hw_access_.getBlockSizesPerSignalType());

  sampling_rate_per_sig_type_ = hw_access_.getSamplingRatePerSignalType();
  tia_server_.setSamplingRatePerSignalType(sampling_rate_per_sig_type_);

  channels_per_sig_type_ = hw_access_.getChannelNames();
  tia_server_.setChannelNames(channels_per_sig_type_);

  subject_info_ = config_parser_.parseSubject();
  server_settings_ = config_parser_.parseServerSettings();
  tia_server_.initialize(subject_info_, server_settings_);

  tid_server_ = new TiD::TiDServer(io_);
  tid_server_->bind ( boost::lexical_cast<unsigned int>(server_settings_[Constants::ss_tid_port]));
  tid_server_->listen();
  io_service_thread_ = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_));

  try
  {
    if(server_settings_[Constants::ss_filetype] == "gdf")
    {
      gdf_writer_ = new gdf::Writer();
      initGdf();
      write_file_ = true;
    }
  }
  catch( std::exception &e )
  {
    std::cerr << "  -- Caught exception from GDF writer!"  << std::endl;
    throw;
  }
}

//-----------------------------------------------------------------------------

SignalServer::~SignalServer()
{
  stop_reading_ = true;


  io_.stop();
  io_service_thread_->interrupt();
  io_service_thread_->join();

  if(io_service_thread_)
    delete(io_service_thread_);

  if(tid_server_)
    delete(tid_server_);

  if(gdf_writer_)
  {
    gdf_writer_->close();
    delete gdf_writer_;
  }
}

//-----------------------------------------------------------------------------

void SignalServer::stop()
{
  stop_reading_ = true;
}

//-----------------------------------------------------------------------------

void SignalServer::readPackets()
{
  std::vector<IDMessage> msgs;
  msgs.reserve(100);
  boost::uint64_t sample_count = 0;
  std::ofstream events_file;

  while (!stop_reading_)
  {
    sample_count += master_blocksize_;

    DataPacket packet = hw_access_.getDataPacket();
    tia_server_.sendDataPacket(packet);

    if(tid_server_->newMessagesAvailable())
      tid_server_->getLastMessages(msgs);

    if(write_file_)
    {
      uint32_t nr_values = 0;
      uint32_t nr_blocks = 0;
      std::vector<double> v;
      uint32_t ch_start = 0;

      for(std::map<uint32_t, std::vector<std::string> >::iterator it(channels_per_sig_type_.begin());
        it != channels_per_sig_type_.end();  it++)
      {
        try
        {
          v = packet.getSingleDataBlock(it->first);
          nr_values = packet.getNrOfValues(it->first);
          nr_blocks = packet.getNrOfBlocks(it->first);

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
    }

    if(msgs.size())
      msgs.clear();
  }

  if(!events_file.is_open())
    events_file.close();
}

//-----------------------------------------------------------------------------

void SignalServer::initGdf()
{
  std::map<uint32_t, std::vector<std::string> >::iterator it(channels_per_sig_type_.begin());

  Constants cst;

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
  gdf_writer_->setEventSamplingRate( hw_access_.getMastersSamplingRate() );

  double systime = boost::numeric_cast<double>( time( NULL ) );
  double tmptime = ( systime/(3600.0*24.0) + 719529.0 ) * pow(2.0,32);
  gdf_writer_->getMainHeader( ).set_recording_start( boost::numeric_cast<gdf::uint64>(tmptime) );

  std::cout << "  ** Saving data to file: " << server_settings_[Constants::ss_filename] << ".gdf" << std::endl;

  boost::filesystem::path p( server_settings_[Constants::ss_filename] + ".gdf" );

  if(exists(p) && server_settings_[Constants::ss_file_overwrite] == "1")
  {
    if(is_regular_file(p))
    {
      std::cout << "  **** WARNING **** Old file is overwritten now!" << std::endl;
      boost::filesystem3::remove(p);
    }
    else
      throw(std::invalid_argument( p.string() + "exists, but is not a regular file!"));
  }

  gdf_writer_->open( server_settings_[Constants::ss_filename] + ".gdf" );
}

//-----------------------------------------------------------------------------

}  // tobiss


/**
* @file signal_server.cpp
*
* @brief
*
*
*
**/

// STL
#include <iostream>
#include <sstream>
#include <vector>

// Boost
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

// TICPP
#include "ticpp/ticpp.h"

// LibGdf
#include "libgdf/gdfwriter.h"

// local
#include "hardware/hw_access.h"
#include "network/control_connection_server.h"
#include "signalserver/signal_server.h"
#include "signalserver/ssmethods.h"
#include "network/tcp_data_server.h"
#include "network/udp_data_server.h"

#ifdef TIMING_TEST
  #include "LptTools/LptTools.h"
  #define LPT1  0
  #define LPT2  1
#endif

namespace tobiss
{
using boost::uint64_t;
using boost::uint32_t;
using boost::uint16_t;
using boost::int16_t;
using boost::int64_t;
using boost::lexical_cast;

const string CLIENT_XML_CONFIG = "new_client_config.xml";

//-----------------------------------------------------------------------------

SignalServer::SignalServer(boost::asio::io_service& io_service)
  : io_service_(io_service),
  config_(0),
  tcp_data_server_(0),
  udp_data_server_(0),
  control_connection_server_(0),
  write_file(0),
  gdf_writer_(0),
  timeout_(io_service_)
{
  #ifdef TIMING_TEST
    timestamp_ = boost::posix_time::microsec_clock::local_time();
    counter_ = 0;
    t_var_ = 0;
    lpt_flag_ = 0;

      if(!LptDriverInstall())
      {
        cerr << "Installing LptTools lpt driver failed (do you have access rights for the lpt-port?)." << endl;
        throw std::runtime_error("Error installing LptTools lpt driver!");
      }

      if(!LptInit())
      {
        cerr << "Initializing lpt driver failed (do you have access rights for the lpt-port?)." << endl;
        throw std::runtime_error("Error initializing lpt driver!");
      }
  #endif
}

//-----------------------------------------------------------------------------

SignalServer::~SignalServer()
{
//   delete config_;
  delete tcp_data_server_;
  delete udp_data_server_;
  delete control_connection_server_;

  if(gdf_writer_)
  {
    gdf_writer_->close();
    delete gdf_writer_;
  }

  #ifdef TIMING_TEST
    LptExit();
  #endif
}

//-----------------------------------------------------------------------------

void SignalServer::initialize(XMLParser* config)
{
  //Temporäre Auffüllung des Vektors
  vector<Constants::DataType> v2;
  for(int i = 0; i<17; i++)
  {
    v2.push_back(Constants::uint32_);
  }
  setDataType(v2);

  assert(config != 0);
  config_ = config;
  server_settings_ = config->parseServerSettings();
  uint16_t port = 0;

//   map<string,string>::iterator it(server_settings_.begin());

//   for( ; it != server_settings_.end(); it++)
//     cout << "First: " << it->first << ";  Second: " << it->second << endl;

  port = lexical_cast<uint16_t>(server_settings_[Constants::ss_ctl_port]);
  control_connection_server_ = new ControlConnectionServer(io_service_, *this);
  control_connection_server_->bind(port);
  tcp_data_server_ = new TCPDataServer(io_service_);

  port = lexical_cast<uint16_t>(server_settings_[Constants::ss_udp_port]);
  udp_data_server_ = new UDPDataServer(io_service_);
  udp_data_server_->setDestination(server_settings_[Constants::ss_udp_bc_addr], port);
  control_connection_server_->listen();

  try
  {
    if(server_settings_[Constants::ss_filetype] == "gdf")
    {
      gdf_writer_ =
        new GDFWriter( server_settings_[Constants::ss_filename] + ".gdf" );
      initGdf();
      write_file = true;
    }
  }
  catch( StrException &e )
  {
    cout << "Exception: " << e.msg << endl;
  }

}

//-----------------------------------------------------------------------------

void SignalServer::sendDataPacket(DataPacket& packet)
{
  tcp_data_server_->sendDataPacket(packet);
  udp_data_server_->sendDataPacket(packet);

  if(write_file)
  {
    uint32_t nr_values = 0;
    uint32_t nr_blocks = 0;
    vector<double> v;
    uint32_t ch_start = 0;

    for(map<uint32_t, vector<string> >::iterator it(channels_per_sig_type_.begin());
      it != channels_per_sig_type_.end();  it++)
    {
      if(gdf_writer_)
      {
//         cout << "Writing Samples for SigType: " << it->first << endl;
        try
        {
          v = packet.getSingleDataBlock(it->first);
          nr_values = packet.getNrOfValues(it->first);
          nr_blocks = packet.getNrOfBlocks(it->first);

//           cout << "Nr_Values: " << nr_values << ";  Nr_Blocks: " << nr_blocks << endl;

          for(uint32_t n = 0; n < nr_values/nr_blocks; n++)
            for(uint32_t m = 0; m < nr_blocks; m++)
            {
//               cout << "n: " << n << ";  m: " << m << endl;
              gdf_writer_->addSample(ch_start + n, v[ (n*nr_blocks) + m]);
            }

        }
        catch(std::invalid_argument& e)
        {

        }
        catch( StrException &e )
        {
          cout << "Exception: " << e.msg << endl;
          throw;
        }

      }

      ch_start += it->second.size();
    }
  }


  #ifdef TIMING_TEST
    timestamp_ = boost::posix_time::microsec_clock::local_time();
    int port_state = LptPortIn(LPT1,0);
    if(!lpt_flag_)
    {
      lpt_flag_ = 1;
      LptPortOut(LPT1, 0, port_state | 0x02);
    }
    else
    {
      lpt_flag_ = 0;
      LptPortOut(LPT1, 0, port_state & ~0x02);
    }
    counter_++;

    diff_ = timestamp_ - packet.getTimestamp();
    t_mean_ = (t_mean_ + diff_)/2;
    t_var_  = (t_var_ +
    ( (diff_.total_microseconds() - t_mean_.total_microseconds() )*
    (diff_.total_microseconds() - t_mean_.total_microseconds() )  ) )/2;

    if( (master_samplingrate_/master_blocksize_ < 1) ||
       (counter_%((master_samplingrate_/master_blocksize_) *2) == 0) )
    {
      cout << "Packet Nr.: " << counter_ << ";  ";
      cout << "Timing -- mean: " << t_mean_.total_microseconds() << " microsecs,  ";
      cout << "variance: " << t_var_ << " microsecs"<< endl;
    }

  #endif

}

//-----------------------------------------------------------------------------

void SignalServer::initGdf()
{

  std::map<uint32_t, std::vector<std::string> >::iterator it(channels_per_sig_type_.begin());

  for(uint32_t n = 0 ; it != channels_per_sig_type_.end(); it++)
    gdf_writer_->addSignal(GDF_FLOAT32_TYPE, fs_per_sig_type_[n++], it->second.size());


  gdf_writer_->forceRecordDuration( true, 1 , 1 );

  gdf_writer_->createHeader();

  it = channels_per_sig_type_.begin();
  for(uint32_t m = 0 ; it != channels_per_sig_type_.end(); it++)
  {
    for(uint32_t n = 0; n < it->second.size(); n++)
    {
      gdf_writer_->getSignalHeader( )->digmin[m]  = -1;
      gdf_writer_->getSignalHeader( )->digmax[m]  =  1;
      gdf_writer_->getSignalHeader( )->physmin[m] = -1;
      gdf_writer_->getSignalHeader( )->physmax[m] =  1;

      string label = cst_.getSignalName(it->first) + ":" + it->second[n];

      // FIXME: len ... better methods will be provided by libgdf
      uint16_t len = 0;
      for( ; (len != 15) || (len < label.size()); len++ )
        gdf_writer_->getSignalHeader( )->label[m][len] =  label.c_str()[len];

      gdf_writer_->getSignalHeader( )->label[m][len] =  0;

      m++;
    }
  }

  gdf_writer_->setEventMode( 1, master_samplingrate_ );     // FIXME: 1 ... EventMode 1 (gdf)

  gdf_writer_->open();

}

//-----------------------------------------------------------------------------

void SignalServer::setClientConfig(const std::string& config, bool& configOk)
{
  if(checkClientConfig(config))
  {
    ticpp::Document doc_;
    doc_.Parse(config);
    doc_.SaveFile(CLIENT_XML_CONFIG);

    cout << "XMLParser hat funktioniert" << endl;
    *config_ = XMLParser(CLIENT_XML_CONFIG);
    ss_methods_->setClientConfig(config_);
    configOk = true;
  }
  else
  {
    configOk = false;
  }
}

//-----------------------------------------------------------------------------

void SignalServer::getConfig(XMLParser& config)
{
  config = *config_;
}

//-----------------------------------------------------------------------------

void SignalServer::setConfig(XMLParser* config)
{
  config_ = config;
  server_settings_ = config_->parseServerSettings();
}

//-----------------------------------------------------------------------------

void SignalServer::setServerSettings()
{
  server_settings_ = config_->parseServerSettings();
}

//-----------------------------------------------------------------------------

bool SignalServer::checkClientConfig(const std::string& config)
{
  ticpp::Document doc_;
  doc_.Parse(config);
  doc_.SaveFile(CLIENT_XML_CONFIG);
  doc_.LoadFile(CLIENT_XML_CONFIG);

  ticpp::Iterator<ticpp::Element> conf(doc_.FirstChildElement(cst_.tobi, true));
  ticpp::Iterator<ticpp::Element> hw(conf->FirstChildElement(cst_.hardware, true));

  std::string hw_name = hw->GetAttribute("name");;
  int it = 0;
  it = cst_.isSupportedHardware(hw_name);
  if(!it)
  {
    cout << "ClientConfig: Hardware not supported" << endl;
    return false;
  }

  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(cst_.hw_ds, true));

  ticpp::Iterator<ticpp::Element> sr(ds->FirstChildElement(cst_.hw_fs, true));
  if(sr->GetText(false) == "")
  {
    cout << "ClientConfig: Samplingrate not available" << endl;
    return false;
  }
  ticpp::Iterator<ticpp::Element> bs(ds->FirstChildElement(cst_.hw_buffer, true));
  if(bs->GetText(false) == "")
  {
    cout << "ClientConfig: Blocksize not available" << endl;
    return false;
  }

  ticpp::Iterator<ticpp::Element> cs(hw->FirstChildElement(cst_.hw_cs, true));
  cs = cs->FirstChildElement(cst_.hw_sel, true);
  ticpp::Iterator<ticpp::Element> channels(cs->FirstChildElement(cst_.hw_cs_ch, true));

  //TODO weitere Ueberpruefungen

  return true;
}

//-----------------------------------------------------------------------------

void SignalServer::setTimeoutKeepAlive(boost::uint32_t seconds)
{
  sec_for_timeout_ = seconds;
  timeout_.expires_from_now(boost::posix_time::seconds(sec_for_timeout_));
  timeout_.async_wait(boost::bind(&SignalServer::handleTimeoutKeepAlive,this));
}

//-----------------------------------------------------------------------------

void SignalServer::handleTimeoutKeepAlive()
{
  control_connection_server_->checkAllKeepAlive();
  setTimeoutKeepAlive(sec_for_timeout_);
}

//-----------------------------------------------------------------------------

} // Namespace tobiss


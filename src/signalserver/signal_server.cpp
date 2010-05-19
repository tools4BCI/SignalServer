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

// local
#include "hardware/hw_access.h"
#include "network/control_connection_server.h"
#include "signalserver/signal_server.h"
#include "network/tcp_data_server.h"
#include "network/udp_data_server.h"

#ifdef TIMING_TEST
  #include "LptTools/LptTools.h"
  #define LPT1  0
  #define LPT2  1
#endif

using boost::uint64_t;
using boost::uint32_t;
using boost::uint16_t;
using boost::int16_t;
using boost::int64_t;
using boost::lexical_cast;

//-----------------------------------------------------------------------------

SignalServer::SignalServer(boost::asio::io_service& io_service)
  : io_service_(io_service),
  config_(0),
  tcp_data_server_(0),
  udp_data_server_(0),
  control_connection_server_(0)
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

  //     LptDetectPorts(tmp,addr,3);

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

  #ifdef TIMING_TEST
    LptExit();
  #endif
}

//-----------------------------------------------------------------------------

void SignalServer::initialize(XMLParser* config)
{
  assert(config != 0);
  config_ = config;
  server_settings_ = config->parseServerSettings();
  uint16_t port = 0;

  port = lexical_cast<uint16_t>(server_settings_[Constants::ss_ctl_port]);
  control_connection_server_ = new ControlConnectionServer(io_service_, *this);
  control_connection_server_->bind(port);
  tcp_data_server_ = new TCPDataServer(io_service_);

  port = lexical_cast<uint16_t>(server_settings_[Constants::ss_udp_port]);
  udp_data_server_ = new UDPDataServer(io_service_);
  udp_data_server_->setDestination(server_settings_[Constants::ss_udp_bc_addr], port);
  control_connection_server_->listen();
}

//-----------------------------------------------------------------------------

void SignalServer::sendDataPacket(DataPacket& packet)
{


  tcp_data_server_->sendDataPacket(packet);
  udp_data_server_->sendDataPacket(packet);

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

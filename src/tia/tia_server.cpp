/*
    This file is part of the TOBI Interface A (TiA) library.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file lgpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/lgpl.html.

    In case of GNU Lesser General Public License Usage ,the TiA library
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TiA library. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: TiA@tobi-project.org
*/

/**
* @file tia_server.cpp
**/

// STL
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

// Boost
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

// TICPP
#include "ticpp/ticpp.h"

// local
#include "tia/data_packet.h"
#include "tia-private/network/control_connection_server.h"
#include "tia/tia_server.h"
#include "tia-private/network/tcp_data_server.h"
#include "tia-private/network/udp_data_server.h"
#include "tia-private/newtia/server_impl/tia_server_state_server_impl.h"
#include "tia-private/newtia/network_impl/boost_tcp_server_socket_impl.h"

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
using namespace std;

//-----------------------------------------------------------------------------

TiAServer::TiAServer(boost::asio::io_service& io_service, bool new_tia)
  : io_service_(io_service),
//  config_(0),
  tcp_data_server_(0),
  udp_data_server_(0),
  control_connection_server_(0),
  server_state_server_(0),
  new_tia_ (new_tia),
  write_file(0)
{
  #ifdef TIMING_TEST
    timestamp_ = boost::posix_time::microsec_clock::local_time();
    counter_ = 0;
    t_max_last_ = boost::posix_time::time_duration (0, 0, 0);
    t_max_total_ = boost::posix_time::time_duration (0, 0, 0);
    t_min_last_ = boost::posix_time::time_duration (10, 0, 0);
    t_min_total_ = boost::posix_time::time_duration (10, 0, 0);
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

TiAServer::~TiAServer()
{
//   delete config_;
  delete tcp_data_server_;
  delete udp_data_server_;
  delete control_connection_server_;
  delete server_state_server_;

  #ifdef TIMING_TEST
    LptExit();
  #endif
}

//-----------------------------------------------------------------------------

void TiAServer::initialize(std::map<std::string,std::string>& subject_info,
                              std::map<std::string,std::string>& server_settings)
{
//  assert(config != 0);
//  config_ = config;
  server_settings_ = server_settings;
  uint16_t port = 0;

//   map<string,string>::iterator it(server_settings_.begin());

//   for( ; it != server_settings_.end(); it++)
//     cout << "First: " << it->first << ";  Second: " << it->second << endl;


  port = lexical_cast<uint16_t>(server_settings_[Constants::ss_ctl_port]);
  control_connection_server_ = new ControlConnectionServer(subject_info,
                                                           io_service_, *this);
  control_connection_server_->bind(port);
  tcp_data_server_ = new TCPDataServer(io_service_);

  port = lexical_cast<uint16_t>(server_settings_[Constants::ss_udp_port]);
  udp_data_server_ = new UDPDataServer(io_service_);
  udp_data_server_->setDestination(server_settings_[Constants::ss_udp_bc_addr], port);

  if (new_tia_)
  {
    boost::shared_ptr<tia::TCPServerSocket> server_state_server_socket (new tia::BoostTCPServerSocketImpl (io_service_));
    server_state_server_ = new tia::TiAServerStateServerImpl (server_state_server_socket);
  }
  control_connection_server_->listen();

}

//-----------------------------------------------------------------------------

void TiAServer::sendDataPacket(DataPacket& packet)
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
    t_diffs_.push_back (diff_);
    t_min_total_ = min (t_min_total_, diff_);
    t_max_total_ = max (t_max_total_, diff_);
    t_min_last_ = min (t_min_last_, diff_);
    t_max_last_ = max (t_max_last_, diff_);

    t_mean_ = (t_mean_ + diff_)/2;
    t_var_  = (t_var_ +
    ( (diff_.total_microseconds() - t_mean_.total_microseconds() )*
    (diff_.total_microseconds() - t_mean_.total_microseconds() )  ) )/2;

    if( (master_samplingrate_/master_blocksize_ < 1) ||
       (counter_%((master_samplingrate_/master_blocksize_) *2) == 0) )
    {
      sort (t_diffs_.begin(), t_diffs_.end());

      cout << "Packet Nr.: " << counter_ << ";  ";
      cout << "Timing (microsecs) -- mean: " << t_mean_.total_microseconds() << ", ";
      cout << "variance: " << t_var_;
      cout << ", min: " << t_min_last_.total_microseconds() << " (total: "<<  t_min_total_.total_microseconds() <<"), ";
      cout << "max: "<< t_max_last_.total_microseconds() << " (total: "<< t_max_total_.total_microseconds() << "), ";
      cout << "median: " << t_diffs_[t_diffs_.size() / 2].total_microseconds () << endl;
      t_diffs_.clear();
      t_min_last_ = boost::posix_time::time_duration (10, 0, 0);
      t_max_last_ = boost::posix_time::time_duration (0, 0, 0);
    }

  #endif

}

//-----------------------------------------------------------------------------


} // Namespace tobiss


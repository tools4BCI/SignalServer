/*
    This file is part of TOBI Interface A (TiA).

    TOBI Interface A (TiA) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TOBI Interface A (TiA) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TOBI Interface A (TiA).  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

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
#include <algorithm>

// Boost
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

// TICPP
#include "ticpp/ticpp.h"

// LibGdf
#ifdef WRITE_GDF
#include "libgdf/gdfwriter.h"
#endif

// local
#include "tia/data_packet.h"
#include "tia-private/network/control_connection_server.h"
#include "tia/signal_server.h"
#include "tia-private/network/tcp_data_server.h"
#include "tia-private/network/udp_data_server.h"

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

SignalServer::SignalServer(boost::asio::io_service& io_service)
  : io_service_(io_service),
//  config_(0),
  tcp_data_server_(0),
  udp_data_server_(0),
  control_connection_server_(0),
  write_file(0)
#ifdef WRITE_GDF
  ,gdf_writer_(0)
#endif
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

SignalServer::~SignalServer()
{
//   delete config_;
  delete tcp_data_server_;
  delete udp_data_server_;
  delete control_connection_server_;

#ifdef WRITE_GDF
  if(gdf_writer_)
  {
    gdf_writer_->close();
    delete gdf_writer_;
  }
#endif

  #ifdef TIMING_TEST
    LptExit();
  #endif
}

//-----------------------------------------------------------------------------

void SignalServer::initialize(std::map<std::string,std::string> subject_info,
                              std::map<std::string,std::string> server_settings)
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
  control_connection_server_->listen();

#ifdef WRITE_GDF
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
#endif

}

//-----------------------------------------------------------------------------

void SignalServer::sendDataPacket(DataPacket& packet)
{
  tcp_data_server_->sendDataPacket(packet);
  udp_data_server_->sendDataPacket(packet);

#ifdef WRITE_GDF
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
        catch(std::invalid_argument&)
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
#endif


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
#ifdef WRITE_GDF
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
#endif
//-----------------------------------------------------------------------------


} // Namespace tobiss


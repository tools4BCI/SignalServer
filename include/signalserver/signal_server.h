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
* @file server_server.h
*
* @brief
*
**/

#ifndef SIGNALSERVER_H
#define SIGNALSERVER_H

// STL
#include <map>

#ifdef WIN32
  #include	<conio.h>
  #include	<stdio.h>
#endif

// Boost
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>
#include <boost/utility.hpp>

// local
#include "definitions/constants.h"

// forward declarations
#ifdef WRITE_GDF
class GDFWriter;
#endif

namespace tobiss
{
// forward declarations
class ControlConnectionServer;
class TCPDataServer;
class UDPDataServer;
class XMLParser;
class DataPacket;

//-----------------------------------------------------------------------------

/**
* @class SignalServer
*
* @brief The core of the Signal Server
*
*
*
* @todo
*/
class SignalServer : boost::noncopyable
{
  friend class ControlConnectionServer;

    // Methods
    //-----------------------------------
  public:
    /**
    * @brief Constructor
    * \param the boost::asio::io_service to use
    */
    SignalServer(boost::asio::io_service& io_service);

    /**
     * @brief Destructor
     * @param[in]  packet
     * @throws
     */
    virtual ~SignalServer();

    /**
    * @brief TODO
    */
    void initialize(std::map<std::string,std::string> subject_info,
                    std::map<std::string,std::string> server_settings);

    /**
    * @brief Sends a DataPacket to the clients
    * @param[in]  packet
    * @throws
    */
    void sendDataPacket(DataPacket& packet);

    /**
     * @brief Sends a DataPacket to the clients
     * @param[in]  packet
     * @throws
     */
    void setMasterSamplingRate(boost::uint32_t value) { master_samplingrate_ = value; }

    /**
     * @brief Sends a DataPacket to the clients
     * @param[in]  packet
     * @throws
     */
    void setMasterBlocksize(boost::uint32_t value) { master_blocksize_ = value; }

    /**
     * @brief Sends a DataPacket to the clients
     * @param[in]  packet
     * @throws
     */
    void setAcquiredSignalTypes(const std::vector<boost::uint32_t>& sig_types)
      { sig_types_ = sig_types; }

    /**
     * @brief Sends a DataPacket to the clients
     * @param[in]  packet
     * @throws
     */
    void setBlockSizesPerSignalType(const std::vector<boost::uint16_t>& blocksizes)
      { blocksizes_ = blocksizes; }

    /**
     * @brief Sends a DataPacket to the clients
     * @param[in]  packet
     * @throws
     */
    void setSamplingRatePerSignalType(const std::vector<boost::uint32_t>& fs_per_sig_type)
      { fs_per_sig_type_ = fs_per_sig_type; }

    /**
     * @brief Sends a DataPacket to the clients
     * @param[in]  packet
     * @throws
     */
    void setChannelNames(const std::map<boost::uint32_t, std::vector<std::string> >& channels_per_sig_type)
      { channels_per_sig_type_ = channels_per_sig_type; }

  private:
    /**
     * @brief Sends a DataPacket to the clients
     * @param[in]  packet
     * @throws
     */
    void initGdf();

  private:
    boost::asio::io_service&            io_service_; ///<
//    XMLParser*                          config_; ///<
    std::map<std::string, std::string>  server_settings_; ///<
    TCPDataServer*                      tcp_data_server_; ///<
    UDPDataServer*                      udp_data_server_; ///<
    ControlConnectionServer*            control_connection_server_; ///<

    boost::uint32_t                                       master_blocksize_; ///<
    boost::uint32_t                                       master_samplingrate_; ///<
    std::vector<boost::uint32_t>                          sig_types_; ///<
    std::vector<boost::uint16_t>                          blocksizes_; ///<
    std::vector<boost::uint32_t>                          fs_per_sig_type_; ///<
    std::map<boost::uint32_t, std::vector<std::string> >  channels_per_sig_type_; ///<

    Constants                           cst_; ///<

    bool                                write_file; ///<
#ifdef WRITE_GDF
	GDFWriter*                          gdf_writer_; ///<
#endif

#ifdef TIMING_TEST
    boost::posix_time::ptime timestamp_;
    boost::posix_time::time_duration diff_;
    boost::posix_time::time_duration t_mean_;
    boost::posix_time::time_duration t_min_total_;
    boost::posix_time::time_duration t_max_total_;
    boost::posix_time::time_duration t_min_last_;
    boost::posix_time::time_duration t_max_last_;
    std::vector<boost::posix_time::time_duration> t_diffs_;
    boost::int64_t t_var_;
    boost::uint64_t counter_;

    bool  lpt_flag_;

#endif
};

} // Namespace tobiss

//-----------------------------------------------------------------------------

#endif // SIGNALSERVER_H

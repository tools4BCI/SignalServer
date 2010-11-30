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
* @file tia_server.h
* @brief
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
#include "tia/constants.h"

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
class TiAServer : boost::noncopyable
{
  friend class ControlConnectionServer;

    // Methods
    //-----------------------------------
  public:
    /**
    * @brief Constructor
    * \param the boost::asio::io_service to use
    */
    TiAServer(boost::asio::io_service& io_service);

    /**
     * @brief Destructor
     * @param[in]  packet
     */
    virtual ~TiAServer();

    /**
    * @brief Initialize the server
    * param[in] map<std::string,std::string> subject_info
    * param[in] map<std::string,std::string> server_settings
    *
    * The server needs to be initializedwith subject specific information (birthday,...) and
    * parameters for the server configuration (ctrl port, udp port, udp broadcast addr).
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
     * @brief Set the sampling rate of the master
     * @param[in]  value
     */
    void setMasterSamplingRate(boost::uint32_t value) { master_samplingrate_ = value; }

    /**
     * @brief Set the blocksize of the master
     * @param[in]  value
     */
    void setMasterBlocksize(boost::uint32_t value) { master_blocksize_ = value; }

    /**
     * @brief Set the acquired signal types
     * @param[in]  vector<uint32_t> types
     */
    void setAcquiredSignalTypes(const std::vector<boost::uint32_t>& sig_types)
      { sig_types_ = sig_types; }

    /**
     * @brief Set the blocksize for every signal type
     * @param[in]  vector<uint16_t> bs
     */
    void setBlockSizesPerSignalType(const std::vector<boost::uint16_t>& blocksizes)
      { blocksizes_ = blocksizes; }

    /**
     * @brief Set the sampling rate for every signal type
     * @param[in]  vector<uint32_t> fs
     */
    void setSamplingRatePerSignalType(const std::vector<boost::uint32_t>& fs_per_sig_type)
      { fs_per_sig_type_ = fs_per_sig_type; }

    /**
     * @brief Set the channel names for the channels of every signal type
     * @param[in]  map<uint32_t, vector<string> > names
     *
     * Every signal type has a defined number of channels; the vector< string > holds
     * the names for every channel assigned with the signal type.
     */
    void setChannelNames(const std::map<boost::uint32_t, std::vector<std::string> >& channels_per_sig_type)
      { channels_per_sig_type_ = channels_per_sig_type; }

  private:
    #ifdef WRITE_GDF
    /**
     * @brief Sends a DataPacket to the clients
     * @param[in]  packet
     */
    void initGdf();
    #endif

  private:
    boost::asio::io_service&            io_service_; ///<
    std::map<std::string, std::string>  server_settings_; ///< A map holding an identifier + value (e.g. port + number)
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

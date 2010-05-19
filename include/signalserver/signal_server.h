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


// forward declarations
class ControlConnectionServer;
class TCPDataServer;
class UDPDataServer;
class XMLParser;
class DataPacket;

//-----------------------------------------------------------------------------

/*
 * @
 */
class SignalServer
{
  friend class ControlConnectionServer;

    // Methods
    //-----------------------------------
  public:
    //@
    SignalServer(boost::asio::io_service& io_service);

    //@
    virtual ~SignalServer();

    void initialize(XMLParser* config);

    void sendDataPacket(DataPacket& packet);

    void setMasterSamplingRate(boost::uint32_t value) { master_samplingrate_ = value; }

    void setMasterBlocksize(boost::uint32_t value) { master_blocksize_ = value; }

    void setAcquiredSignalTypes(const std::vector<boost::uint32_t>& sig_types)
      { sig_types_ = sig_types; }
    void setBlockSizesPerSignalType(const std::vector<boost::uint16_t>& blocksizes)
      { blocksizes_ = blocksizes; }
	void setSamplingRatePerSignalType(const std::vector<boost::uint32_t>& fs_per_sig_type)
      { fs_per_sig_type_ = fs_per_sig_type; }
	void setChannelNames(const std::map<boost::uint32_t, std::vector<std::string> >& channels_per_sig_type)
      { channels_per_sig_type_ = channels_per_sig_type; }

  private:
    boost::asio::io_service&            io_service_;
    XMLParser*                          config_;
    std::map<std::string, std::string>  server_settings_;
    TCPDataServer*                      tcp_data_server_;
    UDPDataServer*                      udp_data_server_;
    ControlConnectionServer*            control_connection_server_;

    boost::uint32_t                                       master_blocksize_;
    boost::uint32_t                                       master_samplingrate_;
    std::vector<boost::uint32_t>                          sig_types_;
    std::vector<boost::uint16_t>                          blocksizes_;
    std::vector<boost::uint32_t>                          fs_per_sig_type_;
    std::map<boost::uint32_t, std::vector<std::string> >  channels_per_sig_type_;

#ifdef TIMING_TEST
    boost::posix_time::ptime timestamp_;
    boost::posix_time::time_duration diff_;
    boost::posix_time::time_duration t_mean_;
    boost::int64_t t_var_;
    boost::uint64_t counter_;

    bool  lpt_flag_;

#endif
};

//-----------------------------------------------------------------------------

#endif // SIGNALSERVER_H

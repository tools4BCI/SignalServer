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
class GDFWriter;

namespace tobiss
{
// forward declarations
class ControlConnectionServer;
class TCPDataServer;
class UDPDataServer;
class XMLParser;
class DataPacket;
class ControlMsgEncoder;
class ControlMsgDecoder;
class SSMethods;

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
    * @brief Insert data (only one signal type) from a hardware device into the DataPacket (will be automatically placed correct).
    */
    void initialize(XMLParser* config);

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

    void setPhysicalRange(const std::vector<boost::int32_t>& physical_range)
      { physical_range_ = physical_range; }
    void setDigitalRange(const std::vector<boost::int32_t>& digital_range)
      { digital_range_ = digital_range; }
    void setDataType(const std::vector<Constants::DataType>& data_type)
      { data_type_ = data_type; }
    void setBpFilterSettings(const std::vector<std::pair<float, float> >& bandpass_filter)
      { bandpass_filter_ = bandpass_filter; }
    void setNFilterSettings(const std::vector<std::pair<float, float> >& notch_filter)
      { notch_filter_ = notch_filter; }
    void setDescription(const std::vector<std::string>& description)
      { description_ = description; }
    void setDeviceId(const std::vector<boost::uint32_t>& device_id)
      { device_id_ = device_id; }
    /**
    * @brief Starts HWAccess and passes config-information to server
    */
    void startServerSettings(XMLParser& config);

    /**
    * @brief Writes a new XML-document and sends it to a Method in SSMethods
    * which sets then the new config sent by the Client
    */
    void setClientConfig(const std::string& config, bool& configOk);

    /**
    * @brief Sets a pointer to the config
    */
    void setConfig(XMLParser* config);

    /**
    * @brief Returns reference to the used config
    */
    void getConfig(XMLParser& config);

    /**
    * @brief Returns reference to the used IOService
    */
    boost::asio::io_service& getIOService() {return io_service_;}

    /**
    * @brief Sets a pointer to SSMethods
    */
    void setSSMethods(SSMethods* ssm) {ss_methods_ = ssm;}

    /**
    * @brief Sets server_settings_
    * used by SSMethods to tell the Server the new settings sent by the Client
    */
    void setServerSettings();

    /**
    * @brief Sets timeout for KeepAlive
    */
    void setTimeoutKeepAlive(boost::uint32_t seconds);

    /**
    * @brief Sets Server in deamon-mode
    */
    void setDeamonMode() {deamon_ = true;}

    bool isDeamon() {return deamon_;}

  private:
    /**
    * @brief Handles timeout for KeepAlive
    */
    void handleTimeoutKeepAlive();

    /**
     * @brief Sends a DataPacket to the clients
     * @param[in]  packet
     * @throws
     */
    void initGdf();

  private:
    boost::asio::io_service&            io_service_; ///<
    XMLParser*                          config_; ///<
    std::map<std::string, std::string>  server_settings_; ///<
    TCPDataServer*                      tcp_data_server_; ///<
    UDPDataServer*                      udp_data_server_; ///<
    ControlConnectionServer*            control_connection_server_; ///<
    ControlMsgEncoder*                  msg_encoder_; ///<
    ControlMsgDecoder*                  msg_decoder_; ///<

    boost::uint32_t                                       master_blocksize_; ///<
    boost::uint32_t                                       master_samplingrate_; ///<
    std::vector<boost::uint32_t>                          sig_types_; ///<
    std::vector<boost::uint16_t>                          blocksizes_; ///<
    std::vector<boost::uint32_t>                          fs_per_sig_type_; ///<
    std::map<boost::uint32_t, std::vector<std::string> >  channels_per_sig_type_; ///<

    std::vector<boost::uint32_t>            device_id_;
    std::vector<std::string>                description_;
    std::vector<boost::int32_t>             physical_range_;
    std::vector<boost::int32_t>             digital_range_;
    std::vector<std::pair<float, float> >   bandpass_filter_;
    std::vector<std::pair<float, float> >   notch_filter_;
    std::vector<Constants::DataType>        data_type_;

    Constants                               cst_; ///<

    bool                                    write_file; ///<
    GDFWriter*                              gdf_writer_; ///<
    SSMethods*                              ss_methods_;

    boost::asio::deadline_timer             timeout_; ///< needed for KeepAlive
    boost::uint32_t                         sec_for_timeout_;

    bool                                    deamon_;

#ifdef TIMING_TEST
    boost::posix_time::ptime timestamp_;
    boost::posix_time::time_duration diff_;
    boost::posix_time::time_duration t_mean_;
    boost::int64_t t_var_;
    boost::uint64_t counter_;

    bool  lpt_flag_;

#endif
};

} // Namespace tobiss

//-----------------------------------------------------------------------------

#endif // SIGNALSERVER_H

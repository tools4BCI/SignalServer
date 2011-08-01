#ifndef SIGNALSERVER_H
#define SIGNALSERVER_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>


#ifdef USE_TID_SERVER
  namespace TiD
  {
    class TiDServer;
  }
#endif

#ifdef USE_GDF_SAVER
  namespace gdf
  {
    class Writer;
  }
#endif

namespace tobiss
{

class XMLParser;
class HWAccess;
class TiAServer;

//-----------------------------------------------------------------------------

class SignalServer
{
  public:
    SignalServer(HWAccess& hw_access, TiAServer& server, XMLParser& config_parser);
    virtual ~SignalServer();

    void stop();
    void readPackets();

  private:
    void initGdf(); ///< Initialize writing acquired data into a .gdf file.

    boost::asio::io_service     io_;
    boost::thread*              io_service_thread_;

    HWAccess&                   hw_access_;
    TiAServer&                  tia_server_;
    XMLParser&                  config_parser_;

    #ifdef USE_TID_SERVER
      TiD::TiDServer*             tid_server_;
    #endif

    #ifdef USE_GDF_SAVER
      gdf::Writer*                gdf_writer_;
    #endif

    bool                                stop_reading_;
    bool                                write_file_;
    boost::uint32_t                     master_blocksize_;
    boost::uint32_t                     master_samplingrate_;
    std::map<std::string,std::string>   subject_info_;
    std::map<std::string,std::string>   server_settings_;
    std::map<boost::uint32_t, std::vector<std::string> >    channels_per_sig_type_;
    std::vector<boost::uint32_t>                            sampling_rate_per_sig_type_;
};

//-----------------------------------------------------------------------------

} // tobiss

#endif // SIGNALSERVER_H

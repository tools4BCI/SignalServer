#ifndef SIGNALSERVER_H
#define SIGNALSERVER_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>


namespace TiD
{
  class TiDServer;
}

class GDFWriter;

namespace tobiss
{

class HWAccess;
class TiAServer;

//-----------------------------------------------------------------------------

class SignalServer
{
  public:
    SignalServer(HWAccess& hw_access, TiAServer& server);
    virtual ~SignalServer();

    void stop();
    void readPackets();

  private:
    void initGdf(); ///< Initialize writing acquired data into a .gdf file.

    boost::asio::io_service     io_;
    boost::thread*              io_service_thread_;

    HWAccess&                   hw_access_;
    TiAServer&                  tia_server_;
    TiD::TiDServer*              tid_server_;

    GDFWriter*                          gdf_writer_; ///<

    bool                        stop_reading_;
};

//-----------------------------------------------------------------------------

} // tobiss

#endif // SIGNALSERVER_H

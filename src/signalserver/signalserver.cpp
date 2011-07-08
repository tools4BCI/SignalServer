#include "signalserver/signalserver.h"

#include "tia/tia_server.h"
#include "hardware/hw_access.h"
#include "TiDlib/tid_server.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

SignalServer::SignalServer(HWAccess& hw_access, TiAServer& server)
  : hw_access_(hw_access), tia_server_(server), stop_reading_(false)
{
  tid_server_ = new TiD::TiDServer(io_);

  tid_server_->bind (12345);
  tid_server_->listen();
  io_service_thread_ = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_));
}

//-----------------------------------------------------------------------------

SignalServer::~SignalServer()
{
  io_.stop();
  io_service_thread_->interrupt();
  io_service_thread_->join();

  if(io_service_thread_)
    delete(io_service_thread_);

  if(tid_server_)
    delete(tid_server_);
}

//-----------------------------------------------------------------------------

void SignalServer::stop()
{
  stop_reading_ = true;
}

//-----------------------------------------------------------------------------

void SignalServer::readPackets()
{
  while (!stop_reading_)
  {
    DataPacket p = hw_access_.getDataPacket();
    tia_server_.sendDataPacket(p);
  }
}

//-----------------------------------------------------------------------------

}  // tobiss


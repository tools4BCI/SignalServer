#include "tia-private/newtia/server_impl/control_connection_server_2_impl.h"

#include "tia-private/newtia/server_impl/control_connection_2.h"
#include <iostream>

namespace tia
{

static const int SECONDS_TO_RE_CHECK_CONNECTIONS = 1;

//-----------------------------------------------------------------------------

ControlConnectionServer2Impl::ControlConnectionServer2Impl(
    boost::shared_ptr<TCPServerSocket> server_socket, unsigned port,
    DataServer* data_server, TiAServerStateServer* server_state_server,
    HardwareInterface* hardware_interface)
 : server_socket_ (server_socket), server_socket_port_ (port),
   data_server_(data_server), server_state_server_(server_state_server),
   hardware_interface_(hardware_interface), check_connections_timer_(io_service_)
{
  server_socket_->startListening (server_socket_port_, this);

  boost::system::error_code err;
  checkConnections (err);
  io_thread_  = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));
}

//-----------------------------------------------------------------------------

ControlConnectionServer2Impl::~ControlConnectionServer2Impl ()
{
  #ifdef DEBUG
    std::cout << "ControlConnectionServer2Impl::~ControlConnectionServer2Impl ()" << std::endl;
  #endif

  io_service_.stop();
  io_thread_->join();
  if(io_thread_)
    delete io_thread_;
}

//-----------------------------------------------------------------------------

void ControlConnectionServer2Impl::newConnection (boost::shared_ptr<Socket> socket)
{
  try
  {
    boost::unique_lock<boost::mutex> lock(mutex_);

    tia::ControlConnection2* control_connection =
        new tia::ControlConnection2 (socket, *data_server_, *server_state_server_,
                                     *hardware_interface_);
    unsigned id = control_connection->getId();
    connections_[id] = control_connection;

    std::cout << " Client " << id <<" @" << socket->getRemoteEndPointAsString();
    std::cout << " has connected. (local: "  << socket->getLocalEndPointAsString() << ")"<< std::endl;
    std::cout << " # Connected clients: " << connections_.size () << std::endl;

    control_connection->asyncStart ();
  }
  catch (TiALostConnection& /*exc*/)
  {
    // do nothing
  }
}

//-----------------------------------------------------------------------------

void ControlConnectionServer2Impl::checkConnections (boost::system::error_code error)
{
  if(error)
      return;
  boost::unique_lock<boost::mutex> lock(mutex_);

  #ifdef DEBUG
    static unsigned call_counter = 0;
    std::cout << " <- check connection "<< ++call_counter <<" -> " << std::endl;
  #endif

  unsigned int nr_removed = 0;

  for (std::map<unsigned, tia::ControlConnection2*>::iterator iter = connections_.begin();
       iter != connections_.end();   )
  {
    if (!(iter->second->isRunning()))
    {
      std::map<unsigned, tia::ControlConnection2*>::iterator iter_to_del = iter;
      std::cout << "  -- Removing connection to: " ;
      std::cout << iter->second->getRemoteEndPointAsString() << std::endl;

      nr_removed++;
      delete iter->second;
      ++iter;
      connections_.erase (iter_to_del);
    }
    else
      ++iter;

  }

  if(nr_removed)
    std::cout << " # Connected clients: " << connections_.size() << std::endl;

  check_connections_timer_.cancel ();
  check_connections_timer_.expires_from_now (boost::posix_time::seconds (SECONDS_TO_RE_CHECK_CONNECTIONS));
  check_connections_timer_.async_wait(boost::bind(&ControlConnectionServer2Impl::checkConnections, this, boost::asio::placeholders::error));
}

//-----------------------------------------------------------------------------

}

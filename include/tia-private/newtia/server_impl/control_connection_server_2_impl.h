#ifndef CONTROL_CONNECTION_SERVER_2_IMPL_H
#define CONTROL_CONNECTION_SERVER_2_IMPL_H

#include<list>


#include "tia-private/newtia/network/tcp_server_socket.h"
#include "tia-private/newtia/network_impl/boost_tcp_socket_impl.h"

#include "tia-private/newtia/server/control_connection_server_2.h"
#include "tia-private/newtia/server/tia_server_state_server.h"

#include "tia-private/newtia/tia_control_command.h"
#include "tia-private/newtia/tia_control_command_context.h"

#include "tia-private/newtia/messages/tia_control_message.h"
#include "tia-private/newtia/messages/tia_control_message_builder.h"
#include "tia-private/newtia/messages/tia_control_message_parser.h"

#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>

namespace tia
{

class ControlConnection2;


class ControlConnectionServer2Impl : public ControlConnectionServer2, public NewConnectionListener
{
  public:
    ControlConnectionServer2Impl(boost::shared_ptr<TCPServerSocket> server_socket,
                                 unsigned port, DataServer* data_server,
                                 TiAServerStateServer* server_state_server,
                                 HardwareInterface* hardware_interface);

    virtual ~ControlConnectionServer2Impl ();
    virtual void newConnection (boost::shared_ptr<Socket> socket);

  private:
    void checkConnections (boost::system::error_code error);

  private:
    typedef std::map<std::string, TiAControlCommand*> CommandMap;

    boost::shared_ptr<TCPServerSocket>        server_socket_;
    unsigned int                              server_socket_port_;
    tia::DataServer*                          data_server_;
    TiAServerStateServer*                     server_state_server_;
    tia::HardwareInterface*                   hardware_interface_;

    boost::mutex                              mutex_;      ///< mutex needed for the connection list

    std::map<unsigned, tia::ControlConnection2*>    connections_;

    boost::asio::io_service                         io_service_;
    boost::asio::deadline_timer                     check_connections_timer_;
    boost::thread*                                  io_thread_;
};
}

#endif // CONTROL_CONNECTION_SERVER_2_IMPL_H

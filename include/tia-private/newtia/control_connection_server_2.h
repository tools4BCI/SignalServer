#ifndef SERVER_CONTROL_CONNECTION_H
#define SERVER_CONTROL_CONNECTION_H

#include "network/socket.h"
#include "tia_control_command.h"
#include "tia_control_command_context.h"
#include "tia_control_message.h"
#include "tia_control_message_builder.h"
#include "tia_control_message_parser.h"
#include "hardware_interface.h"
#include "data_server.h"

#include <boost/thread.hpp>
#include <map>
#include <memory>

namespace tia
{

//-----------------------------------------------------------------------------
class ServerControlConnection
{
public:
    ServerControlConnection (Socket& socket, DataServer& data_server, HardwareInterface& hardware_interface);
    ~ServerControlConnection ();

    void asyncStart ();
    void stop ();

private:
    void run ();

    bool running_;
    Socket& socket_;
    DataServer& data_server_;
    typedef std::map<std::string, TiAControlCommand*> CommandMap;
    CommandMap command_map_;
    std::auto_ptr<TiAControlMessageParser> parser_;
    std::auto_ptr<TiAControlMessageBuilder> builder_;
    boost::thread* thread_;
    TiAControlCommandContext command_context_;
};

}

#endif // CONTROL_CONNECTION_SERVER_2_H

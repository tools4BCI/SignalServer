#ifndef SERVER_CONTROL_CONNECTION_H
#define SERVER_CONTROL_CONNECTION_H

#include "network/socket.h"
#include "tia_control_command.h"
#include "tia_control_command_context.h"
#include "messages/tia_control_message.h"
#include "messages/tia_control_message_builder.h"
#include "messages/tia_control_message_parser.h"
#include "server/tia_server_state_server.h"
#include "hardware_interface.h"
#include "server/data_server.h"

#include <boost/thread.hpp>
#include <map>
#include <memory>

namespace tia
{

//-----------------------------------------------------------------------------
class ServerControlConnection
{
public:
    ServerControlConnection (Socket& socket, DataServer& data_server, HardwareInterface& hardware_interface, TiAServerStateServer& server_state_server);
    ~ServerControlConnection ();

    void asyncStart ();
    void stop ();
    bool isRunning () const;
    unsigned getId () const {return id_;}

private:
    void run ();

    bool running_;
    Socket& socket_;
    DataServer& data_server_;
    TiAServerStateServer& server_state_server_;
    typedef std::map<std::string, TiAControlCommand*> CommandMap;
    CommandMap command_map_;
    std::auto_ptr<TiAControlMessageParser> parser_;
    std::auto_ptr<TiAControlMessageBuilder> builder_;
    boost::thread* thread_;
    TiAControlCommandContext command_context_;
    unsigned id_;
    static unsigned next_free_id_;
};

}

#endif // CONTROL_CONNECTION_SERVER_2_H

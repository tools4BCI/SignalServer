#include "tia-private/server/control_connection_server_2.h"
#include "tia-private/server/commands/check_protocol_version_control_command.h"
#include "tia-private/server/version_1_0/tia_control_message_tags_1_0.h"
#include "tia-private/server/version_1_0/tia_control_message_parser_1_0.h"
#include "tia-private/server/version_1_0/tia_control_message_builder_1_0.h"
#include "tia-private/server/messages/standard_control_messages.h"

using std::string;

#include <iostream>

namespace tia
{

//-----------------------------------------------------------------------------
ServerControlConnection::ServerControlConnection (Socket& socket, DataServer& data_server)
    : running_ (false),
      socket_ (socket),
      data_server_ (data_server),
      parser_ (new TiAControlMessageParser10),
      builder_ (new TiAControlMessageBuilder10)
{
    command_map_[TiAControlMessageTags10::CHECK_PROTOCOL_VERSION] = new CheckProtocolVersionControlCommand ();
}

//-----------------------------------------------------------------------------
ServerControlConnection::~ServerControlConnection ()
{
    for (CommandMap::iterator iter = command_map_.begin(); iter != command_map_.end();
         ++iter)
        delete iter->second;
}

//-----------------------------------------------------------------------------
void ServerControlConnection::asyncStart ()
{
    thread_ = new boost::thread (boost::bind (&ServerControlConnection::run, boost::ref(*this)));
}

//-----------------------------------------------------------------------------
void ServerControlConnection::stop ()
{
    running_ = false;
}

//-----------------------------------------------------------------------------
void ServerControlConnection::run ()
{
    running_ = true;
    while (running_)
    {
        socket_.waitForData ();

        std::cout << "ServerControlConnection::run " << std::endl;

        TiAControlMessage message = parser_->parseMessage (socket_);

        std::cout << "ServerControlConnection::run received message: " << message.getVersion()  << "; command = \" " << message.getCommand() << "\""<< std::endl;

        CommandMap::iterator command_iter = command_map_.find (message.getCommand ());
        string response;
        if (command_iter != command_map_.end ())
            response = builder_->buildTiAMessage (command_iter->second->execute (message));
        else
            response = builder_->buildTiAMessage (ErrorControlMessage (message.getVersion()));

        std::cout << "ServerControlConnection::run will send: \"" << response << "\"" << std::endl;

        socket_.sendString (response);
    }
}

}

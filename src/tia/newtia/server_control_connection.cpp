#include "tia-private/newtia/server_control_connection.h"
#include "tia-private/newtia/commands/check_protocol_version_control_command.h"
#include "tia-private/newtia/commands/get_data_connection_control_command.h"
#include "tia-private/newtia/commands/start_data_transmission_control_command.h"
#include "tia-private/newtia/commands/stop_data_transmission_control_command.h"
#include "tia-private/newtia/commands/get_metainfo_control_command.h"
#include "tia-private/newtia/commands/get_serverstate_connection_command.h"

#include "tia-private/newtia/messages/tia_control_message_tags_1_0.h"
#include "tia-private/newtia/messages_impl/tia_control_message_parser_1_0.h"
#include "tia-private/newtia/messages_impl/tia_control_message_builder_1_0.h"
#include "tia-private/newtia/messages/standard_control_messages.h"
#include "tia-private/newtia/tia_exceptions.h"

using std::string;

#include <iostream>

namespace tia
{
unsigned ServerControlConnection::next_free_id_ = 0;

//-----------------------------------------------------------------------------
ServerControlConnection::ServerControlConnection (Socket& socket, DataServer& data_server, HardwareInterface& hardware_interface, TiAServerStateServer& server_state_server)
    : running_ (true),
      socket_ (socket),
      data_server_ (data_server),
      server_state_server_ (server_state_server),
      parser_ (new TiAControlMessageParser10),
      builder_ (new TiAControlMessageBuilder10),
      command_context_ (hardware_interface),
      id_ (next_free_id_++)
{
    command_map_[TiAControlMessageTags10::CHECK_PROTOCOL_VERSION] = new CheckProtocolVersionControlCommand ();
    command_map_[TiAControlMessageTags10::GET_METAINFO] = new GetMetaInfoControlCommand (command_context_);
    command_map_[TiAControlMessageTags10::GET_DATA_CONNECTION] = new GetDataConnectionControlCommand (command_context_, data_server_);
    command_map_[TiAControlMessageTags10::START_DATA_TRANSMISSION] = new StartDataTransmissionControlCommand (command_context_, data_server_);
    command_map_[TiAControlMessageTags10::STOP_DATA_TRANSMISSION] = new StopDataTransmissionControlCommand (command_context_, data_server_);
    command_map_[TiAControlMessageTags10::GET_SERVER_STATE_CONNECTION] = new GetServerStateConnectionCommand (server_state_server_.getPort());
}

//-----------------------------------------------------------------------------
ServerControlConnection::~ServerControlConnection ()
{
    for (CommandMap::iterator iter = command_map_.begin(); iter != command_map_.end();
         ++iter)
        delete iter->second;
    delete thread_;
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
bool ServerControlConnection::isRunning() const
{
    return running_;
}

//-----------------------------------------------------------------------------
void ServerControlConnection::run ()
{
    running_ = true;
    while (running_)
    {
        try
        {
            socket_.waitForData ();

            //std::cout << "ServerControlConnection::run " << std::endl;

            TiAControlMessage message = parser_->parseMessage (socket_);

            //std::cout << "ServerControlConnection::run received message: " << message.getVersion()  << "; command = \"" << message.getCommand() << "\""<< std::endl;

            CommandMap::iterator command_iter = command_map_.find (message.getCommand ());
            string response;
            if (command_iter != command_map_.end ())
                response = builder_->buildTiAMessage (command_iter->second->execute (message));
            else
                response = builder_->buildTiAMessage (ErrorControlMessage (message.getVersion()));

            //std::cout << "ServerControlConnection::run will send: \"" << response << "\"" << std::endl;

            socket_.sendString (response);
        }
        catch (TiALostConnection& exc)
        {
            //std::cout << "ServerControlConnection::run lost connection to client: \"" << exc.what() << "\"" << std::endl;
            running_ = false;
        }
        catch (TiAException& exc)
        {
            std::cout << "ServerControlConnection::run error occured: \"" << exc.what() << "\"" << std::endl;
            running_ = false;
        }
    }
}

}

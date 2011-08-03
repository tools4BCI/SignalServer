/*
    This file is part of the TOBI Interface A (TiA) library.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file lgpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/lgpl.html.

    In case of GNU Lesser General Public License Usage ,the TiA library
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TiA library. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: TiA@tobi-project.org
*/


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

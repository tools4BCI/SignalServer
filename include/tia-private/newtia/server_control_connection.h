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

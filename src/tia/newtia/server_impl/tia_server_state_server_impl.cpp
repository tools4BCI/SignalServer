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

#include "tia-private/newtia/server_impl/tia_server_state_server_impl.h"
#include "tia-private/newtia/messages_impl/tia_control_message_builder_1_0.h"

#include <boost/thread.hpp>
#include <iostream>

namespace tia
{

//-----------------------------------------------------------------------------
std::map<ServerState, std::string> TiAServerStateServerImpl::state_map_;

//-----------------------------------------------------------------------------
void TiAServerStateServerImpl::init ()
{
    state_map_[SERVER_STATE_RUNNING] = TiAControlMessageTags10::SERVER_STATE_RUNNING;
    state_map_[SERVER_STATE_SHUTDOWN] = TiAControlMessageTags10::SERVER_STATE_SHUTDOWN;
}


//-----------------------------------------------------------------------------
TiAServerStateServerImpl::TiAServerStateServerImpl(
  boost::shared_ptr<TCPServerSocket> server_socket, unsigned port)
    : server_socket_ (server_socket),
      message_builder_ (new TiAControlMessageBuilder10),
      port_ (port),
      current_state_ (SERVER_STATE_RUNNING)
{
    init ();
    server_socket_->startListening (port_, this);
}

//-----------------------------------------------------------------------------
TiAServerStateServerImpl::TiAServerStateServerImpl (boost::shared_ptr<TCPServerSocket> server_socket)
    : server_socket_ (server_socket),
      message_builder_ (new TiAControlMessageBuilder10),
      port_ (0),
      current_state_ (SERVER_STATE_RUNNING)
{
    init ();
    port_ = server_socket_->startListening (this);
}

//-----------------------------------------------------------------------------
TiAServerStateServerImpl::~TiAServerStateServerImpl ()
{
    server_socket_->stopListening ();
}

//-----------------------------------------------------------------------------
unsigned TiAServerStateServerImpl::getPort () const
{
    return port_;
}

//-----------------------------------------------------------------------------
void TiAServerStateServerImpl::emitState (ServerState server_state)
{
    current_state_ = server_state;

    std::list<boost::shared_ptr<Socket> >::iterator client_iter = server_to_client_sockets_.begin();
    while (client_iter != server_to_client_sockets_.end ())
    {
        try
        {
            emitStateViaSocket (*client_iter, current_state_);
            ++client_iter;
        }
        catch (TiALostConnection& /*exc*/)
        {
            client_iter = server_to_client_sockets_.erase (client_iter);
        }
    }
}

//-----------------------------------------------------------------------------
void TiAServerStateServerImpl::newConnection (boost::shared_ptr<Socket> socket)
{
    try
    {
        // std::cout << __FUNCTION__ << " new client connected to server state server" << std::endl;
        emitStateViaSocket (socket, current_state_);
        server_to_client_sockets_.push_back (socket);
    }
    catch (TiALostConnection& /*exc*/)
    {
        // do nothing
    }
}

//-----------------------------------------------------------------------------
void TiAServerStateServerImpl::emitStateViaSocket (boost::shared_ptr<Socket> socket, ServerState server_state)
{
    socket->sendString (message_builder_->buildTiAMessage (TiAControlMessage ("1.0", state_map_[server_state], "", "")));
}


}

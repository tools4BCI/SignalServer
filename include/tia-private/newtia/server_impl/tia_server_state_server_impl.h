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

#ifndef TIA_SERVER_STATE_SERVER_IMPL_H
#define TIA_SERVER_STATE_SERVER_IMPL_H

#include "tia-private/newtia/server/tia_server_state_server.h"
#include "tia-private/newtia/network/tcp_server_socket.h"
#include "tia-private/newtia/messages/tia_control_message_builder.h"

#include <boost/asio/io_service.hpp>
#include <list>
#include <map>

namespace tia
{

//-----------------------------------------------------------------------------
class TiAServerStateServerImpl : public TiAServerStateServer, public NewConnectionListener
{
public:
    TiAServerStateServerImpl (boost::shared_ptr<TCPServerSocket> server_socket, unsigned port);
    TiAServerStateServerImpl (boost::shared_ptr<TCPServerSocket> server_socket);
    virtual ~TiAServerStateServerImpl ();

    virtual unsigned getPort () const;

    virtual void newConnection (boost::shared_ptr<Socket> socket);
    virtual void emitState (ServerState server_state);
private:
    void emitStateViaSocket (boost::shared_ptr<Socket> socket, ServerState server_state);

    static std::map<ServerState, std::string> state_map_;
    static void init ();
    boost::shared_ptr<TCPServerSocket> server_socket_;
    std::list<boost::shared_ptr<Socket> > server_to_client_sockets_;
    std::auto_ptr<TiAControlMessageBuilder> message_builder_;
    unsigned port_;
    ServerState current_state_;
};

}

#endif // TIA_SERVER_STATE_SERVER_IMPL_H

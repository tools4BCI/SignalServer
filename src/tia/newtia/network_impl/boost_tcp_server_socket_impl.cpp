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

#include "tia-private/newtia/network_impl/boost_tcp_server_socket_impl.h"
#include "tia-private/newtia/network_impl/boost_tcp_socket_impl.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/weak_ptr.hpp>

using namespace boost::asio::ip;
using boost::shared_ptr;
using boost::weak_ptr;

namespace tia
{

//-----------------------------------------------------------------------------
void BoostTCPServerSocketImpl::startListening (unsigned port,
                                               NewConnectionListener* new_connection_listener)
{
    new_connection_listener_ = new_connection_listener;

    tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
    acceptor_.open (endpoint.protocol());
    acceptor_.set_option (boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind (endpoint);
    acceptor_.listen ();
    asyncAccept ();
}

//-----------------------------------------------------------------------------
unsigned BoostTCPServerSocketImpl::startListening (NewConnectionListener* new_connection_listener)
{
    new_connection_listener_ = new_connection_listener;

    tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 0);
    acceptor_.open (endpoint.protocol());
    acceptor_.set_option (boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind (endpoint);
    acceptor_.listen ();
    asyncAccept ();
    return acceptor_.local_endpoint().port();
}

//-------------------------------------------------------------------------
void BoostTCPServerSocketImpl::stopListening ()
{
    new_connection_listener_ = 0;
    acceptor_.cancel ();
}

//-----------------------------------------------------------------------------
void BoostTCPServerSocketImpl::asyncAccept ()
{
    shared_ptr<tcp::socket> peer_socket (new tcp::socket(acceptor_.get_io_service()));
    acceptor_.async_accept (*peer_socket, boost::bind (&BoostTCPServerSocketImpl::handleAccept,
                                                       this, peer_socket));
}

//-----------------------------------------------------------------------------
void BoostTCPServerSocketImpl::handleAccept (shared_ptr<tcp::socket> socket)
{
    if (new_connection_listener_)
        new_connection_listener_->newConnection (boost::shared_ptr<Socket>(new BoostTCPSocketImpl (socket)));
    asyncAccept ();
}


}

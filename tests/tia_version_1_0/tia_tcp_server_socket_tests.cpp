/*
    This file is part of the TOBI SignalServer test routine.

    The TOBI SignalServer test routine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI SignalServer test routine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI SignalServer test routine. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

#include "../UnitTest++/UnitTest++.h"
#include "tia-private/newtia/network/tcp_server_socket.h"
#include "tia-private/newtia/network_impl/boost_tcp_server_socket_impl.h"
#include "tia-private/newtia/network_impl/boost_tcp_socket_impl.h"

#include <iostream>
#include <boost/thread.hpp>

using namespace tia;
using namespace boost;
using namespace std;

SUITE (TiANetwork)
{

string const send_string = "blub";

class TestListener : public NewConnectionListener
{
public:
    virtual void newConnection (shared_ptr<Socket> socket)
    {
        socket->sendString (send_string);
    }
};

TEST(TiATCPServerSocket)
{
    asio::io_service io_service;

    shared_ptr<TestListener> testlistener (new TestListener);
    auto_ptr<TCPServerSocket> server_socket (new BoostTCPServerSocketImpl (io_service));
    server_socket->startListening (9008, testlistener.get());

    asio::ip::tcp::endpoint connection_endpoint (boost::asio::ip::address_v4::from_string ("127.0.0.1"), 9008);
    auto_ptr<Socket> tcp_client_socket (new BoostTCPSocketImpl (io_service, connection_endpoint, 9000));

    boost::thread* io_thread = new boost::thread (boost::bind(&boost::asio::io_service::run, &io_service));

    tcp_client_socket->waitForData ();
    CHECK_EQUAL (send_string, tcp_client_socket->readString (send_string.size()));

    server_socket->stopListening ();
    io_service.stop ();
    io_thread->join ();
    delete io_thread;
}

}

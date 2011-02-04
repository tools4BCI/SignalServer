#include "../UnitTest++/UnitTest++.h"
#include "tia-private/newtia/network/tcp_server_socket.h"
#include "tia-private/newtia/network_impl/boost_tcp_server_socket_impl.h"
#include "tia-private/newtia/network_impl/boost_socket_impl.h"

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
    server_socket->startListening (9008, testlistener);

    asio::ip::tcp::endpoint connection_endpoint (boost::asio::ip::address_v4::from_string ("127.0.0.1"), 9008);
    auto_ptr<Socket> tcp_client_socket (new BoostTCPSocketImpl (io_service, connection_endpoint));

    boost::thread* io_thread = new boost::thread (boost::bind(&boost::asio::io_service::run, &io_service));

    tcp_client_socket->waitForData ();
    CHECK_EQUAL (send_string, tcp_client_socket->readString (send_string.size()));

    io_service.stop ();
    io_thread->join ();
    delete io_thread;
}

}

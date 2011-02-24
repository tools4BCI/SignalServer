#include "UnitTest++/UnitTest++.h"

#include "tia-private/newtia/network_impl/boost_socket_impl.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/thread.hpp>
#include <iostream>

using namespace tia;

SUITE (BoostSocketTests)
{

size_t const BIG_NUMBER = 10000;

void handle_accept (boost::shared_ptr<boost::asio::ip::tcp::socket> accept_socket)
{
    char delimiter = 0x0a;
    accept_socket->send (boost::asio::buffer (std::string ("Hugo")));
    accept_socket->send (boost::asio::buffer (&delimiter, 1));
    accept_socket->send (boost::asio::buffer (std::string ("Hugo Klaus")));
    accept_socket->send (boost::asio::buffer (&delimiter, 1));
    accept_socket->send (boost::asio::buffer (&delimiter, 1));
    accept_socket->send (boost::asio::buffer (std::string ("12345")));
    accept_socket->send (boost::asio::buffer (std::string ("some string")));
    for (size_t i = 0; i < BIG_NUMBER; i++)
        accept_socket->send (boost::asio::buffer (std::string (1, 'x')));
}

//-----------------------------------------------------------------------------
TEST(boostTCPSocketImpl)
{
    boost::asio::io_service io_service;

    boost::asio::ip::tcp::acceptor acceptor (io_service, boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4(), 9007));
    boost::shared_ptr<boost::asio::ip::tcp::socket> accept_socket (new boost::asio::ip::tcp::socket(io_service));
    acceptor.async_accept (*accept_socket, boost::bind (handle_accept, accept_socket));

    boost::asio::ip::tcp::endpoint connection_endpoint (boost::asio::ip::address_v4::from_string ("127.0.0.1"), 9007);

    Socket* socket = new BoostTCPSocketImpl (io_service, connection_endpoint, 8000);

    boost::thread* io_thread = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service));

    socket->waitForData ();
    CHECK_EQUAL (std::string ("Hugo"), socket->readLine (100));

    socket->waitForData ();
    CHECK_EQUAL (std::string ("Hugo Klaus"), socket->readLine (100));
    CHECK_EQUAL (std::string (""), socket->readLine (100));
    CHECK_EQUAL ('1', socket->readCharacter ());
    CHECK_EQUAL ('2', socket->readCharacter ());
    CHECK_EQUAL ('3', socket->readCharacter ());
    CHECK_EQUAL ('4', socket->readCharacter ());
    CHECK_EQUAL ('5', socket->readCharacter ());
    CHECK_EQUAL (std::string ("some string"), socket->readString (11));

    for (size_t i = 0; i < BIG_NUMBER / 100; i++)
        socket->readString (100);

    io_service.stop ();
    io_thread->join ();
    delete io_thread;

    delete socket;
}

}

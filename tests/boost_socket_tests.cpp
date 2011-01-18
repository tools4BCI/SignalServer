#include "UnitTest++/UnitTest++.h"

#include "tia-private/server/boost_socket_impl.h"

#include <boost/asio/ip/tcp.hpp>

using namespace tia;

//-----------------------------------------------------------------------------
TEST(boostTCPSocketImpl)
{
    boost::asio::io_service io_service;

    boost::asio::ip::tcp::acceptor acceptor (io_service, boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4(), 90050));
    boost::asio::ip::tcp::socket accept_socket;
            acceptor.async_accept ()

    Socket* socket = new BoostTCPSocketImpl (io_service, endpoint);

    io_service.run ();

    delete socket;
}

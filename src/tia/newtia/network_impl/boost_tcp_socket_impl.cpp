#include "tia-private/newtia/network_impl/boost_socket_impl.h"
#include "tia-private/newtia/messages/tia_control_message_tags_1_0.h"
#include "tia-private/newtia/tia_exceptions.h"

#include "tia-private/network/tcp_server.h"

#include <boost/asio.hpp>
#include <iostream>

using std::string;

namespace tia
{

//-----------------------------------------------------------------------------
BoostTCPSocketImpl::BoostTCPSocketImpl (boost::shared_ptr<tobiss::TCPConnection> con)
   : fusty_connection_ (con)
{
}

//-----------------------------------------------------------------------------
BoostTCPSocketImpl::BoostTCPSocketImpl (boost::shared_ptr<boost::asio::ip::tcp::socket> boost_socket)
    : socket_ (boost_socket)
{

}

//-----------------------------------------------------------------------------
BoostTCPSocketImpl::~BoostTCPSocketImpl ()
{
    if (socket_)
        socket_->close ();
    buffered_string_.clear ();
}

//-----------------------------------------------------------------------------
void BoostTCPSocketImpl::setReceiveBufferSize (unsigned size)
{
    boost::asio::socket_base::receive_buffer_size option(size);
    if (socket_)
        socket_->set_option (option);
    else if (fusty_connection_)
        fusty_connection_->socket().set_option (option);
}

//-----------------------------------------------------------------------------
string BoostTCPSocketImpl::readLine (unsigned /*max_length*/)
{
    string line;

    if (!buffered_string_.size())
        readBytes (1);

    while (buffered_string_[0] != TiAControlMessageTags10::NEW_LINE_CHAR)
    {
        line.push_back (buffered_string_[0]);
        buffered_string_.erase (0, 1);
        if (!buffered_string_.size())
            readBytes (1);
    }
    buffered_string_.erase (0, 1);

    return line;
}


//-----------------------------------------------------------------------------
string BoostTCPSocketImpl::readString (unsigned length)
{
    if (length > buffered_string_.size())
        readBytes (length - buffered_string_.size ());

    string str = buffered_string_.substr (0, length);
    buffered_string_.erase (0, length);

    return str;
}

//-----------------------------------------------------------------------------
char BoostTCPSocketImpl::readCharacter ()
{
    if (!buffered_string_.size ())
        readBytes (1);
    char character = buffered_string_[0];
    buffered_string_.erase (0, 1);
    return character;
}

//-----------------------------------------------------------------------------
void BoostTCPSocketImpl::waitForData ()
{
    if (!buffered_string_.size ())
        readBytes (1);
}

//-----------------------------------------------------------------------------
void BoostTCPSocketImpl::sendString (string const& str) throw (TiALostConnection)
{
    boost::system::error_code error;
    if (fusty_connection_)
        fusty_connection_->socket().send (boost::asio::buffer (str), 0, error);
    else
        socket_->send (boost::asio::buffer (str), 0, error);
    if (error)
        throw TiALostConnection ("BoostTCPSocketImpl: sending string failed");
}

//-----------------------------------------------------------------------------
void BoostTCPSocketImpl::readBytes (unsigned num_bytes)
{
    boost::system::error_code error;

    unsigned available = 0;
    if (fusty_connection_)
        available = fusty_connection_->socket().available (error);
    else
        available = socket_->available (error);

    if (error)
        throw TiALostConnection ("BoostTCPSocketImpl: error calling available: " + string (error.category().name()) + error.message());
    unsigned allocating = std::max<unsigned> (num_bytes, available);

    std::vector<char> data (allocating);
    if (fusty_connection_)
        fusty_connection_->socket().read_some (boost::asio::buffer (data, available), error);
    else
        socket_->read_some (boost::asio::buffer (data, available), error);

    if (error)
    {
        throw TiALostConnection ("BoostTCPSocketImpl: error read_some");
    }

    buffered_string_.append (data.data(), available);

    if (available < num_bytes)
    {
        if (fusty_connection_)
            fusty_connection_->socket().read_some (boost::asio::buffer (data, num_bytes - available), error);
        else
            socket_->read_some (boost::asio::buffer (data, num_bytes - available), error);

        if (error)
        {
            throw TiALostConnection ("BoostTCPSocketImpl: error read_some 2");
        }

        buffered_string_.append (data.data(), num_bytes - available);
    }
}


}

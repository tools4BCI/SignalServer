#include "tia-private/newtia/network_impl/boost_socket_impl.h"
#include "tia-private/newtia/version_1_0/tia_control_message_tags_1_0.h"
#include "tia-private/newtia/tia_exceptions.h"

#include "tia-private/network/tcp_server.h"

#include <boost/asio.hpp>
#include <iostream>

using std::string;

namespace tia
{

//-----------------------------------------------------------------------------
BoostTCPSocketImpl::BoostTCPSocketImpl (boost::asio::io_service& io_service,
                                        boost::shared_ptr<tobiss::TCPConnection> con)
   : fusty_connection_ (con)
{
    socket_ = &(con->socket());
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
void BoostTCPSocketImpl::sendString (string const& str)
{
    boost::system::error_code error;
    socket_->send (boost::asio::buffer (str), 0, error);
    if (error)
        throw TiALostConnection ("BoostTCPSocketImpl");
}

//-----------------------------------------------------------------------------
void BoostTCPSocketImpl::readBytes (unsigned num_bytes)
{
    boost::system::error_code error;

    unsigned available = socket_->available (error);
    if (error)
        throw TiALostConnection ("BoostTCPSocketImpl");
    unsigned allocating = std::max<unsigned> (num_bytes, available);

    char* data = new char [allocating];
    socket_->read_some (boost::asio::buffer (data, available), error);
    if (error)
        throw TiALostConnection ("BoostTCPSocketImpl");
    buffered_string_.append (data, available);

    if (available < num_bytes)
    {
        socket_->read_some (boost::asio::buffer (data, num_bytes - available), error);
        if (error)
            throw TiALostConnection ("BoostTCPSocketImpl");

        buffered_string_.append (data, num_bytes - available);
    }

    delete data;
}


}

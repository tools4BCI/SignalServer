#include "tia-private/newtia/network_impl/boost_udp_read_socket.h"
#include "tia-private/newtia/messages/tia_control_message_tags_1_0.h"
#include "tia-private/newtia/tia_exceptions.h"

#include <boost/asio.hpp>
#include <iostream>

using std::string;

namespace tia
{


//-----------------------------------------------------------------------------
string BoostUDPReadSocket::readLine (unsigned max_length)
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
string BoostUDPReadSocket::readString (unsigned length)
{
    if (length > buffered_string_.size())
        readBytes (length - buffered_string_.size ());

    string str = buffered_string_.substr (0, length);
    buffered_string_.erase (0, length);

    return str;
}

//-----------------------------------------------------------------------------
char BoostUDPReadSocket::readCharacter ()
{
    if (!buffered_string_.size ())
        readBytes (1);
    char character = buffered_string_[0];
    buffered_string_.erase (0, 1);
    return character;
}

//-----------------------------------------------------------------------------
void BoostUDPReadSocket::waitForData ()
{
    if (!buffered_string_.size ())
        readBytes (1);
}

//-----------------------------------------------------------------------------
void BoostUDPReadSocket::readBytes (unsigned num_bytes)
{
    boost::system::error_code error;

    unsigned available = socket_->available (error);
    if (error)
        throw TiALostConnection ("BoostUDPReadSocket");
    unsigned allocating = std::max<unsigned> (num_bytes, available);

    char* data = new char [allocating];
    socket_->receive (boost::asio::buffer (data, available), 0, error);
    if (error)
    {
        delete[] data;
        throw TiALostConnection ("BoostUDPReadSocket");
    }
    buffered_string_.append (data, available);

    if (available < num_bytes)
    {
        socket_->receive (boost::asio::buffer (data, num_bytes - available), 0, error);
        if (error)
        {
            delete[] data;
            throw TiALostConnection ("BoostUDPReadSocket");
        }

        buffered_string_.append (data, num_bytes - available);
    }

    delete[] data;
}


}


#include "tia-private/newtia/network_impl/boost_udp_read_socket.h"
#include "tia-private/newtia/messages/tia_control_message_tags_1_0.h"
#include "tia-private/newtia/tia_exceptions.h"

#include <boost/asio.hpp>
#include <iostream>

using std::string;

namespace tia
{


//-----------------------------------------------------------------------------
BoostUDPReadSocket::BoostUDPReadSocket (boost::asio::io_service& io_service, boost::asio::ip::udp::endpoint const& endpoint, unsigned buffer_size)
    : socket_ (new boost::asio::ip::udp::socket (io_service)),
      buffer_size_ (buffer_size),
      rec_buffer_ (buffer_size)
{
    boost::system::error_code error;
    boost::asio::socket_base::receive_buffer_size rec_buffer_size (buffer_size);

    boost::asio::socket_base::broadcast bcast(true);

    socket_->open (boost::asio::ip::udp::v4(), error);
    if (error)
        throw TiAException ("Could not open UDP socket.");

    socket_->bind (endpoint, error);
    socket_->set_option (bcast);
    socket_->set_option (rec_buffer_size);

    if (error)
        throw TiAException ("Could not bind UDP socket.");
    else
        std::cout << "UDP socket connect to " << endpoint << " successfull." << std::endl;
}


//-----------------------------------------------------------------------------
void BoostUDPReadSocket::setReceiveBufferSize (unsigned size)
{
    boost::asio::socket_base::receive_buffer_size option(size);
    socket_->set_option (option);
}

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

    if (error)
        throw TiALostConnection ("BoostUDPReadSocket");

    unsigned received = socket_->receive (boost::asio::buffer (rec_buffer_), 0, error);
    //std::cout << __FUNCTION__ << " received: " << received << "; data size = " << rec_buffer_.size()  <<  std::endl;
    if (error)
    {
        throw TiALostConnection ("BoostUDPReadSocket");
    }
    buffered_string_.append (rec_buffer_.data(), received);
}


}


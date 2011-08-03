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

#include "tia-private/newtia/network_impl/boost_tcp_socket_impl.h"
#include "tia-private/newtia/messages/tia_control_message_tags_1_0.h"
#include "tia-private/newtia/tia_exceptions.h"

#include "tia-private/network/tcp_server.h"

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

using std::string;

namespace tia
{
BoostTCPSocketImpl::BoostTCPSocketImpl (boost::asio::io_service& io_service,
                                        boost::asio::ip::tcp::endpoint const& endpoint, unsigned buffer_size)
    : socket_ (new boost::asio::ip::tcp::socket (io_service))
{
    socket_->connect (endpoint);
    boost::asio::socket_base::receive_buffer_size option (buffer_size);
    socket_->set_option (option);
    remote_endpoint_str_ = endpoint.address().to_string() + ":" + boost::lexical_cast<std::string>( endpoint.port() );
}

//-----------------------------------------------------------------------------
BoostTCPSocketImpl::BoostTCPSocketImpl (boost::shared_ptr<tobiss::TCPConnection> con)
  : fusty_connection_ (con), remote_endpoint_str_(con->endpointToString( con->socket().remote_endpoint() ))
{
}

//-----------------------------------------------------------------------------
BoostTCPSocketImpl::BoostTCPSocketImpl (boost::shared_ptr<boost::asio::ip::tcp::socket> boost_socket)
    : socket_ (boost_socket)
{
  remote_endpoint_str_ = socket_->remote_endpoint().address().to_string() + ":"
      + boost::lexical_cast<std::string>( socket_->remote_endpoint().port() );
}

//-----------------------------------------------------------------------------
BoostTCPSocketImpl::~BoostTCPSocketImpl ()
{
    if (socket_)
    {
        socket_->close ();
    }
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
    while (length > buffered_string_.size())
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

void BoostTCPSocketImpl::readBytes (unsigned requested_bytes)
{
    boost::system::error_code error;

    unsigned available = 0;
    unsigned read_bytes = 0;
    if (fusty_connection_)
        available = fusty_connection_->socket().available (error);
    else
        available = socket_->available (error);

    if (error)
        throw TiALostConnection ("BoostTCPSocketImpl: error calling available: " + string (error.category().name()) + error.message());
    unsigned allocating = std::max<unsigned> (requested_bytes, available);

    std::vector<char> data (allocating);

    while (read_bytes < requested_bytes)
    {
        unsigned read_bytes_now = 0;
        if (fusty_connection_)
            read_bytes_now += fusty_connection_->socket().read_some (boost::asio::buffer (data), error);
        else
            read_bytes_now += socket_->read_some (boost::asio::buffer (data), error);

        if (error)
        {
            throw TiALostConnection ("BoostTCPSocketImpl: error read_some 2");
        }

        buffered_string_.append (data.data(), read_bytes_now);
        read_bytes += read_bytes_now;
    }
}

//-----------------------------------------------------------------------------

std::string BoostTCPSocketImpl::getRemoteEndPointAsString()
{
  return(remote_endpoint_str_);
}

//-----------------------------------------------------------------------------

}

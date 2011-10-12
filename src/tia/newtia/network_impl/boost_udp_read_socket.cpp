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

string BoostUDPReadSocket::readUntil (char delimiter)
{
//  boost::system::error_code  error;

//  size_t transfered = boost::asio::read_until (socket_, stream_buffer_, delimiter,error );

//  if(error)
//    throw TiALostConnection ("InputStreamSocket::readUntil error read_until: "
//                             + string (error.category().name()) + error.message());
//  str_buffer_.clear();

//  char c;
//  size_t n = 1;
//  while(n != transfered)
//  {
//    input_stream_.get(c);
//    str_buffer_.append(&c);
//    n++;
//  }
//  input_stream_.get(c);

  return(buffered_string_);
}

//-----------------------------------------------------------------------------

string BoostUDPReadSocket::readUntil (std::string delimiter)
{

//  boost::system::error_code  error;

//  size_t transfered = boost::asio::read_until (socket_, stream_buffer_, delimiter,error );

//  if(error)
//    throw TiALostConnection ("InputStreamSocket::readLine error read_until: "
//                             + string (error.category().name()) + error.message());
//  str_buffer_.clear();

//  char c;
//  size_t n = delimiter.size();
//  while(n != transfered)
//  {
//    input_stream_.get(c);
//    str_buffer_.append(&c);
//    n++;
//  }

//  for(unsigned int s = 0; s < delimiter.size(); s++)
//    input_stream_.get();

  return(buffered_string_);
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


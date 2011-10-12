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

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

using std::string;

static const int RESERVED_STRING_LENGTH = 2048;
static const int RESERVED_STREAM_BUFFER_SIZE = 2048;

namespace tia
{
BoostTCPSocketImpl::BoostTCPSocketImpl (boost::asio::io_service& io_service,
                                        boost::asio::ip::tcp::endpoint const& endpoint, unsigned buffer_size)
  : socket_ (new boost::asio::ip::tcp::socket (io_service) ), input_stream_(&stream_buffer_)
{
  socket_->connect (endpoint);
  boost::asio::socket_base::receive_buffer_size option (buffer_size);
  socket_->set_option (option);

  remote_endpoint_str_ = socket_->remote_endpoint().address().to_string() + ":"
      + boost::lexical_cast<std::string>( socket_->remote_endpoint().port() );

  local_endpoint_str_ = socket_->local_endpoint().address().to_string() + ":"
      + boost::lexical_cast<std::string>( socket_->local_endpoint().port() );


  stream_buffer_.prepare(RESERVED_STREAM_BUFFER_SIZE);
  str_buffer_.reserve(RESERVED_STRING_LENGTH);
}

//-----------------------------------------------------------------------------

BoostTCPSocketImpl::BoostTCPSocketImpl (boost::shared_ptr<boost::asio::ip::tcp::socket> boost_socket)
    : socket_ (boost_socket), input_stream_(&stream_buffer_)
{
  remote_endpoint_str_ = socket_->remote_endpoint().address().to_string() + ":"
      + boost::lexical_cast<std::string>( socket_->remote_endpoint().port() );

  local_endpoint_str_ = socket_->local_endpoint().address().to_string() + ":"
      + boost::lexical_cast<std::string>( socket_->local_endpoint().port() );

  stream_buffer_.prepare(RESERVED_STREAM_BUFFER_SIZE);
  str_buffer_.reserve(RESERVED_STRING_LENGTH);
}

//-----------------------------------------------------------------------------
BoostTCPSocketImpl::~BoostTCPSocketImpl ()
{
    if (socket_)
    {
        socket_->close ();
    }
    str_buffer_.clear ();
}

//-----------------------------------------------------------------------------
void BoostTCPSocketImpl::setReceiveBufferSize (unsigned size)
{
    boost::asio::socket_base::receive_buffer_size option(size);
    if (socket_)
        socket_->set_option (option);
}

//-----------------------------------------------------------------------------

string BoostTCPSocketImpl::readUntil (char delimiter)
{
  error_.clear();
  size_t transfered = boost::asio::read_until (*socket_, stream_buffer_, delimiter, error_ );

  if(error_)
    throw TiALostConnection ("InputStreamSocket::readUntil error_ read_until: "
                             + string (error_.category().name()) + error_.message());
  str_buffer_.resize(transfered -1);

  size_t n = 0;
  while(n != transfered-1)
  {
    str_buffer_[n] = input_stream_.peek();
    stream_buffer_.consume(1);
    n++;
  }
  input_stream_.get();

  return(str_buffer_);
}

//-----------------------------------------------------------------------------

string BoostTCPSocketImpl::readUntil (std::string delimiter)
{
  error_.clear();
  size_t transfered = boost::asio::read_until (*socket_, stream_buffer_, delimiter,error_ );

  if(error_)
    throw TiALostConnection ("InputStreamSocket::readUntil error_ read_until: "
                             + string (error_.category().name()) + error_.message());
  str_buffer_.resize(transfered -delimiter.size());

  size_t n = 0;
  while(n != transfered-delimiter.size())
  {
    str_buffer_[n] = input_stream_.peek();
    stream_buffer_.consume(1);
    n++;
  }

  for(unsigned int s = 0; s < delimiter.size(); s++)
    input_stream_.get();

  return(str_buffer_);
}


//-----------------------------------------------------------------------------
string BoostTCPSocketImpl::readString (unsigned length)
{
  if(stream_buffer_.size() < length)
  {
    error_.clear();
    boost::asio::read (*socket_, stream_buffer_,
                       boost::asio::transfer_at_least( length - stream_buffer_.size() ), error_ ) ;
    if (error_)
      throw TiALostConnection ("InputStreamSocket::readString error_ read: "
                               + string (error_.category().name()) + error_.message());
  }

  str_buffer_.clear();
  str_buffer_.resize(length);

  for(unsigned int n = 0; n < length; n++)
  {
    str_buffer_[n] = input_stream_.peek();
    stream_buffer_.consume(1);
  }
  return(str_buffer_);
}

//-----------------------------------------------------------------------------
char BoostTCPSocketImpl::readCharacter ()
{
  if(!stream_buffer_.size())
    waitForData();

  char c = input_stream_.peek();
  stream_buffer_.consume(1);
  return(c);

}

//-----------------------------------------------------------------------------
void BoostTCPSocketImpl::waitForData ()
{
  error_.clear();
  if (!stream_buffer_.size () )
    boost::asio::read (*socket_, stream_buffer_, boost::asio::transfer_at_least(1), error_) ;

  if (error_)
    throw TiALostConnection ("InputStreamSocket::readBytes error_ read: "
                             + string (error_.category().name()) + error_.message());
}

//-----------------------------------------------------------------------------
void BoostTCPSocketImpl::sendString (string const& str) throw (TiALostConnection)
{
    error_.clear();
    socket_->send (boost::asio::buffer (str), 0, error_);
    if (error_)
        throw TiALostConnection ("BoostTCPSocketImpl: sending string failed");
}

//-----------------------------------------------------------------------------

size_t BoostTCPSocketImpl::readBytes (char* data, size_t bytes_to_read)
{

  if(stream_buffer_.size () < bytes_to_read )
  {
    error_.clear();
    boost::asio::read (*socket_, stream_buffer_,
                       boost::asio::transfer_at_least(bytes_to_read-stream_buffer_.size ()),error_);
    if (error_)
      throw TiALostConnection ("InputStreamSocket::readBytes error_ read: "
                               + string (error_.category().name()) + error_.message());

    input_stream_.read(data, bytes_to_read);
    return bytes_to_read;
  }
  else
  {
    input_stream_.read( data, bytes_to_read);
    return bytes_to_read;
  }
}

//-----------------------------------------------------------------------------

size_t BoostTCPSocketImpl::getAvailableData (char* data, size_t max_size)
{

  size_t available_data = stream_buffer_.size ();
  if ( !available_data )
  {
    data = 0;
    return 0;
  }
  else
  {
    if(available_data > max_size)
    {
      input_stream_.read( data, max_size);
      return max_size;
    }
    else
    {
      input_stream_.read( data, available_data);
      return available_data;
    }
  }
}

//-----------------------------------------------------------------------------

std::string BoostTCPSocketImpl::getRemoteEndPointAsString()
{
  return(remote_endpoint_str_);
}

//-----------------------------------------------------------------------------

std::string BoostTCPSocketImpl::getLocalEndPointAsString()
{
  return(local_endpoint_str_);
}

//-----------------------------------------------------------------------------

}

/*
    This file is part of the TOBI signal server.

    The TOBI signal server is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI signal server is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

/**
* @ssclient.cpp
*
* @brief \TODO.
*
**/

// local
#include "tia/ssclient.h"
#include "tia-private/signalserver-client/ssclientimpl.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

SSClient::SSClient() :
  impl_(0)
{
  impl_ = new SSClientImpl;
}

//-----------------------------------------------------------------------------

SSClient::~SSClient()
{
  if(impl_)
    delete impl_;
}

//-----------------------------------------------------------------------------

void SSClient::connect(const std::string& address, short unsigned port)
{
  impl_->connect(address, port);
}

//-----------------------------------------------------------------------------

bool SSClient::connected() const
{
  return impl_->connected();
}

//-----------------------------------------------------------------------------

void SSClient::disconnect()
{
  impl_->disconnect();
}

//-----------------------------------------------------------------------------

void SSClient::requestConfig()
{
  impl_->requestConfig();
}

//-----------------------------------------------------------------------------

SSConfig SSClient::config() const
{
  return impl_->config();
}

//-----------------------------------------------------------------------------

void SSClient::startReceiving(bool use_udp_bc)
{
  impl_->startReceiving(use_udp_bc);
}

//-----------------------------------------------------------------------------

bool SSClient::receiving() const
{
  return impl_->receiving();
}

//-----------------------------------------------------------------------------

void SSClient::stopReceiving()
{
  impl_->stopReceiving();
}

//-----------------------------------------------------------------------------

void SSClient::getDataPacket(DataPacket& packet)
{
  impl_->getDataPacket(packet);
}

//-----------------------------------------------------------------------------

void SSClient::setBufferSize(size_t size)
{
  impl_->setBufferSize(size);
}

//-----------------------------------------------------------------------------

} // Namespace tobiss

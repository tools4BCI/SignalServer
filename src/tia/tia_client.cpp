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
* @file tia_client.cpp
*
* @brief \TODO.
*
**/

// local
#include "tia/tia_client.h"
#include "tia-private/client/tia_client_impl.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

TiAClient::TiAClient() :
  impl_(0)
{
  impl_ = new TiAClientImpl;
}

//-----------------------------------------------------------------------------

TiAClient::~TiAClient()
{
  if(impl_)
    delete impl_;
}

//-----------------------------------------------------------------------------

void TiAClient::connect(const std::string& address, short unsigned port)
{
  impl_->connect(address, port);
}

//-----------------------------------------------------------------------------

bool TiAClient::connected() const
{
  return impl_->connected();
}

//-----------------------------------------------------------------------------

void TiAClient::disconnect()
{
  impl_->disconnect();
}

//-----------------------------------------------------------------------------

void TiAClient::requestConfig()
{
  impl_->requestConfig();
}

//-----------------------------------------------------------------------------

SSConfig TiAClient::config() const
{
  return impl_->config();
}

//-----------------------------------------------------------------------------

void TiAClient::startReceiving(bool use_udp_bc)
{
  impl_->startReceiving(use_udp_bc);
}

//-----------------------------------------------------------------------------

bool TiAClient::receiving() const
{
  return impl_->receiving();
}

//-----------------------------------------------------------------------------

void TiAClient::stopReceiving()
{
  impl_->stopReceiving();
}

//-----------------------------------------------------------------------------

void TiAClient::getDataPacket(DataPacket& packet)
{
  impl_->getDataPacket(packet);
}

//-----------------------------------------------------------------------------

void TiAClient::setBufferSize(size_t size)
{
  impl_->setBufferSize(size);
}

//-----------------------------------------------------------------------------

} // Namespace tobiss

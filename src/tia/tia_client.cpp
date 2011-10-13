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

/**
* @file tia_client.cpp
*
* @brief \TODO.
*
**/

// local
#include "tia/tia_client.h"
#include "tia-private/client/tia_client_impl.h"
#include "tia-private/client/tia_new_client_impl.h"

#include <iostream>

namespace tia
{
//-----------------------------------------------------------------------------

TiAClient::TiAClient(bool use_new_tia) :
  impl_(0)
{
  if(use_new_tia)
    impl_ = new tia::TiANewClientImpl;
  else
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

DataPacket* TiAClient::getEmptyDataPacket()
{
  return impl_->getEmptyDataPacket();
}

//-----------------------------------------------------------------------------

void TiAClient::setBufferSize(size_t size)
{
  impl_->setBufferSize(size);
}

//-----------------------------------------------------------------------------

} // Namespace tobiss

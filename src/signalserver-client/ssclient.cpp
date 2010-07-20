/**
* @ssclient.cpp
*
* @brief \TODO.
*
**/

// local
#include "signalserver-client/ssclient.h"
#include "signalserver-client/ssclientimpl.h"


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
{}

//-----------------------------------------------------------------------------

void SSClient::connect(const std::string& address, boost::uint16_t port)
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

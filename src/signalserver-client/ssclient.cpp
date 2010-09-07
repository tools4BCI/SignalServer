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

SSClient::SSClient(boost::asio::io_service& io_service) :
  impl_(0)
{
  impl_ = new SSClientImpl(io_service);
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

void SSClient::sendConfig(std::string& config)
{
  impl_->sendConfig(config);
}

//-----------------------------------------------------------------------------

void SSClient::setTimeoutKeepAlive(boost::uint32_t seconds)
{
  impl_->setTimeoutKeepAlive(seconds);
}

//-----------------------------------------------------------------------------

void SSClient::handleTimeoutKeepAlive()
{
  impl_->handleTimeoutKeepAlive();
}

//-----------------------------------------------------------------------------

} // Namespace tobiss

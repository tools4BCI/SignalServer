/**
* @file ssclient.h
*
* @brief
*
**/

#ifndef SSCLIENT_H
#define SSCLIENT_H

// STL
#include <vector>
#include <string>

// local
#include "ssclientimpl_base.h"

#ifndef DECL_EXPORT
  #define DECL_EXPORT
#endif

namespace tobiss
{
//-----------------------------------------------------------------------------

class DECL_EXPORT SSClient
{
public:
  SSClient();

  virtual ~SSClient();

  virtual void connect(const std::string& address, boost::uint16_t port);

  virtual bool connected() const;

  virtual void disconnect();

  virtual void requestConfig();

  virtual SSConfig config() const;

  virtual void startReceiving(bool use_udp_bc);

  virtual bool receiving() const;

  virtual void stopReceiving();

  virtual void getDataPacket(DataPacket& packet);
	
  virtual void setBufferSize(size_t size);

protected:
  SSClientImplBase* impl_;
};

} // Namespace tobiss

//-----------------------------------------------------------------------------

#endif // SSCLIENT_H

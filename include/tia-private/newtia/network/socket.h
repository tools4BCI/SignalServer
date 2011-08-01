#ifndef SOCKET_H
#define SOCKET_H

#include "../tia_exceptions.h"
#include "read_socket.h"

#include <string>

namespace tia
{

//-----------------------------------------------------------------------------
/// base class for any socket that could write data
class WriteSocket
{
  public:
    virtual ~WriteSocket () {}
    virtual void sendString (std::string const& string)  = 0;
  //throw (TiALostConnection)
};

//-----------------------------------------------------------------------------
/// base class for any socket
class Socket : public WriteSocket, public ReadSocket
{
  public:
    virtual ~Socket () {}
    virtual std::string getRemoteEndPointAsString() = 0;

};

}

#endif // SOCKET_H

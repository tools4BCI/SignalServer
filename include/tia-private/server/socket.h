#ifndef SOCKET_H
#define SOCKET_H

#include <string>

namespace tia
{

//-----------------------------------------------------------------------------
/// base class for any socket
class Socket
{
public:
    virtual void sendString (std::string const& string) = 0;
    virtual std::string readString () = 0;
};

}

#endif // SOCKET_H

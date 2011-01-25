#ifndef SOCKET_H
#define SOCKET_H

#include <string>

namespace tia
{

//-----------------------------------------------------------------------------
class InputStream
{
public:
    virtual ~InputStream () {}
    virtual std::string readLine (unsigned max_length) = 0;
    virtual std::string readString (unsigned length) = 0;
    virtual char readCharacter () = 0;
};

//-----------------------------------------------------------------------------
class ReadSocket : public InputStream
{
public:
    virtual ~ReadSocket () {}
    virtual void waitForData () = 0;
};

//-----------------------------------------------------------------------------
/// base class for any socket that could write data
class WriteSocket
{
public:
    virtual ~WriteSocket () {}
    virtual void sendString (std::string const& string) = 0;
};

//-----------------------------------------------------------------------------
/// base class for any socket
class Socket : public WriteSocket, public ReadSocket
{
public:
    virtual ~Socket () {}

};

}

#endif // SOCKET_H

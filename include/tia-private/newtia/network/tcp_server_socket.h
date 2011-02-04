#ifndef TCP_SERVER_SOCKET_H
#define TCP_SERVER_SOCKET_H

#include "new_connection_listener.h"

#include <boost/shared_ptr.hpp>

namespace tia
{

//-----------------------------------------------------------------------------
class TCPServerSocket
{
public:
    //-------------------------------------------------------------------------
    virtual ~TCPServerSocket () {}

    //-------------------------------------------------------------------------
    virtual void startListening (unsigned port,
                                 boost::shared_ptr<NewConnectionListener> new_connection_listener) = 0;
};

}

#endif // TCP_SERVER_SOCKET_H

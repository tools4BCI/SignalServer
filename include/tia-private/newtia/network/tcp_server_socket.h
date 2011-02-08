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
    /// only one listener is allowed
    virtual void startListening (unsigned port,
                                 NewConnectionListener* new_connection_listener) = 0;

    //-------------------------------------------------------------------------
    /// stops listening
    virtual void stopListening () = 0;
};

}

#endif // TCP_SERVER_SOCKET_H

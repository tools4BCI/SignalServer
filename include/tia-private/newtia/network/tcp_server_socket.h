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
    /// listen on the given port for new connections
    virtual void startListening (unsigned port,
                                 NewConnectionListener* new_connection_listener) = 0;

    //-------------------------------------------------------------------------
    /// only one listener is allowed
    /// listen on an arbitrary port for new connections
    /// @return the port number on which the server socket is listening
    virtual unsigned startListening (NewConnectionListener* new_connection_listener) = 0;

    //-------------------------------------------------------------------------
    /// stops listening
    virtual void stopListening () = 0;
};

}

#endif // TCP_SERVER_SOCKET_H

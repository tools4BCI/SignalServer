#ifndef SERVER_STATE_SERVER_H
#define SERVER_STATE_SERVER_H

#include "tcp_server_socket.h"

namespace tia
{

//-----------------------------------------------------------------------------
class ServerStateServer
{
public:
    virtual void emitState ();
};

}

#endif // SERVER_STATE_SERVER_H

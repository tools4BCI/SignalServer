#ifndef TIA_SERVER_STATE_SERVER_H
#define TIA_SERVER_STATE_SERVER_H

#include "server_states.h"

namespace tia
{

//-----------------------------------------------------------------------------
class TiAServerStateServer
{
public:
    virtual ~TiAServerStateServer () {}
    virtual unsigned getPort () const = 0;
    virtual void emitState (ServerState server_state) = 0;
};

}

#endif // TIA_SERVER_STATE_SERVER_H

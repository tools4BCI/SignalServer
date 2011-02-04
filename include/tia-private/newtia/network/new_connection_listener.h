#ifndef NEW_CONNECTION_LISTENER_H
#define NEW_CONNECTION_LISTENER_H

#include "socket.h"

#include <boost/shared_ptr.hpp>

namespace tia
{

//-----------------------------------------------------------------------------
class NewConnectionListener
{
public:
    virtual ~NewConnectionListener () {}

    virtual void newConnection (boost::shared_ptr<Socket> socket) = 0;
};

}

#endif // NEW_CONNECTION_LISTENER_H

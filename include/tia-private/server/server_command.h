#ifndef SERVER_COMMAND_H
#define SERVER_COMMAND_H

#include "socket.h"

namespace tia
{

class ServerCommand
{
public:
    void execute (Socket& socket) = 0;
};


}

#endif // SERVER_COMMAND_H

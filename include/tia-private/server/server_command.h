#ifndef SERVER_COMMAND_H
#define SERVER_COMMAND_H

namespace tia
{

class ServerCommand
{
public:
    virtual void execute () = 0;
};


}

#endif // SERVER_COMMAND_H

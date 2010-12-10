#ifndef STOP_DATA_TRANSMISSION_SERVER_COMMAND_H
#define STOP_DATA_TRANSMISSION_SERVER_COMMAND_H

#include "server_command.h"
#include "socket.h"

namespace tia
{

class StopDataTransmissionServerCommand : public ServerCommand
{
public:
    StopDataTransmissionServerCommand (WriteSocket& socket) : socket_ (socket) {}
    void execute ()
    {
        socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><errorReply  /></message>");
    }

private:
    WriteSocket& socket_;
};

}

#endif // STOP_DATA_TRANSMISSION_SERVER_COMMAND_H

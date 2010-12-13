#ifndef STOP_DATA_TRANSMISSION_SERVER_COMMAND_H
#define STOP_DATA_TRANSMISSION_SERVER_COMMAND_H

#include "server_command.h"
#include "socket.h"
#include "data_server.h"

namespace tia
{

class StopDataTransmissionServerCommand : public ServerCommand
{
public:
    StopDataTransmissionServerCommand (ConnectionID connection_id, DataServer& data_server, WriteSocket& socket) : connection_id_ (connection_id), socket_ (socket), data_server_ (data_server) {}

    void execute ()
    {
        socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><errorReply  /></message>");
    }

private:
    ConnectionID connection_id_;
    WriteSocket& socket_;
    DataServer& data_server_;
};

}

#endif // STOP_DATA_TRANSMISSION_SERVER_COMMAND_H

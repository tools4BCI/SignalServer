#ifndef STOP_DATA_TRANSMISSION_SERVER_COMMAND_H
#define STOP_DATA_TRANSMISSION_SERVER_COMMAND_H

#include "../server_command.h"
#include "../socket.h"
#include "../data_server.h"

namespace tia
{

//-------------------------------------------------------------------------------------------------
class StopDataTransmissionServerCommand : public ServerCommand
{
public:
    //-------------------------------------------------------------------------------------------------
    StopDataTransmissionServerCommand (ConnectionID const connection_id, DataServer& data_server, WriteSocket& socket);

    //-------------------------------------------------------------------------------------------------
    virtual void execute ();

private:
    ConnectionID const connection_id_;
    WriteSocket& socket_;
    DataServer& data_server_;
};

}

#endif // STOP_DATA_TRANSMISSION_SERVER_COMMAND_H

#ifndef GET_DATA_CONNECTION_SERVER_COMMAND_H
#define GET_DATA_CONNECTION_SERVER_COMMAND_H

#include "server_command.h"
#include "socket.h"
#include "data_server.h"

namespace tia
{

class GetDataConnectionServerCommand : public ServerCommand
{
public:
    GetDataConnectionServerCommand (ConnectionID& connection_id, DataServer& data_server, WriteSocket& write_socket)
        : connection_id_ (connection_id), data_server_ (data_server), write_socket_ (write_socket) {}

    virtual void execute ()
    {
    }

private:
    ConnectionID& connection_id_;
    DataServer& data_server_;
    WriteSocket& write_socket_;
};

}

#endif // GET_DATA_CONNECTION_SERVER_COMMAND_H

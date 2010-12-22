#ifndef GET_DATA_CONNECTION_SERVER_COMMAND_H
#define GET_DATA_CONNECTION_SERVER_COMMAND_H

#include "tia-private/server/server_command.h"
#include "tia-private/server/socket.h"
#include "tia-private/server/data_server.h"

namespace tia
{

//-----------------------------------------------------------------------------
class GetDataConnectionServerCommand : public ServerCommand
{
public:
    GetDataConnectionServerCommand (ConnectionID& connection_id, DataServer& data_server, WriteSocket& write_socket)
        : ServerCommand (write_socket),
        connection_id_ (connection_id), data_server_ (data_server), write_socket_ (write_socket)
    {}

    virtual void executeAttributes (std::map<std::string, std::string> const& /*attributes*/);

private:
    ConnectionID& connection_id_;
    DataServer& data_server_;
    WriteSocket& write_socket_;
};

}

#endif // GET_DATA_CONNECTION_SERVER_COMMAND_H

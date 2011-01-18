#ifndef GET_DATA_CONNECTION_SERVER_COMMAND_H
#define GET_DATA_CONNECTION_SERVER_COMMAND_H

#include "tia-private/server/tia_control_command.h"
#include "tia-private/server/socket.h"
#include "tia-private/server/data_server.h"

namespace tia
{

//-----------------------------------------------------------------------------
class GetDataConnectionControlCommand : public TiAControlCommand
{
public:
    GetDataConnectionControlCommand (ConnectionID& connection_id, DataServer& data_server)
        : connection_id_ (connection_id), data_server_ (data_server)
    {}

    virtual ~GetDataConnectionControlCommand () {}

    virtual TiAControlMessage execute (TiAControlMessage const& command);

private:
    ConnectionID& connection_id_;
    DataServer& data_server_;
};

}

#endif // GET_DATA_CONNECTION_SERVER_COMMAND_H

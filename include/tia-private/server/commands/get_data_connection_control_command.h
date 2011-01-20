#ifndef GET_DATA_CONNECTION_CONTROL_COMMAND_H
#define GET_DATA_CONNECTION_CONTROL_COMMAND_H

#include "../tia_control_command.h"
#include "../tia_control_command_context.h"
#include "../data_server.h"

namespace tia
{

//-----------------------------------------------------------------------------
class GetDataConnectionControlCommand : public TiAControlCommand
{
public:
    GetDataConnectionControlCommand (TiAControlCommandContext& command_context, DataServer& data_server)
        : command_context_ (command_context), data_server_ (data_server)
    {}

    virtual ~GetDataConnectionControlCommand () {}

    virtual TiAControlMessage execute (TiAControlMessage const& command);

private:
    TiAControlCommandContext& command_context_;
    DataServer& data_server_;
};

}

#endif // GET_DATA_CONNECTION_SERVER_COMMAND_H

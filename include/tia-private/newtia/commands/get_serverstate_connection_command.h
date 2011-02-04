#ifndef GET_SERVERSTATE_CONNECTION_COMMAND_H
#define GET_SERVERSTATE_CONNECTION_COMMAND_H

#include "../tia_control_command.h"

namespace tia
{

//-----------------------------------------------------------------------------
class StartDataTransmissionControlCommand : public TiAControlCommand
{
public:
    StartDataTransmissionControlCommand (TiAControlCommandContext& command_context, DataServer& data_server)
        : command_context_ (command_context), data_server_ (data_server)
    {}

    virtual ~StartDataTransmissionControlCommand () {}

    virtual TiAControlMessage execute (TiAControlMessage const& command);

private:
    TiAControlCommandContext& command_context_;
    DataServer& data_server_;
};

}

#endif // GET_SERVERSTATE_CONNECTION_COMMAND_H

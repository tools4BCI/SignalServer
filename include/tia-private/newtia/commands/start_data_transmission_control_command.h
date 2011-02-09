#ifndef START_DATA_TRANSMISSION_CONTROL_COMMAND_H
#define START_DATA_TRANSMISSION_CONTROL_COMMAND_H

#include "../tia_control_command.h"
#include "../tia_control_command_context.h"
#include "../server/data_server.h"

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

#endif // START_DATA_TRANSMISSION_CONTROL_COMMAND_H

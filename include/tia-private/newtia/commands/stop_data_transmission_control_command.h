#ifndef STOP_DATA_TRANSMISSION_CONTROL_COMMAND_H
#define STOP_DATA_TRANSMISSION_CONTROL_COMMAND_H

#include "../tia_control_command.h"
#include "../tia_control_command_context.h"
#include "../data_server.h"

namespace tia
{

//-----------------------------------------------------------------------------
class StopDataTransmissionControlCommand : public TiAControlCommand
{
public:
    StopDataTransmissionControlCommand (TiAControlCommandContext& command_context, DataServer& data_server)
        : command_context_ (command_context), data_server_ (data_server)
    {}

    virtual ~StopDataTransmissionControlCommand () {}

    virtual TiAControlMessage execute (TiAControlMessage const& command);

private:
    TiAControlCommandContext& command_context_;
    DataServer& data_server_;
};

}
#endif // STOP_DATA_TRANSMISSION_CONTROL_COMMAND_H

#ifndef GET_METAINFO_CONTROL_COMMAND_H
#define GET_METAINFO_CONTROL_COMMAND_H

#include "../tia_control_command.h"
#include "../tia_control_command_context.h"
#include "../data_server.h"

namespace tia
{

//-----------------------------------------------------------------------------
class GetMetaInfoControlCommand : public TiAControlCommand
{
public:
    GetMetaInfoControlCommand (TiAControlCommandContext& command_context, DataServer& data_server)
        : command_context_ (command_context), data_server_ (data_server)
    {}

    virtual ~GetMetaInfoControlCommand () {}

    virtual TiAControlMessage execute (TiAControlMessage const& command);

private:
};

}

#endif // GET_METAINFO_CONTROL_COMMAND_H

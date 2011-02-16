#ifndef GET_SERVERSTATE_CONNECTION_COMMAND_H
#define GET_SERVERSTATE_CONNECTION_COMMAND_H

#include "../tia_control_command.h"
#include "../string_utils.h"
#include "../messages/tia_control_message_tags_1_0.h"

namespace tia
{

//-----------------------------------------------------------------------------
class GetServerStateConnectionCommand : public TiAControlCommand
{
public:
    GetServerStateConnectionCommand (unsigned state_server_port)
        : state_server_port_ (state_server_port)
    {}

    virtual ~GetServerStateConnectionCommand () {}

    virtual TiAControlMessage execute (TiAControlMessage const& command)
    {
        return TiAControlMessage (command.getVersion(), TiAControlMessageTags10::SERVER_STATE_CONNECTION_PORT,
                                  toString (state_server_port_), "");
    }

private:
    unsigned state_server_port_;
};

}

#endif // GET_SERVERSTATE_CONNECTION_COMMAND_H

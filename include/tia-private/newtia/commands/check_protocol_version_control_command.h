#ifndef CHECK_PROTOCOL_VERSION_CONTROL_COMMAND_H
#define CHECK_PROTOCOL_VERSION_CONTROL_COMMAND_H

#include "../tia_control_command.h"
#include "../messages/tia_control_message_tags_1_0.h"
#include "../messages/standard_control_messages.h"

namespace tia
{

class CheckProtocolVersionControlCommand : public TiAControlCommand
{
public:
    virtual ~CheckProtocolVersionControlCommand () {}

    virtual TiAControlMessage execute (TiAControlMessage const& command)
    {
        if (command.getVersion () == TiAControlMessageTags10::VERSION)
            return OkControlMessage (TiAControlMessageTags10::VERSION);
        else
            return ErrorControlMessage (TiAControlMessageTags10::VERSION);
    }
};

}


#endif // CHECK_PROTOCOL_VERSION_CONTROL_COMMAND_H

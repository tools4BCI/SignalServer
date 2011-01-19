#include "tia-private/server/commands/get_data_connection_server_command.h"
#include "tia-private/server/version_1_0/tia_control_message_tags_1_0.h"


namespace tia
{

//-------------------------------------------------------------------------------------------------
TiAControlMessage GetDataConnectionControlCommand::execute (TiAControlMessage const& command)
{
    if (command.getParameters() == TiAControlMessageTags10::UDP)
    {
        data_server_.addConnection (true);
    }
}

}

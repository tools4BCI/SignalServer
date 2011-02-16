#include "tia-private/newtia/commands/start_data_transmission_control_command.h"
#include "tia-private/newtia/messages/tia_control_message_tags_1_0.h"
#include "tia-private/newtia/messages/standard_control_messages.h"

#include <sstream>

using std::string;

namespace tia
{

//-------------------------------------------------------------------------------------------------
TiAControlMessage StartDataTransmissionControlCommand::execute (TiAControlMessage const& command)
{
    string version = command.getVersion ();
    if (!command_context_.hasDataConnection ())
        return ErrorControlMessage (version);

    data_server_.startTransmission (command_context_.getDataConnectionID ());
    return OkControlMessage (version);
}

}

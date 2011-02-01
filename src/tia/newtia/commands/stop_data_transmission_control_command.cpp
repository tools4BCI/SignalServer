#include "tia-private/newtia/commands/stop_data_transmission_control_command.h"
#include "tia-private/newtia/version_1_0/tia_control_message_tags_1_0.h"
#include "tia-private/newtia/messages/standard_control_messages.h"

using std::string;

namespace tia
{

//-------------------------------------------------------------------------------------------------
TiAControlMessage StopDataTransmissionControlCommand::execute (TiAControlMessage const& command)
{
    string version = command.getVersion ();
    if (!command_context_.hasDataConnection ())
        return ErrorControlMessage (version);

    data_server_.stopTransmission (command_context_.getDataConnectionID ());
    return OkControlMessage (version);
}

}


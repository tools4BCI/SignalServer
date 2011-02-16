#include "tia-private/newtia/commands/get_data_connection_control_command.h"
#include "tia-private/newtia/messages/tia_control_message_tags_1_0.h"
#include "tia-private/newtia/messages/standard_control_messages.h"

#include <sstream>

using std::string;

namespace tia
{

//-------------------------------------------------------------------------------------------------
TiAControlMessage GetDataConnectionControlCommand::execute (TiAControlMessage const& command)
{
    string version = command.getVersion ();
    if (command_context_.hasDataConnection ())
        return ErrorControlMessage (version);

    string command_response = TiAControlMessageTags10::DATA_CONNECTION_PORT;

    string parameter;
    string content;

    ConnectionID data_connection;
    if (command.getParameters() == TiAControlMessageTags10::UDP)
        data_connection = data_server_.addConnection (true);
    else if (command.getParameters() == TiAControlMessageTags10::TCP)
        data_connection = data_server_.addConnection (false);
    else
        return ErrorControlMessage (version);

    command_context_.setDataConnectionID (data_connection);
    std::ostringstream oss;
    oss << data_server_.localPort (data_connection);
    parameter = oss.str ();

    return TiAControlMessage (version, command_response, parameter, content);
}

}

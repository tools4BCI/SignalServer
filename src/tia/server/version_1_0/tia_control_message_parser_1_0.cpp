#include "tia-private/server/version_1_0/tia_control_message_parser_1_0.h"
#include "tia-private/server/version_1_0/tia_control_message_tags_1_0.h"

using std::string;
using std::pair;

namespace tia
{

unsigned const TiAControlMessagParser10::MAX_LINE_LENGTH_ = 1000;

//-----------------------------------------------------------------------------
TiAControlMessage TiAControlMessagParser10::parseMessage (InputStream& stream) const
{
    string version = readVersion (stream);
    pair<string, string> command_and_parameter = readCommandAndParameter (stream);
    string content = readContent (stream);
    return TiAControlMessage (version, command_and_parameter.first, command_and_parameter.second, content);
}


//-----------------------------------------------------------------------------
string TiAControlMessagParser10::readVersion (InputStream& stream) const
{
    return "";
}

//-----------------------------------------------------------------------------
pair<string, string> TiAControlMessagParser10::readCommandAndParameter (InputStream& stream) const
{
    return pair<string, string> ("", "");
}

//-----------------------------------------------------------------------------
string TiAControlMessagParser10::readContent (InputStream& stream) const
{
    string content_length = stream.readLine (MAX_LINE_LENGTH_);
    if (content_length.size () == 0)
        return "";

    // TODO: parse content
    return "";
}


}

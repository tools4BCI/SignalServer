#include "tia-private/server/version_1_0/tia_control_message_parser_1_0.h"
#include "tia-private/server/version_1_0/tia_control_message_tags_1_0.h"
#include "tia-private/server/tia_exceptions.h"
#include "tia-private/server/string_utils.h"

#include <sstream>

using std::string;
using std::pair;

namespace tia
{

unsigned const TiAControlMessageParser10::MAX_LINE_LENGTH_ = 1000;

//-----------------------------------------------------------------------------
TiAControlMessage TiAControlMessageParser10::parseMessage (InputStream& stream) const
{
    string version = readVersion (stream);
    pair<string, string> command_and_parameter = readCommandAndParameter (stream);
    string content = readContent (stream);
    return TiAControlMessage (version, command_and_parameter.first, command_and_parameter.second, content);
}

//-----------------------------------------------------------------------------
string TiAControlMessageParser10::readVersion (InputStream& stream) const
{
    string id_and_version = trim (stream.readLine (MAX_LINE_LENGTH_));

    if (id_and_version != TiAControlMessageTags10::ID_AND_VERSION)
        throw TiAException (string ("TiAControlMessageParser10::readVersion Wrong Message ID and Version. Required \"") + TiAControlMessageTags10::ID_AND_VERSION + "\" but was \"" + id_and_version + "\".");

    return TiAControlMessageTags10::VERSION;
}

//-----------------------------------------------------------------------------
pair<string, string> TiAControlMessageParser10::readCommandAndParameter (InputStream& stream) const
{
    string line = trim (stream.readLine (MAX_LINE_LENGTH_));
    return getPair (line);
}

//-----------------------------------------------------------------------------
string TiAControlMessageParser10::readContent (InputStream& stream) const
{
    string line = trim (stream.readLine (MAX_LINE_LENGTH_));
    if (line.size () == 0)
        return "";

    pair<string, string> content_length_pair = getPair (line);
    if (content_length_pair.first != TiAControlMessageTags10::CONTENT_LENGTH)
        throw TiAException (string ("TiAControlMessageParser10::readContent: Invalid field. Expected \"") + TiAControlMessageTags10::CONTENT_LENGTH + "\" but was \"" + content_length_pair.first + "\"");

    std::istringstream isstr (content_length_pair.second);
    size_t length = 0;
    isstr >> length;
    if (isstr.fail ())
        throw TiAException (string ("TiAControlMessageParser10::readContent: Error while converting \"") + content_length_pair.second + "\" into a number.");

    string empty_line = stream.readLine (MAX_LINE_LENGTH_);
    if (empty_line.size ())
        throw TiAException (string ("TiAControlMessageParser10::readContent: Expecting an empty line before content starts."));

    string content = stream.readString (length);
    if (content.size () != length)
        throw TiAException (string ("TiAControlMessageParser10::readContent: Missing bytes!"));

    return content;
}


//-----------------------------------------------------------------------------
pair<string, string> TiAControlMessageParser10::getPair (std::string const& str) const
{
    size_t delimiter_index = 0;
    while ((str[delimiter_index] != TiAControlMessageTags10::COMMAND_DELIMITER) &&
           delimiter_index < str.size ())
        delimiter_index++;

    string tag = str.substr (0, delimiter_index);
    string value;
    if (delimiter_index < str.size () - 1)
        value = trim (str.substr (delimiter_index + 1, str.size () + 1 - delimiter_index));

    return pair<string, string> (tag, value);
}


}

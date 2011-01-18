#ifndef TIA_CONTROL_MESSAGES_DEFINITION_H
#define TIA_CONTROL_MESSAGES_DEFINITION_H

#include "UnitTest++/UnitTest++.h"
#include <string>

//-----------------------------------------------------------------------------
const std::string XML_DECLARATION = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
const std::string MESSAGE_VERSION = "0.1";


//-----------------------------------------------------------------------------
std::string message (std::string const& child_tag)
{
    std::string message = "<message version=\"" + MESSAGE_VERSION + "\">";
    message += child_tag;
    message += "</message>";
    return message;
}

//-----------------------------------------------------------------------------
/// stopDataTransmission Message
///
/// <?xml version="1.0" encoding="UTF-8"?>
/// <message version="0.2">
///   <stopTransmission />
/// </message>
///
const std::string STOP_DATA_TRANSMISSION_MESSAGE = XML_DECLARATION + message ("<stopTransmission />");

//-----------------------------------------------------------------------------
/// okReply Message
///
/// <?xml version="1.0" encoding="UTF-8"?>
/// <message version="0.2">
///   <okReply />
/// </message>
///
const std::string OK_REPLY_MESSAGE = XML_DECLARATION + message ("<okReply />");

#endif // TIA_CONTROL_MESSAGES_DEFINITION_H

/*
    This file is part of the TOBI SignalServer test routine.

    The TOBI SignalServer test routine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI SignalServer test routine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI SignalServer test routine. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

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

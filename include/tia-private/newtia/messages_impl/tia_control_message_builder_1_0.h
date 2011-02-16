#ifndef TIA_CONTROL_MESSAGE_BUILDER_1_0_H
#define TIA_CONTROL_MESSAGE_BUILDER_1_0_H

#include "../messages/tia_control_message_tags_1_0.h"
#include "../messages/tia_control_message_builder.h"
#include "../tia_exceptions.h"

#include <sstream>

namespace tia
{

//-----------------------------------------------------------------------------
class TiAControlMessageBuilder10 : public TiAControlMessageBuilder
{
public:
    virtual std::string buildTiAMessage (TiAControlMessage const& message)
    {
        std::string str = "TiA ";
        str += message.getVersion ();
        str += TiAControlMessageTags10::NEW_LINE;

        str += message.getCommand ();
        if (message.getParameters().size())
        {
            str += TiAControlMessageTags10::COMMAND_DELIMITER;
            str += message.getParameters ();
        }
        str += TiAControlMessageTags10::NEW_LINE;

        if (message.getContent().size())
        {
            str += TiAControlMessageTags10::CONTENT_LENGTH;
            str += TiAControlMessageTags10::COMMAND_DELIMITER;

            std::ostringstream osstr;
            osstr << message.getContent().size();
            if (osstr.fail())
                throw TiAException ("TiAControlMessageBuilder10::buildTiAMessage failed to convert Content length into a string.");
            str += osstr.str();
            str += TiAControlMessageTags10::NEW_LINE;
        }

        str += TiAControlMessageTags10::NEW_LINE;
        str += message.getContent ();
        return str;
    }

};

}

#endif // TIA_CONTROL_MESSAGE_BUILDER_1_0_H

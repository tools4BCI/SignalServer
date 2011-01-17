#ifndef TIA_CONTROL_MESSAGE_PARSER_1_0_H
#define TIA_CONTROL_MESSAGE_PARSER_1_0_H

#include "../tia_control_message_parser.h"

namespace tia
{

class TiAControlMessagParser10 : public TiAControlMessageParser
{
public:
    virtual ~TiAControlMessagParser10 () {}
    TiAControlMessage parseMessage (InputStream& stream) const;

private:
    static unsigned const MAX_LINE_LENGTH_;

    std::string readVersion (InputStream& stream) const;
    std::pair<std::string, std::string> readCommandAndParameter (InputStream& stream) const;
    std::string readContent (InputStream& stream) const;
};

}

#endif // TIA_CONTROL_MESSAGE_PARSER_1_0_H

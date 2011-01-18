#ifndef TIA_CONTROL_MESSAGE_BUILDER_H
#define TIA_CONTROL_MESSAGE_BUILDER_H

#include "tia_control_message.h"

namespace tia
{

//-----------------------------------------------------------------------------
class TiAControlMessageBuilder
{
public:
    virtual ~TiAControlMessageBuilder () {}

    virtual std::string buildTiAMessage (TiAControlMessage const& message) = 0;
};


}

#endif // TIA_CONTROL_MESSAGE_BUILDER_H

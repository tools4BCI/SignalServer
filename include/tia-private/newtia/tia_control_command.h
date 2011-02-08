#ifndef TIA_CONTROL_COMMAND_H
#define TIA_CONTROL_COMMAND_H

#include "tia_control_message.h"

namespace tia
{


//-----------------------------------------------------------------------------
class TiAControlCommand
{
public:
    virtual ~TiAControlCommand () {}

    virtual TiAControlMessage execute (TiAControlMessage const& command) = 0;
};


}

#endif // TIA_CONTROL_COMMAND_H

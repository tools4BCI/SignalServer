#ifndef TIA_CONTROL_MESSAGE_PARSER_H
#define TIA_CONTROL_MESSAGE_PARSER_H

#include "tia_control_message.h"
#include "../network/input_stream.h"

namespace tia
{

//-----------------------------------------------------------------------------
/// TiAControlMessageParser
///
/// base class for a TiAControlMessageParser
class TiAControlMessageParser
{
public:
    virtual ~TiAControlMessageParser () {}

    //-------------------------------------------------------------------------
    /// @throw exceptions if error occurs
    virtual TiAControlMessage parseMessage (InputStream& socket) const = 0;


};


}

#endif // TIA_CONTROL_MESSAGE_PARSER_H

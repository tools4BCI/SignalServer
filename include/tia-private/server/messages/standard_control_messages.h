#ifndef STANDARD_CONTROL_MESSAGES_H
#define STANDARD_CONTROL_MESSAGES_H

#include "../tia_control_message.h"

namespace tia
{

//-----------------------------------------------------------------------------
class OkControlMessage : public TiAControlMessage
{
public:
    OkControlMessage (std::string const& version) : TiAControlMessage (version, "OK", "", "")
    {}
    virtual ~OkControlMessage () {}
};

//-----------------------------------------------------------------------------
class ErrorControlMessage : public TiAControlMessage
{
public:
    ErrorControlMessage (std::string const& version) : TiAControlMessage (version, "Error", "", "")
    {}
    virtual ~ErrorControlMessage () {}
};

}

#endif // STANDARD_CONTROL_MESSAGES_H

#ifndef STANDARD_CONTROL_MESSAGES_H
#define STANDARD_CONTROL_MESSAGES_H

#include "../messages/tia_control_message.h"
#include "../messages/tia_control_message_tags_1_0.h"

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

//-----------------------------------------------------------------------------
class CheckProtocolVersionTiAControlMessage : public TiAControlMessage
{
public:
    CheckProtocolVersionTiAControlMessage (std::string const& version) : TiAControlMessage (version, TiAControlMessageTags10::CHECK_PROTOCOL_VERSION, "", "")
    {}
    virtual ~CheckProtocolVersionTiAControlMessage () {}
};

//-----------------------------------------------------------------------------
class GetMetaInfoTiAControlMessage : public TiAControlMessage
{
public:
    GetMetaInfoTiAControlMessage (std::string const& version) : TiAControlMessage (version, TiAControlMessageTags10::GET_METAINFO, "", "")
    {}
    virtual ~GetMetaInfoTiAControlMessage () {}
};

//-----------------------------------------------------------------------------
class GetDataConnectionTiAControlMessage : public TiAControlMessage
{
public:
    GetDataConnectionTiAControlMessage (std::string const& version, bool udp)
        : TiAControlMessage (version, TiAControlMessageTags10::GET_DATA_CONNECTION, udp ? TiAControlMessageTags10::UDP : TiAControlMessageTags10::TCP, "")
    {}
    virtual ~GetDataConnectionTiAControlMessage () {}
};


}

#endif // STANDARD_CONTROL_MESSAGES_H

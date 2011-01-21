#ifndef GET_METAINFO_CONTROL_COMMAND_H
#define GET_METAINFO_CONTROL_COMMAND_H

#include "../tia_control_command.h"
#include "../tia_control_command_context.h"
#include "../data_server.h"
#include "../tia_meta_info_parse_and_build_functions.h"

namespace tia
{

//-----------------------------------------------------------------------------
class GetMetaInfoControlCommand : public TiAControlCommand
{
public:
    GetMetaInfoControlCommand (TiAControlCommandContext& command_context)
        : command_context_ (command_context)
    {}

    virtual ~GetMetaInfoControlCommand () {}

    virtual TiAControlMessage execute (TiAControlMessage const& command)
    {
        std::string xml = buildTiAMetaInfoXMLString (command_context_.getHardwareInterface().getTiAMetaInfo());
        return TiAControlMessage (command.getVersion(), "MetaInfo", "", xml);
    }

private:
    TiAControlCommandContext& command_context_;
};

}

#endif // GET_METAINFO_CONTROL_COMMAND_H

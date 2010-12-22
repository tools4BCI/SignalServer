#include "tia-private/server/server_command.h"
#include "tia-private/server/socket.h"

namespace tia
{

//-------------------------------------------------------------------------
ServerCommand::ServerCommand (WriteSocket& socket) : socket_ (socket)
{
    // nothing to do here
}

//-------------------------------------------------------------------------
void ServerCommand::execute ()
{
    socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><errorReply  /></message>");
}

//-------------------------------------------------------------------------
void ServerCommand::executeAttributes (std::map<std::string, std::string> const& /*attributes*/)
{
    socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><errorReply  /></message>");
}

//-------------------------------------------------------------------------
void ServerCommand::executeContent (std::string const& /*content*/)
{
    socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><errorReply  /></message>");
}

//-------------------------------------------------------------------------
void ServerCommand::executeAttributesContent (std::map<std::string, std::string> const& /*attributes*/, std::string const& /*content*/)
{
    socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><errorReply  /></message>");
}

}

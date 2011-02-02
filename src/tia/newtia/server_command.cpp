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
void ServerCommand::executeWithParameter (std::string const& /*parameter*/)
{
    socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><errorReply  /></message>");
}

//-------------------------------------------------------------------------
void ServerCommand::executeWithContent (std::string const& /*content*/)
{
    socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><errorReply  /></message>");
}

//-------------------------------------------------------------------------
void ServerCommand::executeWithParameterAndContent (std::string const& /*parameter*/, std::string const& /*content*/)
{
    socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><errorReply  /></message>");
}

}

#ifndef SERVER_COMMAND_H
#define SERVER_COMMAND_H

#include "socket.h"

#include <map>
#include <string>

namespace tia
{

//-----------------------------------------------------------------------------
class ServerCommand
{
public:
    //-------------------------------------------------------------------------
    ServerCommand (WriteSocket& socket) : socket_ (socket) {}

    //-------------------------------------------------------------------------
    virtual void execute ()
    {
        socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><errorReply  /></message>");
    }

    //-------------------------------------------------------------------------
    virtual void execute (std::map<std::string, std::string> const& attributes)
    {
        socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><errorReply  /></message>");
    }

    //-------------------------------------------------------------------------
    virtual void execute (std::map<std::string, std::string> const& attributes, std::string const& content)
    {
        socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><errorReply  /></message>");
    }

private:
    WriteSocket& socket_;
};


}

#endif // SERVER_COMMAND_H

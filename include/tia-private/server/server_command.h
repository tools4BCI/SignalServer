#ifndef SERVER_COMMAND_H
#define SERVER_COMMAND_H

#include <map>
#include <string>

namespace tia
{

class WriteSocket;

//-----------------------------------------------------------------------------
class ServerCommand
{
public:
    //-------------------------------------------------------------------------
    ServerCommand (WriteSocket& socket);

    //-------------------------------------------------------------------------
    virtual ~ServerCommand () {}

    //-------------------------------------------------------------------------
    /// use if command has no attributes or content (e.g. start data transmission)
    virtual void execute ();

    //-------------------------------------------------------------------------
    /// use if command takes attributes (e.g. get data connection: type = tcp/udp)
    virtual void executeAttributes (std::map<std::string, std::string> const& /*attributes*/);

    //-------------------------------------------------------------------------
    /// use if command takes content (e.g. get config)
    virtual void executeContent (std::string const& /*content*/);

    //-------------------------------------------------------------------------
    /// use if command takes attributes and content
    virtual void executeAttributesContent (std::map<std::string, std::string> const& /*attributes*/, std::string const& /*content*/);

private:
    WriteSocket& socket_;
};


}

#endif // SERVER_COMMAND_H

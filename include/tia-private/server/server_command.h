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
    /// use if command takes parameters (e.g. GetDataConnection)
    virtual void executeWithParameter (std::string const& /*parameter*/);

    //-------------------------------------------------------------------------
    /// use if command takes content (e.g. get config)
    virtual void executeWithContent (std::string const& /*content*/);

    //-------------------------------------------------------------------------
    virtual void executeWithParameterAndContent (std::string const& /*parameter*/, std::string const& /*content*/);

private:
    WriteSocket& socket_;
};


}

#endif // SERVER_COMMAND_H

#ifndef TIA_CONTROL_MESSAGE_H
#define TIA_CONTROL_MESSAGE_H

#include <string>

namespace tia
{

//-----------------------------------------------------------------------------
class TiAControlMessage
{
public:
    TiAControlMessage (std::string const& version, std::string const& command,
                       std::string const& parameters, std::string const& content)
                           : version_ (version), command_ (command),
                             parameters_ (parameters), content_ (content)
    {}
    virtual ~TiAControlMessage () {}

    std::string getVersion () const {return version_;}
    std::string getCommand () const {return command_;}
    std::string getParameters () const {return parameters_;}
    std::string getContent () const {return content_;}

private:
    std::string version_;
    std::string command_;
    std::string parameters_;
    std::string content_;
};

}

#endif // TIA_CONTROL_MESSAGE_H

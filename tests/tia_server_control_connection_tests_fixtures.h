#ifndef TIA_SERVER_CONTROL_CONNECTION_TESTS_FIXTURES_H
#define TIA_SERVER_CONTROL_CONNECTION_TESTS_FIXTURES_H

#include "tia-private/server/socket.h"

#include "UnitTest++/UnitTest++.h"



//-----------------------------------------------------------------------------
class TestSocket : public tia::Socket
{
public:
    virtual void sendString (std::string const& string) {sent_string_ = string;}
    virtual std::string readString () {return string_to_return_on_read_;}

    std::string sentString () {return sent_string_;}
    void setStringToBeRead (std::string const& read_string) {string_to_return_on_read_ = read_string;}

private:
    std::string sent_string_;
    std::string string_to_return_on_read_;
};

#endif // TIA_SERVER_CONTROL_CONNECTION_TESTS_FIXTURES_H

#ifndef TEST_SOCKET_H
#define TEST_SOCKET_H

#include "tia-private/server/socket.h"

//-----------------------------------------------------------------------------
/// TestSocket
///
/// test implementation of the Socket class
class TestSocket : public tia::Socket
{
public:
    virtual void sendString (std::string const& string) {sent_string_ += string;}

    virtual std::string readString (unsigned number_bytes = 4000000000u);
    virtual std::string readLine (unsigned max_length);
    virtual char readCharacter ();

    std::string transmittedString () {return sent_string_;}
    void setStringToBeRead (std::string const& read_string) {string_to_return_on_read_ = read_string;}
    void reset () {sent_string_.clear (); string_to_return_on_read_.clear ();}

private:
    std::string sent_string_;
    std::string string_to_return_on_read_;
};

#endif // TEST_SOCKET_H

#include "test_socket.h"
#include "tia-private/server/version_1_0/tia_control_message_tags_1_0.h"
#include "tia-private/server/tia_exceptions.h"

using std::string;
using namespace tia::TiAControlMessageTags10;

//-----------------------------------------------------------------------------
string TestSocket::readString (unsigned number_bytes)
{
    string ret_val = "";

    if ((number_bytes == 4000000000u) ||
        (number_bytes >= string_to_return_on_read_.size ()))
        ret_val = string_to_return_on_read_;
    else
        ret_val = string_to_return_on_read_.substr (0, number_bytes);

    string_to_return_on_read_.erase (0, ret_val.size ());
    return ret_val;
}

//-----------------------------------------------------------------------------
string TestSocket::readLine (unsigned max_length)
{
    string ret_val;
    size_t new_line_pos = 0;
    while ((string_to_return_on_read_.substr (new_line_pos, NEW_LINE.size ()) != NEW_LINE) &&
           (new_line_pos < string_to_return_on_read_.size ()))
        new_line_pos++;
    if (string_to_return_on_read_.substr (new_line_pos, NEW_LINE.size ()) == NEW_LINE)
        ret_val = readString (new_line_pos);
    else
        throw tia::TiAException ("TestSocket::readLine: No NewLine Character in given string.");

    string_to_return_on_read_.erase (0, 1);

    return ret_val;
}

//-----------------------------------------------------------------------------
char TestSocket::readCharacter ()
{
    char ret_val = 0;
    if (string_to_return_on_read_.size ())
    {
        ret_val = string_to_return_on_read_[0];
        string_to_return_on_read_.erase (0, 1);
    }
    else
        throw tia::TiAException ("TestSocket::readLine: No NewLine Character in given string.");
    return ret_val;
}

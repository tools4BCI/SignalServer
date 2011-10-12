/*
    This file is part of the TOBI SignalServer test routine.

    The TOBI SignalServer test routine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI SignalServer test routine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI SignalServer test routine. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

#include "test_socket.h"
#include "tia-private/newtia/messages/tia_control_message_tags_1_0.h"
#include "tia-private/newtia/tia_exceptions.h"

#include <boost/thread/locks.hpp>

using std::string;
using namespace tia::TiAControlMessageTags10;


//-----------------------------------------------------------------------------
void TestSocket::sendString (std::string const& string) throw (tia::TiALostConnection)
{
    if (failure_if_sending_)
        throw tia::TiALostConnection ("Lost connection simulation.");

    boost::lock_guard<boost::mutex> lock (transmitted_string_lock_);
    sent_string_ += string;
    wait_for_data_condition_.notify_all ();
}


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
string TestSocket::readUntil (char delimiter)
{
    std::string delim;
    delim = delimiter;
    string ret_val;
    size_t new_line_pos = 0;
    while ((string_to_return_on_read_.substr (new_line_pos, 1) != delim) &&
           (new_line_pos < string_to_return_on_read_.size ()))
        new_line_pos++;
    if (string_to_return_on_read_.substr (new_line_pos, 1) == delim)
        ret_val = readString (new_line_pos);
    else
        throw tia::TiAException ("TestSocket::readUntil (char delimiter): No NewLine Character in given string.");

    string_to_return_on_read_.erase (0, 1);

    return ret_val;
}

//-----------------------------------------------------------------------------
string TestSocket::readUntil (std::string delimiter)
{
    string ret_val;
    size_t new_line_pos = 0;
    while ((string_to_return_on_read_.substr (new_line_pos, delimiter.size ()) != delimiter) &&
           (new_line_pos < string_to_return_on_read_.size ()))
        new_line_pos++;
    if (string_to_return_on_read_.substr (new_line_pos, delimiter.size ()) == delimiter)
        ret_val = readString (new_line_pos);
    else
        throw tia::TiAException ("TestSocket::readUntil (std::string delimiter): No NewLine Character in given string.");

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

//-----------------------------------------------------------------------------
void TestSocket::waitForData ()
{
    boost::unique_lock<boost::mutex> lock (transmitted_string_lock_);

    if (sent_string_.size())
        return;
    else
        wait_for_data_condition_.wait (lock);
}


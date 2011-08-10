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

#include "../UnitTest++/UnitTest++.h"

#include "../test_socket.h"
#include "tia-private/newtia/messages_impl/tia_control_message_parser_1_0.h"
#include "tia-private/newtia/tia_exceptions.h"

using namespace tia;
using namespace std;

SUITE (TiAVersion10)
{

TEST(TiAControlMessageParser10_parseMessage)
{
    TiAControlMessageParser* parser = new TiAControlMessageParser10 ();

    TestSocket socket;

    socket.setStringToBeRead ("TiA 1.0\nGetConfig\n\n");
    TiAControlMessage message = parser->parseMessage (socket);
    CHECK_EQUAL (string ("1.0"), message.getVersion());
    CHECK_EQUAL (string ("GetConfig"), message.getCommand());
    CHECK_EQUAL (0u, message.getParameters().size());
    CHECK_EQUAL (0u, message.getContent().size());

    socket.setStringToBeRead ("TiA 1.0  \n AnyCommand: AnyParameter \n  Content-Length:  11 \n\nTestContent");
    TiAControlMessage any_message = parser->parseMessage (socket);
    CHECK_EQUAL (string ("1.0"), any_message.getVersion());
    CHECK_EQUAL (string ("AnyCommand"), any_message.getCommand());
    CHECK_EQUAL (string ("AnyParameter"), any_message.getParameters());
    CHECK_EQUAL (string ("TestContent"), any_message.getContent());

    socket.reset ();
    socket.setStringToBeRead ("Tia 1.0  \n WrongID \n\n");
    CHECK_THROW (parser->parseMessage (socket), TiAException);

    socket.reset ();
    socket.setStringToBeRead ("TiA 0.1  \n WrongVersion \n\n");
    CHECK_THROW (parser->parseMessage (socket), TiAException);

    delete parser;
}

}

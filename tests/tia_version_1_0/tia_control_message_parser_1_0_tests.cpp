#include "../UnitTest++/UnitTest++.h"

#include "../test_socket.h"
#include "tia-private/server/version_1_0/tia_control_message_parser_1_0.h"
#include "tia-private/server/tia_exceptions.h"

using namespace tia;
using namespace std;

SUITE (TiAVersion10)
{

TEST(TiAControlMessageParser10_parseMessage)
{
    TiAControlMessageParser* parser = new TiAControlMessagParser10 ();

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

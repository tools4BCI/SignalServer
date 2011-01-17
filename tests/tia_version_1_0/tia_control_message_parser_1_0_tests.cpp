#include "../UnitTest++/UnitTest++.h"

#include "tia-private/server/version_1_0/tia_control_message_parser_1_0.h"

using namespace tia;

SUITE (TiAVersion10)
{

TEST(TiAControlMessageParser10_parseMessage)
{
    TiAControlMessageParser* parser = new TiAControlMessagParser10 ();

    // parser->parseMessage ();


    delete parser;
}

}

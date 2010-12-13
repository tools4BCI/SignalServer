#include "UnitTest++/UnitTest++.h"
#include "tia_server_control_connection_tests_fixtures.h"
#include "tia_control_messages_definition.h"

#include "tia-private/server/stop_data_transmission_server_command.h"

#include "ticpp/ticpp.h"

#include <iostream>

using namespace tia;

SUITE (ServerControlConnectionTests)
{

//-----------------------------------------------------------------------------
bool validMessage02 (std::string xml)
{
    std::string XML_VERSION = "1.0";
    try
    {
        ticpp::Document reply;
        reply.Parse (xml, true, TIXML_ENCODING_UTF8);
        ticpp::Declaration* declaration = reply.FirstChild ()->ToDeclaration ();
        if (declaration->Version() != "1.0")
        {
            std::cerr << "wrong xml version" << std::endl;
            return false;
        }
        if (declaration->Encoding() != "UTF-8")
        {
            std::cerr << "wrong xml encoding" << std::endl;
            return false;
        }

        ticpp::Element* message_tag = reply.FirstChildElement();
        if (message_tag->Value() != "message")
        {
            std::cerr << "wrong message tag" << std::endl;
            return false;
        }
        if (message_tag->GetAttribute<std::string>("version") != "0.2")
        {
            std::cerr << "wrong message version" << std::endl;
            return false;
        }

        if (message_tag->NextSibling (false))
        {
            std::cerr << "the message tag is not the only first level tag" << std::endl;
            return false;
        }

        return true;
    }
    catch (ticpp::Exception &exc)
    {
        std::cerr << exc.what() << std::endl;
        return false;
    }
}

//-----------------------------------------------------------------------------
std::string message02Command (std::string const& xml)
{
    ticpp::Document reply;
    reply.Parse (xml);
    return reply.FirstChildElement()->FirstChildElement()->Value();
}

//-----------------------------------------------------------------------------
TEST_FIXTURE (TiAServerControlConnectionFixture, stopDataTransmissionServerCommand)
{
    test_control_socket.setStringToBeRead ("");
    ConnectionID valid_connection_id = test_data_server.addConnection ();
    ConnectionID invalid_connection_id = valid_connection_id + 100;

    StopDataTransmissionServerCommand command (valid_connection_id, test_data_server, test_control_socket);

    command.execute ();

    // error reply if transmission already not active
    CHECK (validMessage02 (test_control_socket.transmittedString()));
    CHECK_EQUAL (std::string ("errorReply"), message02Command (test_control_socket.transmittedString()));
    CHECK (!test_data_server.transmissionEnabled (valid_connection_id));

    test_data_server.startTransmission (valid_connection_id);

    command.execute ();

    // ok reply if transmission has been active and could be stopped
    CHECK (validMessage02 (test_control_socket.transmittedString()));
    CHECK_EQUAL (std::string ("okReply"), message02Command (test_control_socket.transmittedString()));
    CHECK (!test_data_server.transmissionEnabled (valid_connection_id));


    // no reply if connection doesn't even exist
    StopDataTransmissionServerCommand command_with_invalid_id (invalid_connection_id, test_data_server, test_control_socket);
    command_with_invalid_id.execute ();
    CHECK_EQUAL (static_cast<unsigned>(0), test_control_socket.transmittedString().size());
}

}

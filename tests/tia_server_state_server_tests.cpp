#include "UnitTest++/UnitTest++.h"
#include "test_socket.h"

#include "tia-private/newtia/tia_server_state_server.h"
#include "tia-private/newtia/tia_server_state_server_impl.h"

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <iostream>

using namespace tia;
using namespace std;

SUITE(ServerStateServer)
{

unsigned const PORT = 9001;

//-----------------------------------------------------------------------------
void testServerStateServer (boost::shared_ptr<TiAServerStateServer> server, boost::shared_ptr<TestTCPServerSocket> test_server_socket)
{
    CHECK_EQUAL (server->getPort(), PORT);
    boost::shared_ptr<TestSocket> server_test_socket = test_server_socket->clientConnects ();
    server_test_socket->waitForData ();
    CHECK (server_test_socket->transmittedString ().find ("TiA") != std::string::npos);
    CHECK (server_test_socket->transmittedString ().find ("ServerStateRunning") != std::string::npos);
    CHECK (server_test_socket->transmittedString ().find ("ServerStateShutdown") == std::string::npos);

    server->emitState (SERVER_STATE_SHUTDOWN);
    server_test_socket->waitForData ();

    CHECK (server_test_socket->transmittedString ().find ("TiA") != std::string::npos);
    CHECK (server_test_socket->transmittedString ().find ("ServerStateShutdown") != std::string::npos);

    server_test_socket->enableFailureIfSending (true);
    string transmitted_string_before_disconnect = server_test_socket->transmittedString();
    server->emitState (SERVER_STATE_RUNNING);
    CHECK_EQUAL (transmitted_string_before_disconnect, server_test_socket->transmittedString());
}

//-----------------------------------------------------------------------------
TEST (ServerStateServerImpl)
{
    boost::shared_ptr<TestTCPServerSocket> test_server_socket (new TestTCPServerSocket);
    boost::shared_ptr<TiAServerStateServer> server (new TiAServerStateServerImpl (test_server_socket, PORT));
    testServerStateServer (server, test_server_socket);
    test_server_socket->shutdown ();
}

}

#include "UnitTest++/UnitTest++.h"

#include "tia/tia_server.h"

#include <boost/asio.hpp>


using namespace tobiss;

//-----------------------------------------------------------------------------
TEST (tiaServerBasicCreationTest)
{
    boost::asio::io_service io_service;
    TiAServer server (io_service);

    std::map<std::string, std::string> subject_info;
    std::map<std::string, std::string> server_settings;

    server.initialize (subject_info, server_settings);
}

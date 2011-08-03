/*
    This file is part of the TOBI signal server.

    The TOBI signal server is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI signal server is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

#include "UnitTest++/UnitTest++.h"

#include "tia/tia_server.h"

#include <boost/asio.hpp>


using namespace tobiss;



//-----------------------------------------------------------------------------
TEST (tiaServerBasicCreationTest)
{
    boost::asio::io_service io_service;
    TiAServer server (io_service);

    // std::map<std::string, std::string> subject_info;
    // std::map<std::string, std::string> server_settings;

    // server.initialize (subject_info, server_settings);
}


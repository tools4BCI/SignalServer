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

#include "UnitTest++/UnitTest++.h"
#include "tia/tia_client.h"
#include "tia-private/datapacket/data_packet_impl.h"

#include <limits>

using namespace tia;

TEST(clientBasicConnection)
{
    TiAClient client(0);

    CHECK(client.connected() == false);
    CHECK(client.receiving() == false);

    CHECK_THROW(client.connect("0.0.0.0", 1000), std::ios_base::failure);
    CHECK(client.connected() == false);

    CHECK_THROW(client.requestConfig(), std::ios_base::failure);

    CHECK_THROW(client.startReceiving(false), std::ios_base::failure);
    CHECK_THROW(client.startReceiving(true), std::ios_base::failure);
    CHECK(client.receiving() == false);

    CHECK_THROW(client.stopReceiving(), std::ios_base::failure);

    client.disconnect();

    DataPacketImpl data_packet;
    CHECK_THROW(client.getDataPacket(data_packet), std::ios_base::failure);
}

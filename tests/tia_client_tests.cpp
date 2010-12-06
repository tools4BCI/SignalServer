#include "UnitTest++/UnitTest++.h"
#include "tia/tia_client.h"
#include "tia/data_packet.h"

#include <limits>

using namespace tobiss;

TEST(clientBasicConnection)
{
    TiAClient client;

    CHECK(client.connected() == false);
    CHECK(client.receiving() == false);

    CHECK_THROW(client.connect("0.0.0.0", 1000);, std::ios_base::failure);
    CHECK(client.connected() == false);

    CHECK_THROW(client.requestConfig(), std::ios_base::failure);

    CHECK_THROW(client.startReceiving(false), std::ios_base::failure);
    CHECK_THROW(client.startReceiving(true), std::ios_base::failure);
    CHECK(client.receiving() == false);

    CHECK_THROW(client.stopReceiving(), std::ios_base::failure);

    client.disconnect();

    DataPacket data_packet;
    CHECK_THROW(client.getDataPacket(data_packet), std::ios_base::failure);
}

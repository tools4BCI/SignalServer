#include "UnitTest++/UnitTest++.h"

#include "datapacket_tests_fixtures.h"

#include "tia/data_packet3.h"

#include <string>
#include <vector>

using namespace tobiss;
using std::vector;

//-------------------------------------------------------------------------------------------------
TEST(emptyDataPacket)
{
    DataPacket3 empty_packet;
    CHECK(empty_packet.getNrOfChannels().size() == 0);
    CHECK(empty_packet.getSamplesPerChannel().size() == 0);
    CHECK(empty_packet.getData().size() == 0);
    CHECK(empty_packet.getConnectionPacketNr() == 0);
}

//-------------------------------------------------------------------------------------------------
TEST_FIXTURE(DataPacketSetterGetterFixture, setterGetterDataPacket)
{
    DataPacket3 empty_packet;

    empty_packet.setConnectionPacketNr (packet_number);
    CHECK(empty_packet.getConnectionPacketNr() == packet_number);

    empty_packet.setConnectionPacketNr (packet_number);
    CHECK(empty_packet.getConnectionPacketNr() == packet_number);
}

//-------------------------------------------------------------------------------------------------
TEST_FIXTURE(DataPacketInsertingFixture, insertingDataPacket)
{
    DataPacket3 packet;

    packet.insertDataBlock (random_data, SIG_EEG, 1);
    CHECK(packet.hasFlag(SIG_EEG));
    CHECK(packet.hasFlag(SIG_EMG) == false);
}

//-------------------------------------------------------------------------------------------------
TEST_FIXTURE(DataPacketRawMemoryFixture, createVersion3DataPacket)
{
    DataPacket3 packet (reinterpret_cast<void*>(raw_data_packet_version_3));
}

//-------------------------------------------------------------------------------------------------
// NOT COMPLETED YET
//TEST_FIXTURE(DataPacketRawMemoryFixture, createVersion2DataPacket)
//{
//    DataPacket packet (reinterpret_cast<void*>(raw_data_packet_version_2));
//}


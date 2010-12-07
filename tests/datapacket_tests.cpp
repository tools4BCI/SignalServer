#include "UnitTest++/UnitTest++.h"

#include "datapacket_tests_fixtures.h"

#include "tia/data_packet.h"

#include <string>
#include <vector>

using namespace tobiss;
using std::vector;

//-------------------------------------------------------------------------------------------------
TEST(emptyDataPacket)
{
    DataPacket empty_packet;
    CHECK(empty_packet.getNrOfBlocks().size() == 0);
    CHECK(empty_packet.getNrOfValues().size() == 0);
    CHECK(empty_packet.getData().size() == 0);
    CHECK(empty_packet.getPacketNr() == 0);
}

//-------------------------------------------------------------------------------------------------
TEST_FIXTURE(DataPacketSetterGetterFixture, setterGetterDataPacket)
{
    DataPacket empty_packet;

    empty_packet.setPacketNr (packet_number);
    CHECK(empty_packet.getPacketNr() == packet_number);

    empty_packet.setPacketNr (packet_number);
    CHECK(empty_packet.getPacketNr() == packet_number);
}

//-------------------------------------------------------------------------------------------------
TEST_FIXTURE(DataPacketInsertingFixture, insertingDataPacket)
{
    DataPacket packet;

    packet.insertDataBlock (random_data, SIG_EEG, 1);
    CHECK(packet.hasFlag(SIG_EEG));
    CHECK(packet.hasFlag(SIG_EMG) == false);
}

//-------------------------------------------------------------------------------------------------
TEST_FIXTURE(DataPacketRawMemoryFixture, createVersion3DataPacket)
{
    DataPacket packet (reinterpret_cast<void*>(raw_data_packet_version_3));
}

//-------------------------------------------------------------------------------------------------
// NOT COMPLETED YET
//TEST_FIXTURE(DataPacketRawMemoryFixture, createVersion2DataPacket)
//{
//    DataPacket packet (reinterpret_cast<void*>(raw_data_packet_version_2));
//}


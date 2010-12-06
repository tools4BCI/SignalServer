#include "UnitTest++/UnitTest++.h"
#include "tia/data_packet.h"

using namespace tobiss;

struct DataPacketSetterGetterFixture
{
    DataPacketSetterGetterFixture ()
        : packet_number (5)
    {}
    unsigned packet_number;
};

struct DataPacketInsertingFixture
{
    DataPacketInsertingFixture ()
    {
        for (size_t i = 0; i < 100; ++i)
            random_data.push_back (i);
    }

    std::vector<double> random_data;

};

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

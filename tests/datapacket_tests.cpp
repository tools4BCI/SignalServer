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
TEST(assemblePacket)
{
    DataPacket3 empty_packet;
    vector<double> v0(25,0);
    vector<double> v1(5,1);
    vector<double> v2(16,2);
    vector<double> v3(12,3);

    empty_packet.insertDataBlock(v0,SIG_EMG,1);

    empty_packet.insertDataBlock(v1,SIG_EEG,1);   // insert
    empty_packet.insertDataBlock(v3,SIG_EEG,1,1);  // prepend
    empty_packet.insertDataBlock(v2,SIG_EEG,1);    // append   --> v3, v1, v2

    empty_packet.insertDataBlock(v2,SIG_EOG,8);
    empty_packet.insertDataBlock(v3,SIG_HR,2);

    vector<double> v;
    vector<double> test_v;
    v = empty_packet.getSingleDataBlock(SIG_EEG);
    CHECK(v.size() == (v1.size() + v3.size() + v2.size()) );

    test_v.insert(test_v.end(), v3.begin(), v3.end());
    test_v.insert(test_v.end(), v1.begin(), v1.end());
    test_v.insert(test_v.end(), v2.begin(), v2.end());

    for(unsigned int n = 0; n < v.size(); n++)
        CHECK(v[n] == test_v[n]);

    v = empty_packet.getSingleDataBlock(SIG_EOG);
    for(unsigned int n = 0; n < v.size(); n++)
        CHECK(v[n] == v2[n]);

    v = empty_packet.getSingleDataBlock(SIG_HR);
    for(unsigned int n = 0; n < v.size(); n++)
        CHECK(v[n] == v3[n]);

    v = empty_packet.getSingleDataBlock(SIG_EMG);
    for(unsigned int n = 0; n < v.size(); n++)
        CHECK(v[n] == v0[n]);

    // How to check for exceptions?
    //empty_packet.getSingleDataBlock(SIG_BP);

}

//-------------------------------------------------------------------------------------------------
// NOT COMPLETED YET
//TEST_FIXTURE(DataPacketRawMemoryFixture, createVersion2DataPacket)
//{
//    DataPacket packet (reinterpret_cast<void*>(raw_data_packet_version_2));
//}


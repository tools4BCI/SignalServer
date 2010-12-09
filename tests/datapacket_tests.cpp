#include "UnitTest++/UnitTest++.h"

#include "datapacket_tests_fixtures.h"

#include "tia/data_packet3.h"
#include "tia/data_packet.h"

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
///
/// createVersion3DataPacket
///
/// test the creation of a data packet from a binary version 3 representation
///
TEST_FIXTURE(DataPacketRawMemoryFixture, createVersion3DataPacket)
{
    DataPacket3 empty_packet (reinterpret_cast<void*>(version_3_binary_packet_empty));
    CHECK_EQUAL (empty_packet.getPacketID(), RANDOM_PACKET_ID);
    CHECK_EQUAL (empty_packet.getConnectionPacketNr(), RANDOM_CONNECTION_PACKET_NUMBER);

    DataPacket3 eeg_packet (reinterpret_cast<void*>(version_3_binary_packet_eeg));
    //CHECK (empty_packet.getPacketNr() == )

    DataPacket3 eeg_emg_packet (reinterpret_cast<void*>(version_3_binary_packet_eeg_emg));
}

//-------------------------------------------------------------------------------------------------
TEST(assemblePacket)
{
    DataPacket3 assembled_packet;
    vector<double> v0 (25,0);
    vector<double> v1 (5,1);
    vector<double> v2 (16,2);
    vector<double> v3 (12,3);

    assembled_packet.insertDataBlock (v0,SIG_EMG,1);

    assembled_packet.insertDataBlock (v1,SIG_EEG,1);   // insert
    assembled_packet.insertDataBlock (v3,SIG_EEG,1,1);  // prepend
    assembled_packet.insertDataBlock (v2,SIG_EEG,1);    // append   --> v3, v1, v2

    assembled_packet.insertDataBlock (v2,SIG_EOG,8);
    assembled_packet.insertDataBlock (v3,SIG_HR,2);


    vector<double> assembled_eeg_data;
    assembled_eeg_data.insert (assembled_eeg_data.end(), v3.begin(), v3.end());
    assembled_eeg_data.insert (assembled_eeg_data.end(), v1.begin(), v1.end());
    assembled_eeg_data.insert (assembled_eeg_data.end(), v2.begin(), v2.end());

    vector<double> eeg_data_from_packet = assembled_packet.getSingleDataBlock(SIG_EEG);
    CHECK_EQUAL (eeg_data_from_packet.size (), assembled_eeg_data.size ());
    for (unsigned n = 0; n < eeg_data_from_packet.size (); n++)
        CHECK_EQUAL (eeg_data_from_packet[n], assembled_eeg_data[n]);

    vector<double> eog_data_from_packet = assembled_packet.getSingleDataBlock (SIG_EOG);
    CHECK_EQUAL (eog_data_from_packet.size (), v2.size ());
    for (unsigned n = 0; n < eog_data_from_packet.size (); n++)
        CHECK_EQUAL(eog_data_from_packet[n], v2[n]);

    vector<double> hr_data_from_packet = assembled_packet.getSingleDataBlock (SIG_HR);
    CHECK_EQUAL (hr_data_from_packet.size (), v3.size ());
    for (unsigned n = 0; n < hr_data_from_packet.size (); n++)
        CHECK_EQUAL (hr_data_from_packet[n], v3[n]);

    vector<double> emg_data_from_packet = assembled_packet.getSingleDataBlock (SIG_EMG);
    CHECK_EQUAL (emg_data_from_packet.size (), v0.size ());
    for (unsigned n = 0; n < emg_data_from_packet.size (); n++)
        CHECK_EQUAL (emg_data_from_packet[n], v0[n]);

    CHECK_THROW (assembled_packet.getSingleDataBlock (SIG_BP), std::invalid_argument);
    CHECK_THROW (assembled_packet.getSingleDataBlock (SIG_BP | SIG_EEG), std::invalid_argument);
}

//-------------------------------------------------------------------------------------------------
// NOT COMPLETED YET
TEST_FIXTURE(DataPacketRawMemoryFixture, createVersion2DataPacket)
{
    DataPacket packet (reinterpret_cast<void*>(version_2_binary_packet_empty));

    CHECK_EQUAL (packet.getPacketNr(), RANDOM_PACKET_ID);
}

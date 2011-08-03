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

#include "datapacket_tests_fixtures.h"

#include "tia/data_packet3.h"
#include "tia/data_packet.h"
#include "tia-private/clock.h"

#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <boost/thread/xtime.hpp>

#include <string>
#include <vector>

using namespace tobiss;
using std::vector;

//-------------------------------------------------------------------------------------------------
///
/// emptyDataPacket
///
/// checks if an newly created empty data packet behaves like an empty one
///
TEST_FIXTURE(DataPacketSignalTypeFlags, emptyDataPacket)
{
    DataPacket3 empty_packet;
    CHECK (empty_packet.getNrOfChannels().size() == 0);
    CHECK (empty_packet.getNrOfSignalTypes() == 0);
    CHECK (empty_packet.getSamplesPerChannel().size() == 0);
    CHECK (empty_packet.getData().size() == 0);
    CHECK (empty_packet.getConnectionPacketNr() == 0);
    CHECK (empty_packet.getFlags() == 0);

    BOOST_FOREACH (boost::uint32_t flag_setting, all_signal_type_flags_single)
    {
        CHECK_THROW (empty_packet.getSingleDataBlock (flag_setting), std::invalid_argument);
        CHECK (empty_packet.hasFlag (flag_setting) == false);
        CHECK (empty_packet.getNrOfValues(flag_setting) == 0);
        CHECK (empty_packet.getSamplesPerChannel (flag_setting) == 0);
        CHECK (empty_packet.getNrOfBlocks (flag_setting) == 0);
    }
}

//-------------------------------------------------------------------------------------------------
///
/// setterGetterDataPacket
///
/// check the setter and getter methods of DataPacket
TEST(setterGetterDataPacket)
{
    DataPacket3 empty_packet;

    empty_packet.setConnectionPacketNr (5);
    CHECK(empty_packet.getConnectionPacketNr() == 5);

    empty_packet.setPacketID (10);
    CHECK (empty_packet.getPacketID() == 10);

    // reset clock
    Clock::instance ().reset ();

    // wait some time
    boost::xtime xt;
    boost::xtime_get (&xt, boost::TIME_UTC);
    xt.nsec += 10000;
    boost::thread::sleep (xt);

    // set the timestamp of the packet
    empty_packet.setTimestamp ();

    CHECK (empty_packet.getTimestamp() > 0);
    CHECK (empty_packet.getTimestamp () <= Clock::instance ().getMicroSeconds ());
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
    CHECK_EQUAL (empty_packet.getTimestamp(), TIME_STAMP);

    DataPacket3 eeg_packet (reinterpret_cast<void*>(version_3_binary_packet_eeg));
    CHECK_EQUAL (eeg_packet.getPacketID(), RANDOM_PACKET_ID);
    CHECK_EQUAL (eeg_packet.getConnectionPacketNr(), RANDOM_CONNECTION_PACKET_NUMBER);
    CHECK_EQUAL (eeg_packet.getNrOfChannels (SIG_EEG), EEG_CHANNELS);
    CHECK_EQUAL (eeg_packet.getTimestamp(), TIME_STAMP);
    BOOST_FOREACH (boost::uint32_t flag_setting, all_signal_type_flags_single)
    {
        if (flag_setting == SIG_EEG)
            CHECK (eeg_packet.hasFlag (flag_setting));
        else
            CHECK (!eeg_packet.hasFlag (flag_setting));
    }

    DataPacket3 eeg_emg_packet (reinterpret_cast<void*>(version_3_binary_packet_eeg_emg));
    CHECK_EQUAL (eeg_emg_packet.getTimestamp(), TIME_STAMP);
    BOOST_FOREACH (boost::uint32_t flag_setting, all_signal_type_flags_single)
    {
        if (flag_setting == SIG_EEG || flag_setting == SIG_EMG)
            CHECK (eeg_emg_packet.hasFlag (flag_setting));
        else
            CHECK (!eeg_emg_packet.hasFlag (flag_setting));
    }


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

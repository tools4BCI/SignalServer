#ifndef DATAPACKET_TESTS_FIXTURES_H
#define DATAPACKET_TESTS_FIXTURES_H

#include "raw_datapacket_definition.h"

#include "UnitTest++/UnitTest++.h"

#include <boost/cstdint.hpp>

#include <vector>

//-------------------------------------------------------------------------------------------------
struct DataPacketSetterGetterFixture
{
    DataPacketSetterGetterFixture ()
        : packet_number (5)
    {}
    unsigned packet_number;
};

//-------------------------------------------------------------------------------------------------
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
struct DataPacketRawMemoryFixture
{
    DataPacketRawMemoryFixture ()
        : eeg_channels (10),
          eeg_blocksize (12),
          packet_number (5),
          connection_packet_number (10)
    {
        for (size_t eeg_channel = 0; eeg_channel < eeg_channels; ++eeg_channel)
        {
            for (size_t eeg_sample = 0; eeg_sample < eeg_blocksize; ++eeg_sample)
                data[0][eeg_channel][eeg_sample] = eeg_channel * eeg_blocksize + eeg_sample;
        }
        generateRawDataPacketVersion2 (raw_data_packet_version_2, EEG_SIG_FLAG, packet_number, connection_packet_number, data);
        generateRawDataPacketVersion3 (raw_data_packet_version_3, EEG_SIG_FLAG, packet_number, connection_packet_number, data);
        generateRawDataPacketVersion3 (empty_raw_data_packet_version_3, NO_SIG_FLAG, packet_number, connection_packet_number, std::vector<std::vector<std::vector<float> > >());
    }

    size_t eeg_channels;
    size_t eeg_blocksize;
    std::vector<std::vector<std::vector<float> > > data;

    boost::uint64_t packet_number;
    boost::uint64_t connection_packet_number;

    unsigned char raw_data_packet_version_2[500];
    unsigned char raw_data_packet_version_3[500];
    unsigned char empty_raw_data_packet_version_3[500];
};



#endif // DATAPACKET_TESTS_FIXTURES_H

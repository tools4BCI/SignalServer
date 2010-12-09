#ifndef DATAPACKET_TESTS_FIXTURES_H
#define DATAPACKET_TESTS_FIXTURES_H

#include "raw_datapacket_definition.h"

#include "UnitTest++/UnitTest++.h"

#include "tia/defines.h"

#include <boost/cstdint.hpp>

#include <vector>

//-------------------------------------------------------------------------------------------------
class DataPacketSignalTypeFlags
{
public:
    DataPacketSignalTypeFlags ()
        : no_flag_set (0),
          all_flags_set (0x3FFFFF),
          smallest_flag (SIG_EEG),
          highest_flag (SIG_EVENT),
          all_signal_type_flags_combinations (all_flags_set, 0)
    {
        for (boost::uint32_t flag = smallest_flag; flag <= highest_flag; flag <<= 1)
            all_signal_type_flags_single.push_back (flag);
        for (boost::uint32_t flags = no_flag_set; flags < all_flags_set; ++flags)
            all_signal_type_flags_combinations[flags] = flags;
    }

    boost::uint32_t const no_flag_set;
    boost::uint32_t const all_flags_set;
    boost::uint32_t const smallest_flag;
    boost::uint32_t const highest_flag;
    std::vector<boost::uint32_t> all_signal_type_flags_single;
    std::vector<boost::uint32_t> all_signal_type_flags_combinations;
};

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
        : EEG_CHANNELS (10),
          EEG_SAMPLES_PER_CHANNEL (12),
          EMG_CHANNELS (2),
          EMG_SAMPLES_PER_CHANNEL (3),
          RANDOM_PACKET_ID (rand()),
          RANDOM_CONNECTION_PACKET_NUMBER (rand())
    {
        initSignalData (eeg_data, EEG_CHANNELS, EEG_SAMPLES_PER_CHANNEL);
        initSignalData (emg_data, EMG_CHANNELS, EMG_SAMPLES_PER_CHANNEL);

        std::vector<std::vector<std::vector<float> > > empty_data;
        std::vector<std::vector<std::vector<float> > > data_only_eeg;
        std::vector<std::vector<std::vector<float> > > data_eeg_and_emg;

        data_only_eeg.push_back (eeg_data);

        data_eeg_and_emg.push_back (eeg_data);
        data_eeg_and_emg.push_back (emg_data);

        generateRawDataPacketVersion2 (version_2_binary_packet_empty, NO_SIG_FLAG, RANDOM_PACKET_ID, RANDOM_CONNECTION_PACKET_NUMBER, empty_data);

        generateRawDataPacketVersion3 (version_3_binary_packet_empty, NO_SIG_FLAG, RANDOM_PACKET_ID, RANDOM_CONNECTION_PACKET_NUMBER, empty_data);
        generateRawDataPacketVersion3 (version_3_binary_packet_eeg, EEG_SIG_FLAG, RANDOM_PACKET_ID, RANDOM_CONNECTION_PACKET_NUMBER, data_only_eeg);
        generateRawDataPacketVersion3 (version_3_binary_packet_eeg_emg, EEG_SIG_FLAG | EMG_SIG_FLAG, RANDOM_PACKET_ID, RANDOM_CONNECTION_PACKET_NUMBER, data_eeg_and_emg);
    }

    size_t const EEG_CHANNELS;
    size_t const EEG_SAMPLES_PER_CHANNEL;
    std::vector<std::vector<float> > eeg_data;

    size_t const EMG_CHANNELS;
    size_t const EMG_SAMPLES_PER_CHANNEL;
    std::vector<std::vector<float> > emg_data;

    boost::uint64_t RANDOM_PACKET_ID;
    boost::uint64_t RANDOM_CONNECTION_PACKET_NUMBER;

    unsigned char version_2_binary_packet_empty[50];

    unsigned char version_3_binary_packet_empty[50];
    unsigned char version_3_binary_packet_eeg[1000];
    unsigned char version_3_binary_packet_eeg_emg[1500];

private:
    void initSignalData (std::vector<std::vector<float> >& signal_data, size_t channels, size_t samples_per_channel)
    {
        signal_data.clear ();
        signal_data.insert (signal_data.begin (), channels, std::vector<float> (samples_per_channel));
        for (size_t channel = 0; channel < channels; ++channel)
        {
            for (size_t sample = 0; sample < samples_per_channel; ++sample)
                signal_data[channel][sample] = (channel * samples_per_channel) + sample;
        }

    }
};



#endif // DATAPACKET_TESTS_FIXTURES_H

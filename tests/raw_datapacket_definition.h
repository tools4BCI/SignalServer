#ifndef RAW_DATAPACKET_DEFINITION_H
#define RAW_DATAPACKET_DEFINITION_H

#include <boost/cstdint.hpp>

#include <vector>
#include <cstring>
#include <stdexcept>

//-------------------------------------------------------------------------------------------------
// signal type flags
boost::uint32_t const NO_SIG_FLAG = 0x0;
boost::uint32_t const EEG_SIG_FLAG = 0x1;
boost::uint32_t const EMG_SIG_FLAG = 0x2;

//-------------------------------------------------------------------------------------------------
namespace datapacket_version_3
{
    size_t const FIXED_HEADER_SIZE = 33;
    size_t const VARIABLE_HEADER_POS = FIXED_HEADER_SIZE;
}


//-------------------------------------------------------------------------------------------------
//
// this function generates a binary representation of a data packet version 3
//
void generateRawDataPacketVersion3 (unsigned char* target_memory,
                                    boost::uint32_t signal_type_flags,
                                    boost::uint64_t packet_id,
                                    boost::uint64_t connection_packet_number,
                                    boost::uint64_t time_stamp,
                                    std::vector<std::vector<std::vector<float> > > const& samples)
{
    size_t variable_header_size = 4 * samples.size ();
    size_t data_position = datapacket_version_3::VARIABLE_HEADER_POS + variable_header_size;
    size_t data_size = 0;

    boost::uint16_t number_channels[samples.size()];
    boost::uint16_t samples_per_channel[samples.size()];
    size_t samples_per_channel_pos = datapacket_version_3::VARIABLE_HEADER_POS + (2 * samples.size());

    for (size_t signal_index = 0; signal_index < samples.size (); ++signal_index)
    {
        number_channels[signal_index] = samples[signal_index].size ();
        samples_per_channel[signal_index] = samples[signal_index][0].size ();

        data_size += number_channels[signal_index] * samples_per_channel[signal_index];
    }

    boost::uint32_t packet_size = datapacket_version_3::FIXED_HEADER_SIZE +
                                  variable_header_size + data_size;

    //-------------------------------------------
    // begin fixed header

    // packet version: byte [0]
    target_memory[0] = 3;

    // packet size: byte [1]-[4], little endian
    memcpy (target_memory + 1, &packet_size, 4);

    // signal type flags: [5]-[8], little endian
    target_memory[5] = signal_type_flags & 0xFF;
    target_memory[6] = (signal_type_flags >> 8) & 0xFF;
    target_memory[7] = (signal_type_flags >> 16) & 0xFF;
    target_memory[8] = (signal_type_flags >> 24) & 0xFF;

    // packet id: byte [9]-[16], little endian
    target_memory[9] = packet_id & 0xFF;
    target_memory[10] = (packet_id >> 8) & 0xFF;
    target_memory[11] = (packet_id >> 16) & 0xFF;
    target_memory[12] = (packet_id >> 24) & 0xFF;
    target_memory[13] = (packet_id >> 32) & 0xFF;
    target_memory[14] = (packet_id >> 40) & 0xFF;
    target_memory[15] = (packet_id >> 48) & 0xFF;
    target_memory[16] = (packet_id >> 56) & 0xFF;

    // connection packet number: byte [17]-[24], little endian
    target_memory[17] = connection_packet_number & 0xFF;
    target_memory[18] = (connection_packet_number >> 8) & 0xFF;
    target_memory[19] = (connection_packet_number >> 16) & 0xFF;
    target_memory[20] = (connection_packet_number >> 24) & 0xFF;
    target_memory[21] = (connection_packet_number >> 32) & 0xFF;
    target_memory[22] = (connection_packet_number >> 40) & 0xFF;
    target_memory[23] = (connection_packet_number >> 48) & 0xFF;
    target_memory[24] = (connection_packet_number >> 56) & 0xFF;

    // posix timestamp: byte [25]-[32], little endian
    memcpy (target_memory + 25, &time_stamp, 8);

    // end of fixed header
    //-------------------------------------------

    //-------------------------------------------
    // begin variable header

    // number channels
    for (size_t signal_index = 0; signal_index < samples.size(); ++signal_index)
    {
        target_memory[datapacket_version_3::VARIABLE_HEADER_POS + (signal_index * 2)] = number_channels[signal_index] & 0xFF;
        target_memory[datapacket_version_3::VARIABLE_HEADER_POS + (signal_index * 2) + 1] = (number_channels[signal_index] >> 8) & 0xFF;
    }

    // samples per channel and signal
    for (size_t signal_index = 0; signal_index < samples.size(); ++signal_index)
    {
        target_memory[samples_per_channel_pos + (signal_index * 2)] = samples_per_channel[signal_index] & 0xFF;
        target_memory[samples_per_channel_pos + (signal_index * 2) + 1] = (samples_per_channel[signal_index] >> 8) & 0xFF;
    }

    // end of variable header
    //-------------------------------------------

    //-------------------------------------------
    // begin data

    size_t data_insert_pos = data_position;
    for (size_t signal_index = 0; signal_index < samples.size (); ++signal_index)
    {
        for (size_t channel_index = 0; channel_index < samples[signal_index].size (); ++channel_index)
        {
            for (size_t sample_index = 0; sample_index < samples[signal_index][channel_index].size (); ++sample_index)
            {
                memcpy (target_memory + data_insert_pos, &(samples[signal_index][channel_index][sample_index]), sizeof(float));
                data_insert_pos += sizeof(float);
            }
        }
    }

    // end of data
    //-------------------------------------------
}


//-------------------------------------------------------------------------------------------------
namespace datapacket_version_2
{
    size_t const FIXED_HEADER_SIZE = 33;
    size_t const VARIABLE_HEADER_POS = FIXED_HEADER_SIZE;
    boost::uint32_t PACKET_VERSION_FLAG = 0x08000000;
    boost::uint32_t PACKET_CONTROL_FLAG = 0x02400000;
}

//-------------------------------------------------------------------------------------------------
//
// this function generates a binary representation of a data packet version 2
//
void generateRawDataPacketVersion2 (unsigned char* target_memory,
                                    boost::uint32_t signal_type_flags,
                                    boost::uint64_t sample_number,
                                    boost::uint64_t packet_number,
                                    std::vector<std::vector<std::vector<float> > > const& samples)
{
    if (signal_type_flags & (datapacket_version_2::PACKET_VERSION_FLAG | datapacket_version_2::PACKET_CONTROL_FLAG))
        throw std::runtime_error ("generateRawDataPacketVersion2: invalid signal type flags");

    signal_type_flags |= datapacket_version_2::PACKET_VERSION_FLAG;
    signal_type_flags |= datapacket_version_2::PACKET_CONTROL_FLAG;

    // packet version
    memcpy(target_memory, &signal_type_flags, 4);

    memcpy(target_memory + 4, &sample_number, 8);
    memcpy(target_memory + 12, &sample_number, 8);

}

#endif // RAW_DATAPACKET_DEFINITION_H

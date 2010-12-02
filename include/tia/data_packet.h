/*
    This file is part of TOBI Interface A (TiA).

    TOBI Interface A (TiA) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TOBI Interface A (TiA) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TOBI Interface A (TiA).  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

/**
* @file data_packet.h
*
* @brief The DataPacket stores all sampled data from every connected device.
*
* The DataPacket contains the samples from all connected devices in ascending order
* (by their signal type).
* It stores acquired samples in a vector and is able to build a raw representation
* including needed header information of its stored samples.
*
**/

#ifndef DATAPACKET_H
#define DATAPACKET_H

#include <map>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/cstdint.hpp>

#include "tia/defines.h"

namespace tobiss
{

class RawMem;

/**
* @class DataPacket
*
* @brief A Class to manage the DataPacket to be distributed over network.
*
* The DataPacket class provides methods inserting Samples of different, predefined signal types
* (see defines.h) into a packet. Inserted data is identified through flags.
* The datapacket stores the acquired samples in ascending order by their signal types.
* (EEG first, then EMG, EOG, ...).
* It provides methods to get stored data wether again as a vector or in a raw representation.
* For raw representations, it holds raw memory objects, storing the respective representation.
*
* @todo Implementing the possibility to built a raw DataPacket representation with freely selectable content.
*/
class DataPacket
{
  public:
    /**
    * @brief Default constructor -- building an empty DataPacket.
    */
    DataPacket()
    : flags_(0), packet_nr_(0),
      timestamp_(boost::posix_time::microsec_clock::local_time()), nr_of_signal_types_(0)
    {  }

    /**
    * @brief Constructor to (re)build a DataPacket from raw memory.
    *
    * @param[in] mem Raw memory containing the raw representation of a DataPacket.
    * @throw std::invalid_argument if packet version missmatch
    * @todo Implementing safety checks, as far as possible.
    *
    * This method (re)builds a DataPacket object from a raw memory region, containing
    * the raw representation of a DataPacket.
    */
    DataPacket(void* mem);

    /**
    * @brief Destructor
    *
    * @todo Check for memory leaks! (can occur, if raw_mem objects are built)
    *
    * Deletes all dynamically built raw_mem objects stored in raw_map.
    */
    virtual ~DataPacket();
	
	/**
    * @brief Copy constructor  --  does NOT copy the raw memory representation.
    * @todo Check for memory leaks! (can occur, if raw_mem objects are built)
    */
    DataPacket(const DataPacket &src)
    {
      sample_nr_ = src.sample_nr_;
      flags_ = src.flags_;
      packet_nr_ = src.packet_nr_;
      timestamp_ = src.timestamp_;
      nr_of_signal_types_ = src.nr_of_signal_types_;

      nr_blocks_ = src.nr_blocks_;
      nr_values_ = src.nr_values_;
      data_ = src.data_;

      std::map<boost::uint32_t, RawMem*> raw_map_;
    }
    /**
    * @brief Resets the whole DataPacket (samples are deleted).
    */
    void reset();
    /**
    * @brief Increase the static sample number by one.
    */
    void incSampleNr();

    /**
    * @brief Insert data (only one signal type) from a hardware device into the DataPacket (will be automatically placed correct).
    * @param[in]  v   Vector containing homogenous samples (only one signal type).
    * @param[in]  signal_flag   The flag specifiying the inserted signal.
    * @param[in]  blocks    The number of blocks, the vector contains.
    * @param[in]  prepend    Prepend the data, otherwise append (default), if Signaltype already inserted.
    * @throws std::logic_error if flags in the DataPacket on not OK, if flag already defined or if more than on flag passed
    *
    * This mehtod inserts a homogenous sample vector (consiting of only ONE single signal type)
    * into the DataPacket. The vector is automatically correctly placed in the Packet,
    * respective to its signal type.
    * @warning Up to the DataPacket can only store one signal type from only one device. If two
    * devices are delivering the same signal type, an exception is thrown. (Could be avoided,
    * if the second device delivers a user-type signal.)
    * @todo If needed: Implementing the possibility to store the same signal type comming from different devices.
    */
    void insertDataBlock(std::vector<double> v, boost::uint32_t signal_flag, boost::uint16_t blocks, bool prepend = false);

    /**
    * @brief Set the packet number (can differ from the samplenumber -- e.g. if sending with different rates)
    */
    void setPacketNr(boost::uint64_t);

    /**
    * @brief Set the packet number (can differ from the samplenumber -- e.g. if sending with different rates)
    */
    boost::uint64_t getPacketNr();

    /**
    * @brief Set the timestamp to localtime.
    */
    void setTimestamp();

    /**
    * @brief Get the timestamp (localtime) of the datapacket.
    */
    boost::posix_time::ptime getTimestamp()   { return(timestamp_);  }

    /**
    * @brief Check, if a flag is already set.
    * @return bool
    */
    bool hasFlag(boost::uint32_t f);
    /**
    * @brief Get the number of signal types stored in the DataPacket.
    * @return The amount of signal types stored in the DataPacket.
    * @throw std::logic_error if flags are not OK.
    */
    boost::uint16_t getNrOfSignalTypes();
    /**
    * @brief Get the flags (including packetversion) of the DataPacket.
    * @return flags
    * @throw std::logic_error if flags are not OK.
    */
    boost::uint32_t getFlags();
    /**
    * @brief Get the sample number of the DataPacket.
    * @return sample number
    */
    boost::uint64_t getSampleNr();
    /**
    * @brief Get a vector containing the blocksizes for every signal type.
    * @return A vector containing the blocksizes for every signal type.
    */
    std::vector<boost::uint16_t> getNrOfBlocks();
    /**
    * @brief Get a vector containing the number of values (NOT channels!) for every signal type.
    * @return A vector containing the number of values (NOT channels!)for every signal type.
    */
    std::vector<boost::uint16_t> getNrOfValues();
    /**
    * @brief Get all samples stored in the DataPacket as a vector.
    * @return A vector containing the samples stored in the DataPacket.
    * @throw std::logic_error if flags are not OK.
    */
    std::vector<double> getData();
    /**
    * @brief Get a vector containing all samples for a specific signal type.
    * @param[in] flag
    * @return A vector containing all samples for a specific signal type.
    * @throw std::logic_error if flags are not OK.
    * @throw std::invalid_argument if flag is not set in the DataPacket
    */
    std::vector<double> getSingleDataBlock(boost::uint32_t flag);

    /**
    * @brief Get the number of values for a specific signal type.
    * @param[in] flag
    * @return boost::uint16_t The number of values.
    */
    boost::uint16_t getNrOfValues(boost::uint32_t flag);

    /**
    * @brief Get the number of blocks for a specific signal type.
    * @param[in] flag
    * @return boost::uint16_t The number of blocks.
    */
    boost::uint16_t getNrOfBlocks(boost::uint32_t flag);

    /**
    * @brief Get a pointer to a memory region, containing the raw representation of the DataPacket.
    * @return A pointer pointing to the beginning of the memory region.
    *
    * Calling this function the first time will also build the raw representation of the DataPacket!
    * @todo Inserting data after building a raw representation, won't change the raw memory and
    * will deliver a pointer pointing to "the old" DataPacket.
    */
    void* getRaw();

    /**}
     *
    * @brief Get the size (in bytes) of the raw memory region holding the raw DataPacket.
    * @return The size of the raw memory region.
    */
    boost::uint32_t getRawMemorySize();

    /**
    * @brief Get the required size (in bytes) to hold a given raw DataPacket.
    * @return The needed size of the raw memory region.
    */
    boost::uint32_t getRequiredRawMemorySize();

    /**
    * @brief Get the required size (in bytes) to hold raw DataPacket stored in RAW memory.
    * This function can be used to check, wether a memory region is able to hold a whole
    * DataPacket by analyzing its header. If the RAW memory Region is too small to hold the
    * DataPacket, 0 is returned.
    * @param[in] mem  A pointer to the RAW memory region holding a DataPacket.
    * @param[in] bytes_available  The memory size, the DataPacket is stored in.
    * @return 0 if the RAW memory region is too small to hold the DataPacket,
    * otherwise the needed size of the raw memory region.
    */
    boost::uint32_t getRequiredRawMemorySize(void* mem, boost::int32_t bytes_available);

  private:
    /**
    * @brief Calculate the number of different signaltypes (equal to bits == 1) defined in a flag.
    * @return The number of bits == 1.
    */
    boost::uint16_t calcNrOfSignalTypes(boost::uint32_t f);
    /**
    * @brief Check, if the flags are OK and not in conflict to other values in the DataPacket.
    * @return bool
    */
    bool flagsOK();
    /**
    * @brief Set a flag in flags.
    * @param[in] flag The Flag to set.
    */
    void setFlag(boost::uint32_t flag);
    /**
    * @brief Get the data position (first signal, second, ...) of a signal type in the DataPacket.
    * @param[in] flag The signal type to get the position for.
    * @return The position of the signal type.
    */
    boost::uint32_t getDataPos(boost::uint32_t flag);
    /**
    * @brief Get the offset in the samples vector for a signal at a desired position (first signal, ...).
    * @param[in] pos The position in the DataPacket (from getDataPos() ).
    * @return The offset in the samples vector for the "n-th" signal.
    */
    boost::uint32_t getOffset(boost::uint32_t pos);

    /**
    * @brief Append data (only one signal type) from a hardware device into the DataPacket (will be automatically placed correct).
    * @param[in]  &v   Vector containing homogenous samples (only one signal type).
    * @param[in]  signal_flag   The flag specifiying the appended signal.
    * @param[in]  blocks    The number of blocks (=blocksize), the vector contains.
    * @throws TODO
    *
    * TODO: Description
    */
    void appendDataBlock(std::vector<double> &v, boost::uint32_t signal_flag, boost::uint16_t blocks);

    /**
    * @brief Append data (only one signal type) from a hardware device into the DataPacket (will be automatically placed correct).
    * @param[in]  &v   Vector containing homogenous samples (only one signal type).
    * @param[in]  signal_flag   The flag specifiying the appended signal.
    * @param[in]  blocks    The number of blocks (=blocksize), the vector contains.
    * @throws TODO
    *
    * TODO: Description
    */
    void prependDataBlock(std::vector<double> &v, boost::uint32_t signal_flag, boost::uint16_t blocks);

  private:
    /**
    * @brief The sample number the data packet is holding.
    * @todo Is a static declaration needed/useful for the sample number?
    */
    static boost::uint64_t  sample_nr_;

    /**
    * @brief Flags, specifying the signal types stored in the DataPacket.
    * @todo Replace by a std::bitset!
    */
    boost::uint32_t  flags_;
    boost::uint64_t  packet_nr_;        ///< The packet number of the DataPacket (not mandatory = the sample number)
    boost::posix_time::ptime  timestamp_;   ///< Timestamp of the datapacket (important ... synchronize clocks!)

    boost::uint16_t  nr_of_signal_types_;   ///<  Number of different signal types stored in the DataPacket.

    std::vector<boost::uint16_t>  nr_blocks_;  ///< Blocksize for every signal type.
    std::vector<boost::uint16_t>  nr_values_;  ///< Number of values for every signal type.
    std::vector<double> data_;  ///< The samples stored in the DataPacket.

    std::map<boost::uint32_t, RawMem*> raw_map_; ///< A map holding pointers to raw_mem objects, identified by their flags.
};

} // Namespace tobiss

#endif // DATAPACKET_H

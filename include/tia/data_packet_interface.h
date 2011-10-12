/*
    This file is part of the TOBI Interface A (TiA) library.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file lgpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/lgpl.html.

    In case of GNU Lesser General Public License Usage ,the TiA library
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TiA library. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: TiA@tobi-project.org
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


#include <boost/cstdint.hpp>

#include "tia/defines.h"

namespace std
{
  class vector;
}

namespace tia
{

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
*/
class DataPacket
{
  /** \example datapacket-usage-example.cpp
  *   This is an example how to build an fill a TiA data packet with
  *   raw samples and set needed parameters.
  */

  public:
    /**
    * @brief Default constructor -- building an empty DataPacket.
    */
    virtual DataPacket() = 0;

    /**
    * @brief Constructor to (re)build a DataPacket from raw memory.
    *
    * @param[in] mem Raw memory containing the raw representation of a DataPacket.
    * @throw std::invalid_argument if packet version missmatch
    * @todo Implementing safety checks, as far as possible.
    * @todo Size field is skipped!
    *
    * This method (re)builds a DataPacket object from a raw memory region, containing
    * the raw representation of a DataPacket.
    */
    virtual DataPacket(void* mem) = 0;

    /**
    * @brief Destructor
    */
    virtual ~DataPacket() = 0;

    /**
    * @brief Copy constructor  --  does NOT copy the raw memory representation.
    * @todo Check for memory leaks! (can occur, if raw_mem objects are built)
    */
    virtual DataPacket(const DataPacket &src) = 0;

    /**
    * @brief Resets the whole DataPacket (samples are deleted).
    */
    virtual void reset() = 0;

    /**
    * @brief Increase the static sample number by one.
    */
    virtual void incPacketID() = 0;

    /**
    * @brief Set the packet ID.
    */
    virtual void setPacketID(boost::uint64_t nr) = 0;

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
    */
    virtual void insertDataBlock(std::vector<double> v, boost::uint32_t signal_flag, boost::uint16_t blocks, bool prepend = false)  = 0;

    /**
    * @brief Set the packet number (can differ from the samplenumber -- e.g. if sending with different rates)
    */
    virtual void setConnectionPacketNr(boost::uint64_t) = 0;

    /**
    * @brief Set the packet number (can differ from the samplenumber -- e.g. if sending with different rates)
    */
    virtual boost::uint64_t getConnectionPacketNr() = 0;

    /**
    * @brief Set the timestamp to localtime.
    */
    virtual void setTimestamp() = 0;

    /**
    * @brief Get the timestamp (microseconds since signal server has been started)
    *        of the datapacket.
    */
    virtual boost::uint64_t getTimestamp() = 0;

    /**
    * @brief Check, if a flag is already set.
    * @return bool
    */
    virtual bool hasFlag(boost::uint32_t f) = 0;
    /**
    * @brief Get the number of signal types stored in the DataPacket.
    * @return The amount of signal types stored in the DataPacket.
    * @throw std::logic_error if flags are not OK.
    */
    virtual boost::uint16_t getNrOfSignalTypes() = 0;
    /**
    * @brief Get the flags (including packetversion) of the DataPacket.
    * @return flags
    * @throw std::logic_error if flags are not OK.
    */
    virtual boost::uint32_t getFlags() = 0;
    /**
    * @brief Get the sample number of the DataPacket.
    * @return sample number
    */
    virtual boost::uint64_t getPacketID() = 0;
    /**
    * @brief Get a vector containing the blocksizes for every signal type.
    * @return A vector containing the blocksizes for every signal type.
    */
    virtual std::vector<boost::uint16_t> getSamplesPerChannel() = 0;
    /**
    * @brief Get a vector containing the number of values (NOT channels!) for every signal type.
    * @return A vector containing the number of values (NOT channels!)for every signal type.
    */
    virtual std::vector<boost::uint16_t> getNrOfChannels() = 0;
    /**
    * @brief Get all samples stored in the DataPacket as a vector.
    * @return A vector reference to the samples stored in the DataPacket.
    * @throw std::logic_error if flags are not OK.
    */
    virtual const std::vector<double>& getData() = 0;
    /**
    * @brief Get a vector containing all samples for a specific signal type.
    * @param[in] flag
    * @return A vector containing all samples for a specific signal type.
    * @throw std::logic_error if flags are not OK.
    * @throw std::invalid_argument if flag is not set in the DataPacket
    */
    virtual std::vector<double> getSingleDataBlock(boost::uint32_t flag) = 0;


    /**
    * @brief TODO
    */
    virtual boost::uint16_t getNrOfValues(boost::uint32_t flag) = 0;

    /**
    * @brief Get the number of values for a specific signal type.
    * @param[in] flag
    * @return boost::uint16_t The number of values.
    */
    virtual boost::uint16_t getNrOfChannels(boost::uint32_t flag) = 0;

    /**
    * @brief Get the number of blocks for a specific signal type.
    * @param[in] flag
    * @return boost::uint16_t The number of blocks.
    */
    virtual boost::uint16_t getSamplesPerChannel(boost::uint32_t flag) = 0;

    /**
    * @brief Get the number of blocks for a specific signal type (the same as getSamplesPerChannel).
    * @param[in] flag
    * @return boost::uint16_t The number of blocks (samples per channel).
    */
    virtual boost::uint16_t getNrOfBlocks(boost::uint32_t flag) = 0;

    /**
    * @brief Get a pointer to a memory region, containing the raw representation of the DataPacket.
    * @return A pointer pointing to the beginning of the memory region.
    *
    * Calling this function the first time will also build the raw representation of the DataPacket!
    * @todo Inserting data after building a raw representation, won't change the raw memory and
    * will deliver a pointer pointing to "the old" DataPacket.
    */
    virtual void* getRaw() = 0;

    /**
     *
    * @brief Get the size (in bytes) of the raw memory region holding the raw DataPacket.
    * @return The size of the raw memory region.
    */
    virtual boost::uint32_t getRawMemorySize() = 0;

    /**
    * @brief Get the required size (in bytes) to hold a given raw DataPacket.
    * @return The needed size of the raw memory region.
    */
    virtual boost::uint32_t getRequiredRawMemorySize() = 0;

    /**
    * @brief Get the required size (in bytes) to hold raw DataPacket stored in RAW memory.
    * This function can be used to check, wether a memory region is able to hold a whole
    * DataPacket by analyzing its header. If the RAW memory Region is too small to hold the
    * DataPacket, 0 is returned.
    * @param[in] mem  A pointer to the RAW memory region holding a DataPacket.
    * @param[in] bytes_available  The memory size, the DataPacket is stored in.
    * @return 0 if the RAW memory region is too small to hold the DataPacket,
    * otherwise the needed size of the raw memory region.
    *
    * @todo Rewrite this mehtod as its size is now available inside the data packet.
    */
    virtual boost::uint32_t getRequiredRawMemorySize(void* mem, boost::int32_t bytes_available) = 0;

};

} // Namespace tia

#endif // DATAPACKET_H

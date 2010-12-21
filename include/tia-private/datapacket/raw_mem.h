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
* @file raw_mem.h
*
* @brief raw_mem holds memory regions containing a raw representation of a datapacket
*
* @todo Up to now only representation of float is possible. Should be enhanced to be a template also
* being able to deal with double.
*
* raw_mem can hold a memory region, containing either a complete datapacket.
* It automatically allocates and frees the needed memory.
**/

#ifndef RAWMEM_H
#define RAWMEM_H

// STL

#include <vector>

// Boost
#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// Local
#include "tia/defines.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class RawMem
*
* @brief Class holding a memory regions storing the raw representation of a DataPacket.
*
* RawMem can holds a memory region, containing a complete DataPacket. It does not care,
* whether there are more signal types in it or not.
* The needed memory size is automaticaly determined.
*/
class RawMem
{

  public:
    /**
    * @brief Constructor
    *
    * @param[in] flags Flags representing the content of the DataPacket.
    * @param[in] sample_nr  The running sample number.
    * @param[in] packet_nr  The running packet number (can be different to sample_nr).
    * @param[in] timestamp  The timestamp of the packet.
    * @param[in] nr_values  Number of values stored for every signal type in flags.
    * @param[in] nr_blocks  Blocksize for every signal type in flags.
    * @param[in] data   The samples to be stored.
    */
    RawMem(boost::uint32_t flags,
           boost::uint64_t sample_nr,
           boost::uint64_t packet_nr, \
           boost::posix_time::ptime timestamp, \
           std::vector<boost::uint16_t>& nr_values, std::vector<boost::uint16_t>& nr_blocks,
           std::vector<double>& data);

    /**
    * @brief Destructor
    *
    * Frees the memory region holding the DataPacket.
    */
    virtual ~RawMem()    {  free(mem_);  }

    /**
    * @brief Returns the size (in bytes) of the memory region.
    * @return Size of the memory region (in bytes).
    */
    boost::uint32_t size()     { return(size_); }
    /**
    * @brief Returns a pointer to the memory region.
    * @return Void* pointing to the memory region.
    * @warning Don't free the memory region by your own!
    */
    void* getMemPtr()   { return(mem_);  }


  private:
    void* mem_;   ///<  Pointer to the memory region.
    boost::uint32_t size_;   ///<  Size of the memory region in bytes.
};

} // Namespace tobiss

//-----------------------------------------------------------------------------

#endif // RAWMEM_H

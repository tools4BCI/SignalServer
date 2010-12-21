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
* @file data_packet_selector.h
**/

#ifndef DATAPACKETSELECTOR_H
#define DATAPACKETSELECTOR_H

#include <set>
#include <vector>

#include <boost/cstdint.hpp>

#include "tia/data_packet.h"

namespace tobiss
{

//-----------------------------------------------------------------------------

/**
* @class DataPacketSelector
* @brief Class to create individual data packet with selected channels and signal types.
*/
class DataPacketSelector
{
  public:
    /**
    * @brief Constructor
    */
    DataPacketSelector();

    void setDesiredChannels(  boost::uint32_t signal_type, boost::uint16_t blocksize,
                         std::set<boost::uint16_t> channels);

    DataPacket getModifiedDataPacket(DataPacket& packet);

  private:

  //-----------------------------------

  private:
    std::map<boost::uint32_t, std::set<boost::uint16_t> >  ch_selection_;

    std::map<boost::uint32_t, std::vector<double> >  sample_map_;

    std::vector< double>  work_vec_;

};

}  //tobiss

#endif // DATAPACKETSELECTOR_H

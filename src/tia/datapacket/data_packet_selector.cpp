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
* @file data_packet_selector.cpp
**/

#include "tia-private/datapacket/data_packet_selector.h"

namespace tobiss
{

using std::vector;
using std::map;
using std::set;

using boost::uint32_t;
using boost::uint16_t;

static const unsigned int RESERVED_NR_SAMPLES_4_WORK_VEC = 2048;

DataPacketSelector::DataPacketSelector()
{
  work_vec_.reserve(RESERVED_NR_SAMPLES_4_WORK_VEC);
}

//-----------------------------------------------------------------------------

void DataPacketSelector::setDesiredChannels( boost::uint32_t signal_type, boost::uint16_t blocksize,
                                         std::set<boost::uint16_t> channels)
{
  ch_selection_.insert( make_pair( signal_type, channels ) );

  vector<double> vec;
  vec.reserve( blocksize * channels.size() );
  sample_map_.insert( make_pair( signal_type, vec ) );
}

//-----------------------------------------------------------------------------

DataPacket DataPacketSelector::getModifiedDataPacket(DataPacket& packet)
{
  DataPacket new_packet;

  map<uint32_t, set<uint16_t> >::iterator it;
  uint32_t flag = 0;
  uint16_t nr_ch = 0;
  uint16_t nr_values = 0;
  uint16_t blocksize = 0;
  uint32_t offset = 0;


  for( it = ch_selection_.begin(); it != ch_selection_.end(); it++  )
  {
    flag = it->first;

    work_vec_ = packet.getSingleDataBlock(flag);
    blocksize = packet.getNrOfBlocks(flag);
    nr_values = packet.getNrOfValues(flag);
    nr_ch  = nr_values/blocksize;

    vector<double>&  sample_vec = sample_map_[flag];
    set< uint16_t >& channel_list = it->second;

    for(unsigned int n = 0; n < nr_ch; n++)
    {
      if( channel_list.find(n) != channel_list.end() )
      {
        offset = n * blocksize;
        for(unsigned int k = 0; k < blocksize; k++)
          sample_vec.push_back(offset + k);
      }
    }

    new_packet.insertDataBlock( sample_vec, flag, blocksize);
    sample_vec.clear();
  }

  return(new_packet);

}

//-----------------------------------------------------------------------------

}   //tobiss

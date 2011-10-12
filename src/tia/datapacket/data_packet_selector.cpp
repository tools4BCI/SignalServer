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
* @file data_packet_selector.cpp
**/

#include "tia-private/datapacket/data_packet_selector.h"

namespace tia
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

DataPacketImpl DataPacketSelector::getModifiedDataPacket(DataPacketImpl& packet)
{
  DataPacketImpl new_packet;

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
    nr_values = packet.getNrOfSamples(flag);
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

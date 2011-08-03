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
* @file data_packet3.cpp
**/

#include <sstream>
#include <stdexcept>

#include "tia/data_packet3.h"
#include "tia-private/clock.h"
#include "tia-private/datapacket/raw_mem3.h"

namespace tobiss
{

using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;
using boost::int32_t;

using std::vector;
using std::map;
using std::string;
using std::stringstream;
using std::make_pair;

static const unsigned int CURRENT_DATA_PACKET_VERSION = 3;

//-----------------------------------------------------------------------------

DataPacket3::DataPacket3()
: version_(CURRENT_DATA_PACKET_VERSION), packet_id_(0),
  flags_(0), connection_packet_nr_(0),
  timestamp_(0), nr_of_signal_types_(0)
{

}

//-----------------------------------------------------------------------------

DataPacket3::DataPacket3(const DataPacket3 &src)
{
  version_ = src.version_;
  packet_id_ = src.packet_id_;
  flags_ = src.flags_;
  connection_packet_nr_ = src.connection_packet_nr_;
  timestamp_ = src.timestamp_;
  nr_of_signal_types_ = src.nr_of_signal_types_;

  nr_channels_ = src.nr_channels_;
  samples_per_channel_ = src.samples_per_channel_;
  data_ = src.data_;

  std::map<boost::uint32_t, RawMem3*> raw_map_;
}

//-----------------------------------------------------------------------------

DataPacket3::DataPacket3(void* mem)
  : flags_(0), connection_packet_nr_(0), nr_of_signal_types_(0)
{
  #ifdef DEBUG
      cout << "DataPacket: RAW Constructor" << endl;
  #endif

  uint8_t* version_ptr = reinterpret_cast<uint8_t*>(mem);
  version_ = *version_ptr;

  if(CURRENT_DATA_PACKET_VERSION != version_)
  {
    string ex_str("DataPacket(void* mem): ERROR -- DataPacket versions don't match!");
    ex_str = ex_str + " - Received packet version: ";
    ex_str += version_;
    ex_str = ex_str + " - Needed packet version: ";
    ex_str += CURRENT_DATA_PACKET_VERSION;
    throw(std::runtime_error(ex_str));
  }

  uint32_t* ui32_ptr = reinterpret_cast<uint32_t*>(++version_ptr);
  ++ui32_ptr;         // FIXME ... skip size field
  flags_ = *ui32_ptr;

  nr_of_signal_types_ = calcNrOfSignalTypes(flags_);

  uint64_t* ui64_ptr = reinterpret_cast<uint64_t*>(++ui32_ptr);
  packet_id_ = *ui64_ptr;
  ui64_ptr++;
  connection_packet_nr_ = *ui64_ptr;

  uint64_t* time_ptr
    = reinterpret_cast<uint64_t*>(++ui64_ptr);
  timestamp_ = *time_ptr;

  uint16_t* ui16_ptr = reinterpret_cast<uint16_t*>(++time_ptr);
  for(unsigned int n = 0; n < nr_of_signal_types_; n++)
  {
    nr_channels_.push_back(*ui16_ptr);
    ui16_ptr++;
  }

  for(unsigned int n = 0; n < nr_of_signal_types_; n++)
  {
    samples_per_channel_.push_back(*ui16_ptr);
    ui16_ptr++;
  }

  float* data_ptr = reinterpret_cast<float*>(ui16_ptr);
  for(unsigned int j = 0; j < nr_channels_.size(); j++)
    for(int32_t k = 0; k < (nr_channels_[j] * samples_per_channel_[j] ) ; k++)
    {
      data_.push_back(*data_ptr);
      data_ptr++;
    }
}

//-----------------------------------------------------------------------------
DataPacket3::~DataPacket3()
{
	for(std::map<boost::uint32_t, RawMem3*>::iterator it(raw_map_.begin());
		it != raw_map_.end(); it++)
	{
		delete(it->second);
	}
}


//-----------------------------------------------------------------------------

void DataPacket3::reset()
{
  #ifdef DEBUG
    cout << "DataPacket: reset" << endl;
  #endif

  flags_ = 0;
  connection_packet_nr_ = 0;
  nr_of_signal_types_ = 0;
  samples_per_channel_.clear();
  nr_channels_.clear();
  setTimestamp();
  data_.clear();
  for(map<uint32_t, RawMem3*>::iterator it(raw_map_.begin()); it != raw_map_.end(); it++)
    if(it->second)
      delete(it->second);
}

//-----------------------------------------------------------------------------

void DataPacket3::incPacketID()
{
  packet_id_++;
}

//-----------------------------------------------------------------------------

void DataPacket3::setPacketID(boost::uint64_t nr)
{
  packet_id_ = nr;
}

//-----------------------------------------------------------------------------

void DataPacket3::insertDataBlock(vector<double> v, uint32_t signal_flag, uint16_t blocks, bool prepend)
{

  if(!flagsOK())
    throw(std::logic_error("DataPacket3::insertDataBlock() -- Flags differ from Amount of Signals in DataPacket!"));

  if(calcNrOfSignalTypes(signal_flag) != 1)
    throw(std::logic_error("DataPacket3::insertDataBlock() -- Trying to insert more or less than 1 signal type at the same time!"));

  if(hasFlag(signal_flag))
  {
    if(prepend)
      prependDataBlock(v, signal_flag, blocks);
    else
      appendDataBlock(v, signal_flag, blocks);

    return;
    /*     throw(std::logic_error("DataPacket3::insertDataBlock() -- Error ... Flag already defined, \
        equal signal types simultaneously from different sources not supported at the moment!")); */
  }

  uint32_t pos = getDataPos(signal_flag);

  setFlag(signal_flag);

  vector<double>::iterator it_data(data_.begin() + getOffset(pos));
  data_.insert(it_data,v.begin(),v.end());

  vector<uint16_t>::iterator it_nr(samples_per_channel_.begin() + pos );
  samples_per_channel_.insert(it_nr, blocks);

  it_nr = nr_channels_.begin() + pos;
  nr_channels_.insert(it_nr, v.size()/blocks);

  nr_of_signal_types_++;
}

//-----------------------------------------------------------------------------

void DataPacket3::appendDataBlock(vector<double> &v, uint32_t signal_flag, uint16_t blocks)
{
  if(getNrOfBlocks(signal_flag) != blocks)
    throw(std::logic_error("DataPacket3::appendDataBlock() -- Blocksize of appended signal does not \
          match the stored one -- check your settings!"));

  uint32_t pos = getDataPos(signal_flag);

  vector<double>::iterator it_data(data_.begin() + getOffset(pos) +
                                   (nr_channels_[pos] * samples_per_channel_[pos] ) );
  data_.insert(it_data,v.begin(),v.end());

  nr_channels_[pos] += v.size()/blocks;
}

//-----------------------------------------------------------------------------

void DataPacket3::prependDataBlock(vector<double> &v, uint32_t signal_flag, uint16_t blocks)
{
  if(getNrOfBlocks(signal_flag) != blocks)
    throw(std::logic_error("DataPacket3::appendDataBlock() -- Blocksize of appended signal does not \
          match the stored one -- check your settings!"));

  uint32_t pos = getDataPos(signal_flag);

  vector<double>::iterator it_data(data_.begin() + getOffset(pos));
  data_.insert(it_data,v.begin(),v.end());

  nr_channels_[pos] += v.size()/blocks;
}

//-----------------------------------------------------------------------------

void DataPacket3::setConnectionPacketNr(boost::uint64_t nr)
{
  connection_packet_nr_ = nr;
}

//--------------------------------------------------------------------

uint64_t DataPacket3::getConnectionPacketNr()
{
  return(connection_packet_nr_);
}

//-------------------------------------------------------------------

void DataPacket3::setTimestamp()
{
    timestamp_ =  Clock::instance ().getMicroSeconds ();
}

//-----------------------------------------------------------------------------

bool DataPacket3::hasFlag(boost::uint32_t f)
{
  return( (f & flags_) == f);
}

//-----------------------------------------------------------------------------

boost::uint32_t DataPacket3::getFlags()
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket3::getFlags() -- Flags differ from Amount of Signals in DataPacket!"));
  return(flags_);
}

//-----------------------------------------------------------------------------

boost::uint64_t DataPacket3::getPacketID()
{
  return(packet_id_);
}

//-----------------------------------------------------------------------------

boost::uint16_t DataPacket3::getNrOfSignalTypes()
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket3::getNrOfSignalTypes() -- Flags differ from Amount of Signals in DataPacket!"));

  return(nr_of_signal_types_);
}

//-----------------------------------------------------------------------------

vector<boost::uint16_t> DataPacket3::getSamplesPerChannel()
{
  return(samples_per_channel_);
}

//-----------------------------------------------------------------------------

vector<uint16_t> DataPacket3::getNrOfChannels()
{
  return(nr_channels_);
}

//-----------------------------------------------------------------------------

const vector<double>& DataPacket3::getData()
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket3::getData() -- Flags differ from Amount of Signals in DataPacket!"));
  return(data_);
}

//-----------------------------------------------------------------------------

vector<double> DataPacket3::getSingleDataBlock(boost::uint32_t flag)
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket3::getSingleDataBlock() -- Flags differ from Amount of Signals in DataPacket!"));

  if(!hasFlag(flag))
    throw(std::invalid_argument("DataPacket3::getSingleDataBlock() -- Error ... Flag not set, unable to get Data!"));

  uint32_t position = getDataPos(flag);

  vector<double> v(data_.begin() + getOffset(position),  \
              data_.begin() + getOffset(position) + \
              ( nr_channels_[position] * samples_per_channel_[position] ) );
  return(v);
}

//-----------------------------------------------------------------------------

boost::uint16_t DataPacket3::getNrOfValues(boost::uint32_t flag)
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket3::getNrOfValues() -- Flags differ from Amount of Signals in DataPacket!"));

  if(!hasFlag(flag))
    return(0);

  return( getNrOfChannels(flag) * getSamplesPerChannel(flag) );
}

//-----------------------------------------------------------------------------

boost::uint16_t DataPacket3::getNrOfChannels(boost::uint32_t flag)
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket3::getNrOfChannels() -- Flags differ from Amount of Signals in DataPacket!"));

  if(!hasFlag(flag))
    return(0);
//    throw(std::invalid_argument("DataPacket3::getNrOfChannels() -- Error ... Flag not set, unable to get Data!"));

  return(nr_channels_[getDataPos(flag)]);
}

//-----------------------------------------------------------------------------

boost::uint16_t DataPacket3::getSamplesPerChannel(boost::uint32_t flag)
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket3::getSamplesPerChannel() -- Flags differ from Amount of Signals in DataPacket!"));

  if(!hasFlag(flag))
    return(0);
//    throw(std::invalid_argument("DataPacket3::getSamplesPerChannel() -- Error ... Flag not set, unable to get Data!"));

  return(samples_per_channel_[getDataPos(flag)]);
}

//-----------------------------------------------------------------------------

boost::uint16_t DataPacket3::getNrOfBlocks(boost::uint32_t flag)
{
  return(getSamplesPerChannel(flag));
}

//-----------------------------------------------------------------------------

boost::uint16_t DataPacket3::calcNrOfSignalTypes(boost::uint32_t f)
{
  uint16_t count = 0;
  uint32_t shift = 1;

  for(unsigned int n = 0; n < sizeof(f) *8; n++)
    count += (((shift << n) & f) >= 1) ;

  return(count);
}

//-----------------------------------------------------------------------------

bool DataPacket3::flagsOK()
{
  if(calcNrOfSignalTypes(flags_) != nr_of_signal_types_)
    return(false);

  return(true);
}

//-----------------------------------------------------------------------------

void DataPacket3::setFlag(boost::uint32_t f)
{
  flags_ |= f;
}

//-----------------------------------------------------------------------------

boost::uint32_t DataPacket3::getDataPos(boost::uint32_t flag)
{
  if(flag > flags_)
    return(nr_of_signal_types_);

  uint32_t pos = 0;
  uint32_t shift = 1;

  while(shift < flag)
  {
    if(flags_ & shift)
      pos++;
    shift <<= 1;
  }

  return(pos);
}

//-----------------------------------------------------------------------------

boost::uint32_t DataPacket3::getOffset(boost::uint32_t pos)
{
  uint32_t offset = 0;

  for(uint32_t n = 0; n < pos; n++)
    offset += (nr_channels_[n] * samples_per_channel_[n]);
  return(offset);
}

//-----------------------------------------------------------------------------

boost::uint32_t DataPacket3::getRawMemorySize()
{
  map<uint32_t, RawMem3*>::iterator it(raw_map_.find(flags_));

  if(it == raw_map_.end()) return 0;

  return it->second->size();
}

//-----------------------------------------------------------------------------

void* DataPacket3::getRaw()
{
  map<uint32_t, RawMem3*>::iterator it(raw_map_.find(flags_));
  RawMem3* r;

  if(it == raw_map_.end())
  {
    if(connection_packet_nr_ == 0)
      connection_packet_nr_ = packet_id_;

    r = new RawMem3(version_, flags_, packet_id_, connection_packet_nr_, \
                    timestamp_, nr_channels_, samples_per_channel_, data_);

    raw_map_.insert(make_pair(flags_,r));
  }
  else
    r = it->second;

  return(r->getMemPtr());
}

//-----------------------------------------------------------------------------

boost::uint32_t DataPacket3::getRequiredRawMemorySize()
{
  uint32_t size = sizeof(version_) + sizeof(uint32_t)\
                  + sizeof(flags_) + sizeof(packet_id_) + sizeof(connection_packet_nr_) \
                  + sizeof(timestamp_) \
                  + samples_per_channel_.size() * sizeof(uint16_t) \
                  + nr_channels_.size() * sizeof(uint16_t) \
                  + data_.size() * sizeof(float);  // FIXME  ...  hardcoded sizeof() !!!!
  return(size);
}

//-----------------------------------------------------------------------------

boost::uint32_t DataPacket3::getRequiredRawMemorySize(void* mem, boost::int32_t ba)
{
  uint32_t bytes_available = 0;

  if(ba <= 0)
    return(0);
  else
    bytes_available = ba;

  uint8_t* version_ptr = reinterpret_cast<uint8_t*>(mem);
  uint8_t version = *version_ptr;

  if(CURRENT_DATA_PACKET_VERSION != version)
  {
    string ex_str("DataPacket(void* mem): ERROR -- DataPacket versions don't match!");
    ex_str = ex_str + " - Received packet version: ";
    ex_str += version_;
    ex_str = ex_str + " - Needed packet version: ";
    ex_str += CURRENT_DATA_PACKET_VERSION;
    throw(std::runtime_error(ex_str));
  }

  vector<uint16_t>  samples_per_channel;
  vector<uint16_t>  nr_channels;

  uint32_t* ui32_ptr = reinterpret_cast<uint32_t*>(++version_ptr);

  *ui32_ptr++;      ///< FIXME:  size field is skipped

  uint32_t flags  = *ui32_ptr;

  uint32_t nr_of_signal_types = calcNrOfSignalTypes(flags);

  uint32_t size = sizeof(version) + sizeof(flags) + sizeof(packet_id_) \
                  + sizeof(connection_packet_nr_) + sizeof(timestamp_);

  if(size > bytes_available)
    return(0);

  uint64_t* ui64_ptr = reinterpret_cast<uint64_t*>(++ui32_ptr);
  ui64_ptr++;
  boost::posix_time::ptime* ptime_ptr
    = reinterpret_cast<boost::posix_time::ptime*>(++ui64_ptr);

  uint16_t* ui16_ptr = reinterpret_cast<uint16_t*>(++ptime_ptr);
  for(unsigned int n = 0; n < nr_of_signal_types; n++)
  {
    size += sizeof(uint16_t);
    if(size > bytes_available)
      return(0);

    nr_channels.push_back(*ui16_ptr);
    ui16_ptr++;
  }

  for(unsigned int n = 0; n < nr_of_signal_types; n++)
  {
    size += sizeof(uint16_t);
    if(size > bytes_available)
      return(0);

    samples_per_channel.push_back(*ui16_ptr);
    ui16_ptr++;
  }

  for(unsigned int j = 0; j < nr_channels.size(); j++)
      size +=  sizeof(float) * (nr_channels[j] * samples_per_channel[j]) ;

//  for(unsigned int j = 0; j < nr_values.size(); j++)
//    for(unsigned int k = 0; k < (nr_channels[j] * samples_per_channel[j]); k++)
//      size +=  sizeof(float);

    return(size);
}

} // Namespace tobiss

//-----------------------------------------------------------------------------

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
* @file data_packet.cpp
**/

#include <sstream>
#include <stdexcept>

#include "tia/defines.h"
#include "tia-private/datapacket/data_packet_impl__tmp.h"
#include "tia-private/datapacket/raw_mem.h"

namespace tia
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

uint64_t  DataPacketImpl::sample_nr_ = 0;

//-----------------------------------------------------------------------------
void DataPacketImpl::reset(void* mem)
{
  #ifdef DEBUG
      cout << "DataPacket: RAW Constructor" << endl;
  #endif

  flags_ = 0;
  packet_nr_ = 0;
  nr_of_signal_types_ = 0;

  uint32_t* f = reinterpret_cast<uint32_t*>(mem);

  if(PACKETCODE != ( *f & PACKETCODE ))
  {
    string ex_str("DataPacket(void* mem): ERROR -- PacketCode doesn't match!");
    throw(std::runtime_error(ex_str));
  }

  if(PACKETVERSION != ( *f & PACKETVERSION ))
  {
    stringstream ss (stringstream::in | stringstream::out);
    string ex_str("DataPacket(void* mem): ERROR -- Packet versions don't match!");
    ex_str = ex_str + " -Raw packet version: ";
    ss << ( (*f >> (8*sizeof(flags_)-BYTES4SIGNALFLAGS)) );
    ss >> ex_str;

    ss << ( (PACKETVERSION >> (8*sizeof(flags_)-BYTES4SIGNALFLAGS)) );
    ex_str = ex_str + " -Needed packet version: ";
    ss >> ex_str;
    throw(std::invalid_argument(ex_str));
  }

  flags_ = (*f & (~PACKETVERSION) & (~PACKETCODE) );

  nr_of_signal_types_ = calcNrOfSignalTypes(flags_);

  uint64_t* ui64_ptr = reinterpret_cast<uint64_t*>(++f);
  sample_nr_ = *ui64_ptr;
  ui64_ptr++;
  packet_nr_ = *ui64_ptr;

  boost::posix_time::ptime* ptime_ptr
    = reinterpret_cast<boost::posix_time::ptime*>(++ui64_ptr);
  timestamp_ = *ptime_ptr;

  uint16_t* ui16_ptr = reinterpret_cast<uint16_t*>(++ptime_ptr);
  for(unsigned int n = 0; n < nr_of_signal_types_; n++)
  {
    nr_values_.push_back(*ui16_ptr);
    ui16_ptr++;
  }

  for(unsigned int n = 0; n < nr_of_signal_types_; n++)
  {
    nr_blocks_.push_back(*ui16_ptr);
    ui16_ptr++;
  }

  float* data_ptr = reinterpret_cast<float*>(ui16_ptr);
  for(unsigned int j = 0; j < nr_values_.size(); j++)
    for(unsigned int k = 0; k < nr_values_[j]; k++)
    {
      data_.push_back(*data_ptr);
      data_ptr++;
    }
}

//-----------------------------------------------------------------------------
DataPacketImpl::~DataPacketImpl()
{
	for(std::map<boost::uint32_t, RawMem*>::iterator it(raw_map_.begin());
		it != raw_map_.end(); it++)
	{
		delete(it->second);
	}
}


//-----------------------------------------------------------------------------

void DataPacketImpl::reset()
{
  #ifdef DEBUG
    cout << "DataPacket: reset" << endl;
  #endif

  flags_ = 0;
  packet_nr_ = 0;
  nr_of_signal_types_ = 0;
  nr_blocks_.clear();
  nr_values_.clear();
  setTimestamp();
  data_.clear();
  for(map<uint32_t, RawMem*>::iterator it(raw_map_.begin()); it != raw_map_.end(); it++)
    if(it->second)
      delete(it->second);
}

//-----------------------------------------------------------------------------

void DataPacketImpl::incSampleNr()
{
  sample_nr_++;
}

//-----------------------------------------------------------------------------

void DataPacketImpl::insertDataBlock(vector<double> v, uint32_t signal_flag, uint16_t blocks, bool prepend)
{

  if(!flagsOK())
    throw(std::logic_error("DataPacket::insertDataBlock() -- Flags differ from Amount of Signals in DataPacket!"));

  if(calcNrOfSignalTypes(signal_flag) >1)
    throw(std::logic_error("DataPacket::insertDataBlock() -- Trying to insert more than 1 signal type at the same time!"));

  if(hasFlag(signal_flag))
  {
    if(prepend)
      prependDataBlock(v, signal_flag, blocks);
    else
      appendDataBlock(v, signal_flag, blocks);

    return;
    /*     throw(std::logic_error("DataPacket::insertDataBlock() -- Error ... Flag already defined, \
        equal signal types simultaneously from different sources not supported at the moment!")); */
  }

  uint32_t pos = getDataPos(signal_flag);

  setFlag(signal_flag);

  vector<double>::iterator it_data(data_.begin() + getOffset(pos));
  data_.insert(it_data,v.begin(),v.end());

  vector<uint16_t>::iterator it_nr(nr_blocks_.begin() + pos );
  nr_blocks_.insert(it_nr, blocks);

  it_nr = nr_values_.begin() + pos;
  nr_values_.insert(it_nr, v.size());

  nr_of_signal_types_++;
}

//-----------------------------------------------------------------------------

void DataPacketImpl::appendDataBlock(vector<double> &v, uint32_t signal_flag, uint16_t blocks)
{
  if(getNrOfBlocks(signal_flag) != blocks)
    throw(std::logic_error("DataPacket::appendDataBlock() -- Blocksize of appended signal does not \
          match the stored one -- check your settings!"));

  uint32_t pos = getDataPos(signal_flag);

  vector<double>::iterator it_data(data_.begin() + getOffset(pos) + nr_values_[pos]);
  data_.insert(it_data,v.begin(),v.end());

  nr_values_[pos] += v.size();
}

//-----------------------------------------------------------------------------

void DataPacketImpl::prependDataBlock(vector<double> &v, uint32_t signal_flag, uint16_t blocks)
{
  if(getNrOfBlocks(signal_flag) != blocks)
    throw(std::logic_error("DataPacket::appendDataBlock() -- Blocksize of appended signal does not \
          match the stored one -- check your settings!"));

  uint32_t pos = getDataPos(signal_flag);

  vector<double>::iterator it_data(data_.begin() + getOffset(pos));
  data_.insert(it_data,v.begin(),v.end());

  nr_values_[pos] += v.size();
}

//-----------------------------------------------------------------------------

void DataPacketImpl::setPacketNr(uint64_t nr)
{
  packet_nr_ = nr;
}

//-----------------------------------------------------------------------------

uint64_t DataPacketImpl::getPacketNr()
{
  return(packet_nr_);
}

//-----------------------------------------------------------------------------

void DataPacketImpl::setBoostPosixTimestamp()
{
  timestamp_ =  boost::posix_time::microsec_clock::local_time();
}

//-----------------------------------------------------------------------------

bool DataPacketImpl::hasFlag(boost::uint32_t f)
{
  return( (f & flags_) == f);
}

//-----------------------------------------------------------------------------

boost::uint32_t DataPacketImpl::getFlags()
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket::getFlags() -- Flags differ from Amount of Signals in DataPacket!"));
  return(flags_ | PACKETVERSION);
}

//-----------------------------------------------------------------------------

boost::uint64_t DataPacketImpl::getSampleNr()
{
  return(sample_nr_);
}

//-----------------------------------------------------------------------------

boost::uint16_t DataPacketImpl::getNrOfSignalTypes()
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket::getNrOfSignalTypes() -- Flags differ from Amount of Signals in DataPacket!"));

  return(nr_of_signal_types_);
}

//-----------------------------------------------------------------------------

vector<boost::uint16_t> DataPacketImpl::getNrOfBlocks()
{
  return(nr_blocks_);
}

//-----------------------------------------------------------------------------

vector<uint16_t> DataPacketImpl::getNrOfSamples()
{
  return(nr_values_);
}

//-----------------------------------------------------------------------------

const vector<double>& DataPacketImpl::getData()
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket::getData() -- Flags differ from Amount of Signals in DataPacket!"));
  return(data_);
}

//-----------------------------------------------------------------------------

vector<double> DataPacketImpl::getSingleDataBlock(boost::uint32_t flag)
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket::getSingleDataBlock() -- Flags differ from Amount of Signals in DataPacket!"));

  if(!hasFlag(flag))
    throw(std::invalid_argument("DataPacket::getSingleDataBlock() -- Error ... Flag not set, unable to get Data!"));

  uint32_t position = getDataPos(flag);

  vector<double> v(data_.begin()    + getOffset(position),  \
              data_.begin() + getOffset(position) + nr_values_.at(position));
  return(v);
}

//-----------------------------------------------------------------------------

boost::uint16_t DataPacketImpl::getNrOfSamples(boost::uint32_t flag)
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket::getNrOfValues() -- Flags differ from Amount of Signals in DataPacket!"));

  if(!hasFlag(flag))
    throw(std::invalid_argument("DataPacket::getNrOfValues() -- Error ... Flag not set, unable to get Data!"));

  return(nr_values_[getDataPos(flag)]);
}

//-----------------------------------------------------------------------------

boost::uint16_t DataPacketImpl::getNrOfBlocks(boost::uint32_t flag)
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket::getNrOfBlocks() -- Flags differ from Amount of Signals in DataPacket!"));

  if(!hasFlag(flag))
    throw(std::invalid_argument("DataPacket::getNrOfBlocks() -- Error ... Flag not set, unable to get Data!"));

  return(nr_blocks_[getDataPos(flag)]);
}

//-----------------------------------------------------------------------------

boost::uint16_t DataPacketImpl::calcNrOfSignalTypes(boost::uint32_t f)
{
  uint16_t count = 0;
  uint32_t shift = 1;
  for(unsigned int n = 0; n < (sizeof(f)*(BYTES4SIGNALFLAGS*2)); n++)
    count += (((shift << n) & f) >= 1) ;

  return(count);
}

//-----------------------------------------------------------------------------

bool DataPacketImpl::flagsOK()
{
  if(calcNrOfSignalTypes(flags_) != nr_of_signal_types_)
    return(false);

  return(true);
}

//-----------------------------------------------------------------------------

void DataPacketImpl::setFlag(boost::uint32_t f)
{
  flags_ |= f;
}

//-----------------------------------------------------------------------------

boost::uint32_t DataPacketImpl::getDataPos(boost::uint32_t flag)
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

boost::uint32_t DataPacketImpl::getOffset(boost::uint32_t pos)
{
  uint32_t offset = 0;

  for(uint32_t n = 0; n<pos; n++)
    offset += nr_values_[n];
  return(offset);
}

//-----------------------------------------------------------------------------

boost::uint32_t DataPacketImpl::getRawMemorySize()
{
  map<uint32_t, RawMem*>::iterator it(raw_map_.find(flags_));

  if(it == raw_map_.end()) return 0;

  return it->second->size();
}

//-----------------------------------------------------------------------------

void* DataPacketImpl::getRaw()
{
  map<uint32_t, RawMem*>::iterator it(raw_map_.find(flags_));
  RawMem* r;

  if(it == raw_map_.end())
  {
    if(packet_nr_ == 0)
      packet_nr_ = sample_nr_;

    r = new RawMem((flags_ | PACKETVERSION | PACKETCODE) , sample_nr_, packet_nr_, \
                    timestamp_, nr_values_, nr_blocks_, data_);

    raw_map_.insert(make_pair(flags_,r));
  }
  else
    r = it->second;

  return(r->getMemPtr());
}

//-----------------------------------------------------------------------------

boost::uint32_t DataPacketImpl::getRequiredRawMemorySize()
{
  uint32_t size = sizeof(flags_) + sizeof(sample_nr_) + sizeof(packet_nr_) \
    + sizeof(timestamp_) \
    + nr_blocks_.size() * sizeof(nr_blocks_[0]) \
    + nr_values_.size() * sizeof(nr_values_[0]) \
    + data_.size() * sizeof(float);  // FIXME  ...  hardcoded sizeof() !!!!
  return(size);
}

//-----------------------------------------------------------------------------

boost::uint32_t DataPacketImpl::getRequiredRawMemorySize(void* mem, boost::uint32_t ba)
{
  uint32_t bytes_available = 0;

  if(ba <= 0)
    return(0);
  else
    bytes_available = ba;

  uint32_t* f = reinterpret_cast<uint32_t*>(mem);

  if(PACKETCODE != ( *f & PACKETCODE ))
  {
    string ex_str("DataPacket(void* mem, int32_t ba): ERROR -- PacketCode doesn't match!");
    throw(std::runtime_error(ex_str));
  }

  if(PACKETVERSION != ( *f & PACKETVERSION ))
  {
    stringstream ss (stringstream::in | stringstream::out);
    string ex_str("DataPacket(void* mem, int32_t ba): ERROR -- Packet versions don't match!");
    ex_str = ex_str + " -Raw packet version: ";
    ss << ( (*f >> (8*sizeof(flags_)-BYTES4SIGNALFLAGS)) );
    ss >> ex_str;

    ss << ( (PACKETVERSION >> (8*sizeof(flags_)-BYTES4SIGNALFLAGS)) );
    ex_str = ex_str + " -Needed packet version: ";
    ss >> ex_str;
    throw(std::invalid_argument(ex_str));
  }

  vector<uint16_t>  nr_blocks;
  vector<uint16_t>  nr_values;

  uint32_t flags  = (*f & (~PACKETVERSION) & (~PACKETCODE) );

  uint32_t nr_of_signal_types = calcNrOfSignalTypes(flags);

  uint32_t size = sizeof(flags) + sizeof(sample_nr_) \
                  + sizeof(packet_nr_) + sizeof(timestamp_);

  if(size > bytes_available)
    return(0);

  uint64_t* ui64_ptr = reinterpret_cast<uint64_t*>(++f);
  ui64_ptr++;
  boost::posix_time::ptime* ptime_ptr
    = reinterpret_cast<boost::posix_time::ptime*>(++ui64_ptr);

  uint16_t* ui16_ptr = reinterpret_cast<uint16_t*>(++ptime_ptr);
  for(unsigned int n = 0; n < nr_of_signal_types; n++)
  {
    size += sizeof(uint16_t);
    if(size > bytes_available)
      return(0);

    nr_values.push_back(*ui16_ptr);
    ui16_ptr++;
  }

  for(unsigned int n = 0; n < nr_of_signal_types; n++)
  {
    size += sizeof(uint16_t);
    if(size > bytes_available)
      return(0);

    nr_blocks.push_back(*ui16_ptr);
    ui16_ptr++;
  }

  for(unsigned int j = 0; j < nr_values.size(); j++)
    for(unsigned int k = 0; k < nr_values[j]; k++)
      size +=  sizeof(float);

    return(size);
}

} // Namespace tobiss

//-----------------------------------------------------------------------------

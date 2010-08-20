
#include "datapacket/data_packet.h"

namespace tobiss
{

using namespace std;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;
using boost::int32_t;

uint64_t  DataPacket::sample_nr_ = 0;

//-----------------------------------------------------------------------------
DataPacket::DataPacket(void* mem)
  : flags_(0), packet_nr_(0), nr_of_signal_types_(0)
{
  #ifdef DEBUG
      cout << "DataPacket: RAW Constructor" << endl;
  #endif

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

void DataPacket::reset()
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

void DataPacket::incSampleNr()
{
  sample_nr_++;
}

//-----------------------------------------------------------------------------

void DataPacket::insertDataBlock(vector<double> v, uint32_t signal_flag, uint16_t blocks, bool prepend)
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

void DataPacket::appendDataBlock(vector<double> &v, uint32_t signal_flag, uint16_t blocks)
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

void DataPacket::prependDataBlock(vector<double> &v, uint32_t signal_flag, uint16_t blocks)
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

void DataPacket::setPacketNr(uint64_t nr)
{
  packet_nr_ = nr;
}

//-----------------------------------------------------------------------------

uint64_t DataPacket::getPacketNr()
{
  return(packet_nr_);
}

//-----------------------------------------------------------------------------

void DataPacket::setTimestamp()
{
  timestamp_ =  boost::posix_time::microsec_clock::local_time();
}

//-----------------------------------------------------------------------------

bool DataPacket::hasFlag(uint32_t f)
{
  return( (f & flags_) == f);
}

//-----------------------------------------------------------------------------

uint32_t DataPacket::getFlags()
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket::getFlags() -- Flags differ from Amount of Signals in DataPacket!"));
  return(flags_ | PACKETVERSION);
}

//-----------------------------------------------------------------------------

uint64_t DataPacket::getSampleNr()
{
  return(sample_nr_);
}

//-----------------------------------------------------------------------------

uint16_t DataPacket::getNrOfSignalTypes()
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket::getNrOfSignalTypes() -- Flags differ from Amount of Signals in DataPacket!"));

  return(nr_of_signal_types_);
}

//-----------------------------------------------------------------------------

vector<uint16_t> DataPacket::getNrOfBlocks()
{
  return(nr_blocks_);
}

//-----------------------------------------------------------------------------

vector<uint16_t> DataPacket::getNrOfValues()
{
  return(nr_values_);
}

//-----------------------------------------------------------------------------

vector<double> DataPacket::getData()
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket::getData() -- Flags differ from Amount of Signals in DataPacket!"));
  return(data_);
}

//-----------------------------------------------------------------------------

vector<double> DataPacket::getSingleDataBlock(uint32_t flag)
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

uint16_t DataPacket::getNrOfValues(uint32_t flag)
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket::getNrOfValues() -- Flags differ from Amount of Signals in DataPacket!"));

  if(!hasFlag(flag))
    throw(std::invalid_argument("DataPacket::getNrOfValues() -- Error ... Flag not set, unable to get Data!"));

  return(nr_values_[getDataPos(flag)]);
}

//-----------------------------------------------------------------------------

uint16_t DataPacket::getNrOfBlocks(uint32_t flag)
{
  if(!flagsOK())
    throw(std::logic_error("DataPacket::getNrOfBlocks() -- Flags differ from Amount of Signals in DataPacket!"));

  if(!hasFlag(flag))
    throw(std::invalid_argument("DataPacket::getNrOfBlocks() -- Error ... Flag not set, unable to get Data!"));

  return(nr_blocks_[getDataPos(flag)]);
}

//-----------------------------------------------------------------------------

uint16_t DataPacket::calcNrOfSignalTypes(uint32_t f)
{
  uint16_t count = 0;
  uint32_t shift = 1;
  for(unsigned int n = 0; n < (sizeof(f)*(BYTES4SIGNALFLAGS*2)); n++)
    count += (((shift << n) & f) >= 1) ;

  return(count);
}

//-----------------------------------------------------------------------------

bool DataPacket::flagsOK()
{
  if(calcNrOfSignalTypes(flags_) != nr_of_signal_types_)
    return(false);

  return(true);
}

//-----------------------------------------------------------------------------

void DataPacket::setFlag(uint32_t f)
{
  flags_ |= f;
}

//-----------------------------------------------------------------------------

uint32_t DataPacket::getDataPos(uint32_t flag)
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

uint32_t DataPacket::getOffset(uint32_t pos)
{
  uint32_t offset = 0;

  for(uint32_t n = 0; n<pos; n++)
    offset += nr_values_[n];
  return(offset);
}

//-----------------------------------------------------------------------------

uint32_t DataPacket::getRawMemorySize()
{
  map<uint32_t, RawMem*>::iterator it(raw_map_.find(flags_));

  if(it == raw_map_.end()) return 0;

  return it->second->size();
}

//-----------------------------------------------------------------------------

void* DataPacket::getRaw()
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

uint32_t DataPacket::getRequiredRawMemorySize()
{
  uint32_t size = sizeof(flags_) + sizeof(sample_nr_) + sizeof(packet_nr_) \
    + sizeof(timestamp_) \
    + nr_blocks_.size() * sizeof(nr_blocks_[0]) \
    + nr_values_.size() * sizeof(nr_values_[0]) \
    + data_.size() * sizeof(float);  // FIXME  ...  hardcoded sizeof() !!!!
  return(size);
}

//-----------------------------------------------------------------------------

uint32_t DataPacket::getRequiredRawMemorySize(void* mem, int32_t ba)
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

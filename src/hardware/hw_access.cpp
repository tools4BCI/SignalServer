/*
    This file is part of the TOBI SignalServer.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU General Public License Usage
    Alternatively, this file may be used under the terms of the GNU
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file gpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/gpl.html.

    In case of GNU General Public License Usage ,the TOBI SignalServer
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI SignalServer. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: SignalServer@tobi-project.org
*/

/**
* @file hw_access.cpp
**/

#include <iostream>
#include <fstream>

#include "hardware/hw_access.h"
#include "hardware/hw_thread_factory.h"
#include "config/xml_parser.h"

#include "tia/data_packet_interface.h"

#ifdef TIMING_TEST
  #include "LptTools/LptTools.h"
  #define LPT1  0
  #define LPT2  1
#endif


namespace tobiss
{
using boost::uint16_t;
using boost::uint32_t;

using std::vector;
using std::string;
using std::map;
using std::pair;
using std::cout;
using std::endl;
using std::make_pair;
using std::hex;
using std::dec;

//-----------------------------------------------------------------------------

HWAccess::HWAccess(boost::asio::io_service& io, XMLParser& parser)
  : master_(0), acqu_running_(0), force_constant_block_size_(0)
{
  #ifdef DEBUG
    cout << "HWAccess Constructor" << endl;
  #endif

  bool hw_not_found = false;
  try
  {
    force_constant_block_size_ = parser.forceConstantBlockSize();

    for(unsigned int n = 0; n < parser.getNrOfHardwareElements(); n++)
    {
      HWThread* thread = HWThreadFactory::instance().createHWThread (parser.getHardwareElementName(n), io, parser.getHardwareElement(n));
      if (thread)
      {
        slaves_.push_back (thread);
        cout << " * " << thread->getType() << " successfully initialized" << endl;
        cout << "    fs: " << thread->getSamplingRate();
        cout << "Hz, nr of channels: " << thread->getNrOfChannels();
        cout << ", blocksize: " << thread->getNrOfBlocks()  << endl;
      }
      else
      {
        cout << " Warning:  \"" << parser.getHardwareElementName(n) << "\" not supported" << endl;
        hw_not_found = true;
      }
    }
  }
  catch(std::invalid_argument& e)
  {
    string ex_str("Error in hardware - ");
    ex_str += + e.what();
    throw;
  }
  catch(ticpp::Exception& e)
  {
    string ex_str("Error in hardware (TICPP exception)- ");
    ex_str += + e.what();
    throw(std::invalid_argument(ex_str));
  }

  if(hw_not_found)
  {
    cout << " Possible hardware abrevations to be used  with this signal server version:" << endl;
    std::vector<std::string> hw_names = getPossibleHardwareNames();
    for(unsigned int n = 0; n < hw_names.size(); n++)
      cout << "   * " << hw_names[n] << endl;
  }

  buildDataInfoMap();
  buildFsInfoMap();
  doHWSetup();

  #ifdef TIMING_TEST
    lpt_flag_ = 0;
  #endif
}

//-----------------------------------------------------------------------------

HWAccess::~HWAccess()
{
  #ifdef DEBUG
    cout << "HWaccess: Destructor" << endl;
  #endif

  if(acqu_running_)
    stopDataAcquisition();

  for (unsigned int n=0; n < slaves_.size(); n++)
    if(slaves_[n])
      delete slaves_[n];
  for (unsigned int n=0; n < aperiodics_.size(); n++)
    if(aperiodics_[n])
      delete aperiodics_[n];

  if(master_)
    delete master_;
}

//-----------------------------------------------------------------------------

void HWAccess::buildDataInfoMap()
{
  #ifdef DEBUG
    cout << "HWAccess: buildDataInfoMap" << endl;
  #endif
  map<uint16_t, pair<string, uint32_t> > ch_info;     //( ch_nr,  (name,  type) )
  map<uint16_t, pair<string, uint32_t> >::iterator it;

  for(unsigned int n = 0; n < slaves_.size(); n++)
  {
    ch_info = slaves_[n]->getChannelInfoMap();
    it = ch_info.begin();

    for(unsigned int m = 0; m < ch_info.size(); m++)
    {
      uint32_t& type = it->second.second;
      string& name = it->second.first;
      if(data_info_.find( type ) == data_info_.end())
      { //  signal type the first time
        data_info_[type] = make_pair(1,slaves_[n]->getNrOfBlocks());
      }
      else
      { // signal type already found
        data_info_[type].first += 1;
      }
      channel_naming_[type].push_back(name);

      it++;
    }
  }

  //   cout << "Data Info Map:"  << endl;
  //   for(map<uint32_t, pair<uint16_t, uint16_t> >::iterator d_it(data_info_.begin());
  //         d_it != data_info_.end(); d_it++)
  //   {
  //     cout << "Type: " << d_it->first << "; ";
  //     cout << "Nr. of Ch: " << d_it->second.first << "; ";
  //     cout << "Blocksize: " << d_it->second.second << "; " << endl;
  //   }
}
//-----------------------------------------------------------------------------

void HWAccess::doHWSetup()
{
  #ifdef DEBUG
    cout << "HWAccess: checkHWSetup" << endl;
  #endif

  checkIfSingleMaster();
  setAperiodics();
  setMasterSlaveRatio();
  reportInHomogenousDevice(master_);
  for(unsigned int n = 0; n < slaves_.size(); n++)
    reportInHomogenousDevice(slaves_[n]);
}

//-----------------------------------------------------------------------------

void HWAccess::checkIfSingleMaster()
{
  #ifdef DEBUG
    cout << "HWAccess: checkIfSingleMaster" << endl;
  #endif

  int master_count = 0;
  for(unsigned int n = 0; n < slaves_.size(); n++)
  {
    master_count += slaves_[n]->isMaster();
    if(slaves_[n]->isMaster())
    {
      master_ = slaves_[n];
      slaves_.erase(slaves_.begin() + n);
      n--;
    }
  }

  if(master_count == 0)
  {
    string ex_str;
    ex_str = "HWAccess::checkIfSingleMaster() -- Error: No device defined as master!";
    throw(ticpp::Exception(ex_str));
  }
  if(master_count > 1)
  {
    string ex_str;
    ex_str = "HWAccess::checkIfSingleMaster() -- Error: More than 1 device defined as master!";
    throw(ticpp::Exception(ex_str));
  }
}

//-----------------------------------------------------------------------------

void HWAccess::setAperiodics()
{
  for(unsigned int n = 0; n < slaves_.size(); n++)
  {
    if(slaves_[n]->isAperiodic())
    {
      aperiodics_.push_back(slaves_[n]);
      slaves_.erase(slaves_.begin() + n);
      n--;
    }
  }
}

//-----------------------------------------------------------------------------

void HWAccess::setMasterSlaveRatio()
{
  #ifdef DEBUG
    cout << "HWAccess: setMasterSlaveRatio" << endl;
  #endif

  for(unsigned int n = 0; n < slaves_.size(); n++)
  {
    uint32_t master_ratio = master_->getSamplingRate()/master_->getNrOfBlocks();
    uint32_t slave_ratio = 0;
    uint32_t ratio = 0;
    if(!slaves_[n]->getSamplingRate() || !slaves_[n]->getNrOfBlocks())
        ratio = 1;
    else
    {
      slave_ratio  = slaves_[n]->getSamplingRate()/slaves_[n]->getNrOfBlocks();
      ratio = master_ratio/slave_ratio;
    }

    fs_ratio_.push_back(ratio);
    sample_it_.push_back(ratio);

    cout << "  --> Ratio between master_ and slave nr." << n+1 <<  ": " << fs_ratio_.back() << endl;
    cout << endl;
  }
}

//-----------------------------------------------------------------------------

void HWAccess::reportInHomogenousDevice(HWThread* dev)
{
  #ifdef DEBUG
    cout << "HWAccess: reportInHomogenousDevice" << endl;
  #endif

  if(!dev->hasHomogenousSignalType())
  {
    cout << "    --> \"" <<  dev->getType()  << "\" delivering different signal types!" << endl;

    vector<uint32_t> v = dev->getChannelTypes();

    cout << "     -- Channel Type-List: (hex)  ";
    for(unsigned int n = 0; n < v.size(); n++)
      cout << hex << v[n] << ", ";
    cout << dec << endl;
  }
}

//-----------------------------------------------------------------------------

void HWAccess::startDataAcquisition()
{
  #ifdef DEBUG
    cout << "HWAccess: startDataAcquisition" << endl;
  #endif

  cout << endl;

  if(acqu_running_)
    return;

  if(slaves_.size())
    cout << endl << " Slaves:" << endl;
  for(unsigned int n = 0; n < slaves_.size(); n++)
  {
    slaves_[n]->run();
    cout << " * " << slaves_[n]->getType() << " successfully started" << endl;
  }

  if(aperiodics_.size())
    cout << endl << " Aperiodic devices:" << endl;
  for(unsigned int n = 0; n < aperiodics_.size(); n++)
  {
    aperiodics_[n]->run();
    cout << " * " << aperiodics_[n]->getType() << " successfully started" << endl;
  }

  cout << endl << " Master:" << endl;
  master_->run();
  cout << " * " << master_->getType() << " successfully started" << endl;

  acqu_running_ = true;
}

//-----------------------------------------------------------------------------

void HWAccess::stopDataAcquisition()
{
  #ifdef DEBUG
    cout << "HWAccess: stopDataAcquisition" << endl;
  #endif

  if(!acqu_running_)
    return;

  cout << endl;
  master_->stop();
  cout << " * " << master_->getType() << " successfully stopped" << endl;
  for(unsigned int n = 0; n < slaves_.size(); n++)
  {
    slaves_[n]->stop();
    cout << " * " << slaves_[n]->getType() << " successfully started" << endl;
  }

  acqu_running_ = false;
}

//-----------------------------------------------------------------------------

void HWAccess::fillDataPacket(tia::DataPacket* packet)
{
  //#ifdef DEBUG
  //  cout << "HWAccess: getDataPacket" << endl;
  //#endif

  packet->reset();

  SampleBlock<double> sb(master_->getSyncData());
  packet->setTimestamp();

  #ifdef TIMING_TEST
    int port_state = LptPortIn(LPT1,0);
    if(!lpt_flag_)
    {
      lpt_flag_ = 1;
      LptPortOut(LPT1, 0, port_state | 0x01);
    }
    else
    {
      lpt_flag_ = 0;
      LptPortOut(LPT1, 0, port_state & ~0x01);
    }
  #endif

  for(int n = 0; n < sb.getNrOfSignalTypes() ; n++)
    packet->insertDataBlock(sb.getSignalByNr(n), sb.getFlagByNr(n), sb.getNrOfBlocks());


  for (unsigned int n=0; n < slaves_.size(); n++)
  {
    if (!force_constant_block_size_)
    {
      sb = slaves_[n]->getAsyncData();
      for (int j = 0; j < sb.getNrOfSignalTypes(); j++)
        packet->insertDataBlock(sb.getSignalByNr(j), sb.getFlagByNr(j), sb.getNrOfBlocks());
    }
    else
    {
      if (sample_it_[n] == fs_ratio_[n])
      {
        sb = slaves_[n]->getAsyncData();
        for (int j = 0; j < sb.getNrOfSignalTypes(); j++)
          packet->insertDataBlock(sb.getSignalByNr(j), sb.getFlagByNr(j), sb.getNrOfBlocks());
        sample_it_[n] = 0;
      }
      sample_it_[n]++;
    }
  }

  for (unsigned int n=0; n < aperiodics_.size(); n++)
  {
    sb = aperiodics_[n]->getAsyncData();
    for(int j = 0; j < sb.getNrOfSignalTypes() ; j++)
      packet->insertDataBlock(sb.getSignalByNr(j), sb.getFlagByNr(j), sb.getNrOfBlocks());
  }

  packet->incPacketID();
}

//-----------------------------------------------------------------------------

vector<uint32_t> HWAccess::getAcquiredSignalTypes()
{
  #ifdef DEBUG
    cout << "HWAccess: getAcquiredSignalTypes" << endl;
  #endif

  vector<uint32_t> v;
  for(map<uint32_t, pair<uint16_t, uint16_t> >::iterator it(data_info_.begin());
      it != data_info_.end(); it++)
    v.push_back(it->first);
  return(v);
}

//-----------------------------------------------------------------------------

vector<uint16_t> HWAccess::getBlockSizesPerSignalType()
{
  #ifdef DEBUG
    cout << "HWAccess: getBlockSizesPerSignalType" << endl;
  #endif

  vector<uint16_t> v;
  for(map<uint32_t, pair<uint16_t, uint16_t> >::iterator it(data_info_.begin());
      it != data_info_.end(); it++)
    v.push_back(it->second.second);
  return(v);
}

//-----------------------------------------------------------------------------

vector<uint16_t> HWAccess::getNrOfChannelsPerSignalType()
{
  #ifdef DEBUG
    cout << "HWAccess: getNrOfChannelsPerSignalType" << endl;
  #endif

  vector<uint16_t> v;
  for(map<uint32_t, pair<uint16_t, uint16_t> >::iterator it(data_info_.begin());
      it != data_info_.end(); it++)
    v.push_back(it->second.first);
  return(v);
}

//-----------------------------------------------------------------------------

uint16_t HWAccess::getNrOfChannels()
{
  #ifdef DEBUG
    cout << "HWAccess: getNrOfChannels" << endl;
  #endif

  uint16_t nr_ch = 0;

  for(map<uint32_t, pair<uint16_t, uint16_t> >::iterator it(data_info_.begin());
      it != data_info_.end(); it++)
    nr_ch += it->second.first;

  return(nr_ch);
}

//-----------------------------------------------------------------------------

void HWAccess::buildFsInfoMap()
{
  #ifdef DEBUG
    cout << "HWAccess::buildFsInfoMap" << endl;
  #endif

  map<uint16_t, pair<string, uint32_t> > ch_info;     //( ch_nr,  (name,  type) )
  map<uint16_t, pair<string, uint32_t> >::iterator it;

  for(unsigned int n = 0; n < slaves_.size(); n++)
  {
    ch_info = slaves_[n]->getChannelInfoMap();
    it = ch_info.begin();

    for(unsigned int m = 0; m < ch_info.size(); m++)
    {
      uint32_t& type = it->second.second;

      if(fs_info_.find( type ) == fs_info_.end())
      { //  signal type the first time
        fs_info_[type] = slaves_[n]->getSamplingRate();
      }
      else
      { // signal type already found
        if(fs_info_[type] != slaves_[n]->getSamplingRate())
          throw(std::runtime_error("HWAccess::buildFsInfoMap -- Sampling Rates per Signaltype differ!"));
      }
      it++;
    }
  }
}

//-----------------------------------------------------------------------------

vector<boost::uint32_t> HWAccess::getSamplingRatePerSignalType()
{
  #ifdef DEBUG
    cout << "HWAccess: getSamplingRatePerSignalType" << endl;
  #endif

  vector<boost::uint32_t> v;
  for(map<uint32_t, double>::iterator it(fs_info_.begin());
      it != fs_info_.end(); it++)
    v.push_back(it->second);
  return(v);
}

std::vector<std::string> HWAccess::getPossibleHardwareNames()
{
  return(HWThreadFactory::instance().getPossibleHardwareNames());
}

//-----------------------------------------------------------------------------

} // Namespace tobiss

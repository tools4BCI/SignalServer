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

#include "hardware/gBSamp_base.h"

#include <iostream>
#include <utility>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "tia/constants.h"

namespace tobiss
{

using std::string;
using std::cout;
using std::cerr;
using std::endl;

using boost::lexical_cast;
using boost::bad_lexical_cast;

const string gBSampBase::hw_fil_("filter");
const string gBSampBase::hw_fil_type_("type");
const string gBSampBase::hw_fil_order_("order");
const string gBSampBase::hw_fil_low_("f_low");
const string gBSampBase::hw_fil_high_("f_high");
const string gBSampBase::hw_fil_sense_("sense");
const string gBSampBase::hw_notch_("notch");
const string gBSampBase::hw_notch_center_("f_center");
const string gBSampBase::hw_daq_mode_("daq_mode");
const string gBSampBase::hw_device_id_("device_label");


//-----------------------------------------------------------------------------

gBSampBase::gBSampBase()
  : daq_mode_(RSE)
{
  daq_modes_map_.insert( std::make_pair("rse", RSE) );
  daq_modes_map_.insert( std::make_pair("nrse", NRSE) );
  daq_modes_map_.insert( std::make_pair("diff", diff) );

  setType("g.USBamp");
}

//-----------------------------------------------------------------------------

void gBSampBase::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{
  #ifdef DEBUG
    cout << "gBSamp: setHardware" << endl;
  #endif

  checkMandatoryHardwareTags(hw);
  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(hw_devset_, true));

  setDeviceSettings(ds);

  ticpp::Iterator<ticpp::Element> cs(hw->FirstChildElement(hw_chset_, false));
  if (cs != cs.end())
  {
    for(ticpp::Iterator<ticpp::Element> it(cs); ++it != it.end(); )
      if(it->Value() == hw_chset_)
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Multiple channel_settings found!";
        throw(ticpp::Exception(ex_str));
      }
      setChannelSettings(cs);
  }

  //TODO: set extra filtersettings per channel if wanted

}

//---------------------------------------------------------------------------------------

void gBSampBase::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "gBSamp: setDeviceSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_fs_,true));
  setSamplingRate(elem);

  elem = father->FirstChildElement(hw_device_id_,true);
  setDeviceName(elem);

  elem = father->FirstChildElement(hw_channels_,false);
  if(elem != elem.end())
    setDeviceChannels(elem);

  elem = father->FirstChildElement(hw_blocksize_,false);
  if(elem != elem.end())
    setBlocks(elem);

  elem = father->FirstChildElement(hw_daq_mode_,false);
  if(elem != elem.end())
    setAcquisitionMode(elem);

  ticpp::Iterator<ticpp::Element> filter(father->FirstChildElement(hw_fil_, false));
  if (filter != filter.end())
  {
    for(ticpp::Iterator<ticpp::Element> it(filter); ++it != it.end(); )
      if(it->Value() == hw_chset_)
        setDeviceFilterSettings(filter);
  }

}

//---------------------------------------------------------------------------------------

void gBSampBase::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "gBSamp: setChannelSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_chset_sel_,false));
  if (elem != elem.end())
    setChannelSelection(elem);
}

//---------------------------------------------------------------------------------------

void gBSampBase::setAcquisitionMode(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "gBSamp: setAcquisitionMode" << endl;
  #endif

  std::string mode = elem->GetText(true);

  std::map<string, daq_mode_type>::iterator it;

  it = daq_modes_map_.find(boost::algorithm::to_lower_copy(mode));

  if(it == daq_modes_map_.end())
  {
    string e = "g.BSamp acquisition mode \"" + mode + "\" not found -- please also check spelling!";
    throw(std::invalid_argument(e));
  }

  daq_mode_ = it->second;
}

//---------------------------------------------------------------------------------------

void gBSampBase::setDeviceName(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "gBSamp: setAcquisitionMode" << endl;
  #endif

  device_id_ = elem->GetText(true);
}


//---------------------------------------------------------------------------------------

void gBSampBase::setDeviceFilterSettings(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "gBSamp: setDeviceFilterSettings" << endl;
  #endif

  checkFilterAttributes(elem);

  unsigned int type = 0;
  bool notch = 0;
  float f_low = 0;
  float f_high = 0;
  float sense = 0;

  getFilterParams(elem, type, notch, f_low, f_high, sense);

  cout << " * g.BSamp -- filters per type set to:" << endl;
  cout << "    ...  type: " << type << ", f_low: " << f_low << ", f_high: " << f_high << ", sense: " << sense << ", notch: " << notch << endl;
  cout << endl;

}

//---------------------------------------------------------------------------------------

void gBSampBase::checkFilterAttributes(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: checkFilterAttributes" << endl;
  #endif

  if(!elem.Get()->HasAttribute(hw_fil_type_))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil_+"> given, filter type ("+hw_fil_type_+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_notch_))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil_+"> given, filter order ("+hw_notch_+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_fil_low_))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil_+"> given, lower cutoff frequency ("+hw_fil_low_+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_fil_high_))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil_+"> given, upper cutoff frequency ("+hw_fil_high_+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_fil_sense_))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil_+"> given, parameter ("+hw_fil_sense_+") not given!";
    throw(ticpp::Exception(ex_str));
  }
}

//---------------------------------------------------------------------------------------

void gBSampBase::getFilterParams(ticpp::Iterator<ticpp::Element>const &elem,\
  unsigned int &type, bool &notch, float &f_low, float &f_high, float &sense)
{
  #ifdef DEBUG
    cout << "gBSamp: getFilterParams" << endl;
  #endif

  // EEG 0.1 mV.   -->.  * 20  (Saettigung: 200 muV Ampl --> 400muV SS)
  //     0.05 mV...-->   * 10  (Saettigung: 100 muV Ampl)
  // EMG 1           ;  5
  // EOG 0.1         ;  2
  // ECG 2           ;  5

  tia::Constants cst;
  type = cst.getSignalFlag(elem.Get()->GetAttribute(hw_fil_type_));
  try
  {
    notch = equalsOnOrOff(elem.Get()->GetAttribute(hw_notch_));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil_+"> given, but notch is not 'on' or 'off'";
    throw(ticpp::Exception(ex_str));
  }
  if(type == SIG_EEG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_low_));
      if((f_low != .5) || (f_low != 2))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not '0.5' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_high_));
      if((f_high != 30) || (f_high != 100))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not '30' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_sense_));
      if((f_low != .05) || (f_low != .1))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not '0.05' or '.1'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }

  if(type == SIG_EOG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_low_));
      if((f_low != .5) || (f_low != 2))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not '0.5' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_high_));
      if((f_high != 30) || (f_high != 100))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not '30' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_sense_));
      if((f_low != .1) || (f_low != 1))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not '0.1' or '1'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }

  if(type == SIG_EMG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_low_));
      if((f_low != .01) || (f_low != 2))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not '0.01' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_high_));
      if((f_high != 60) || (f_high != 100))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not '60' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_sense_));
      if((f_low != 1) || (f_low != 5))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not '1' or '5'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }

  if(type == SIG_ECG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_low_));
      if((f_low != .01) || (f_low != 2))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not '0.01' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_high_));
      if((f_high != 60) || (f_high != 100))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not '60' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_sense_));
      if((f_low != 2) || (f_low != 5))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not '2' or '5'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil_+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }
}

//-----------------------------------------------------------------------------

}

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
#include <boost/lexical_cast.hpp>

#include "tia/constants.h"

namespace tobiss
{

using std::string;
using std::cout;
using std::cerr;
using std::endl;

using boost::lexical_cast;
using boost::bad_lexical_cast;

const string gBSampBase::hw_fil("filter");
const string gBSampBase::hw_fil_type("type");
const string gBSampBase::hw_fil_order("order");
const string gBSampBase::hw_fil_low("f_low");
const string gBSampBase::hw_fil_high("f_high");
const string gBSampBase::hw_fil_sense("sense");
const string gBSampBase::hw_notch("notch");
const string gBSampBase::hw_notch_center("f_center");

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

  elem = father->FirstChildElement(hw_channels_,false);
  if(elem != elem.end())
    setDeviceChannels(elem);

  elem = father->FirstChildElement(hw_blocksize_,false);
  if(elem != elem.end())
    setBlocks(elem);

  ticpp::Iterator<ticpp::Element> filter(father->FirstChildElement(hw_fil, false));
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

  if(!elem.Get()->HasAttribute(hw_fil_type))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil+"> given, filter type ("+hw_fil_type+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_notch))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil+"> given, filter order ("+hw_notch+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_fil_low))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil+"> given, lower cutoff frequency ("+hw_fil_low+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_fil_high))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil+"> given, upper cutoff frequency ("+hw_fil_high+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_fil_sense))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil+"> given, parameter ("+hw_fil_sense+") not given!";
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

  tia::Constants cst;
  type = cst.getSignalFlag(elem.Get()->GetAttribute(hw_fil_type));
  try
  {
    notch = equalsOnOrOff(elem.Get()->GetAttribute(hw_notch));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil+"> given, but notch is not 'on' or 'off'";
    throw(ticpp::Exception(ex_str));
  }
  if(type == SIG_EEG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_low));
      if((f_low != .5) || (f_low != 2))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '0.5' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_high));
      if((f_high != 30) || (f_high != 100))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '30' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_sense));
      if((f_low != .05) || (f_low != .1))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '0.05' or '.1'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }

  if(type == SIG_EOG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_low));
      if((f_low != .5) || (f_low != 2))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '0.5' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_high));
      if((f_high != 30) || (f_high != 100))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '30' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_sense));
      if((f_low != .1) || (f_low != 1))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '0.1' or '1'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }

  if(type == SIG_EMG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_low));
      if((f_low != .01) || (f_low != 2))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '0.01' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_high));
      if((f_high != 60) || (f_high != 100))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '60' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_sense));
      if((f_low != 1) || (f_low != 5))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '1' or '5'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }

  if(type == SIG_ECG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_low));
      if((f_low != .01) || (f_low != 2))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '0.01' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_high));
      if((f_high != 60) || (f_high != 100))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '60' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_sense));
      if((f_low != 2) || (f_low != 5))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '2' or '5'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }
}

//-----------------------------------------------------------------------------

}

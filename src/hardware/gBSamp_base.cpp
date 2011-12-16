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
#include <boost/format.hpp>
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

const string gBSampBase::hw_jumper_("jumper");

const string gBSampBase::hw_jumper_hp_("highpass");
const string gBSampBase::hw_jumper_lp_("lowpass");
const string gBSampBase::hw_jumper_sense_("sense");
const string gBSampBase::hw_jumper_notch_("notch");

const string gBSampBase::hw_daq_mode_("daq_mode");
const string gBSampBase::hw_device_id_("device_label");

static const float GBSAMP_EEG_HP_FILTER[2] = { 0.5,    2.0};
static const float GBSAMP_EEG_LP_FILTER[2] = {30.0,  100.0};
static const float GBSAMP_EEG_SENS[2]      = { 0.05,   0.1};  // scaling *10  * 20

static const float GBSAMP_EOG_HP_FILTER[2] = { 0.5,    2.0};
static const float GBSAMP_EOG_LP_FILTER[2] = {30.0,  100.0};
static const float GBSAMP_EOG_SENS[2]      = { 0.1,    1.0};  // scaling *20  * 200

static const float GBSAMP_EMG_HP_FILTER[2] = { 0.01,   2.0};
static const float GBSAMP_EMG_LP_FILTER[2] = {60.0,  100.0};
static const float GBSAMP_EMG_SENS[2]      = { 1.0,    5.0};  // scaling *200  * 1000

static const float GBSAMP_ECG_HP_FILTER[2] = { 0.01,   2.0};
static const float GBSAMP_ECG_LP_FILTER[2] = {60.0,  100.0};
static const float GBSAMP_ECG_SENS[2]      = { 2.0,    5.0};  // scaling *400  * 1000

static const float GBSAMP_MU_VOLT_SCALING_FACTOR = 200.0;

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
        throw(std::invalid_argument(ex_str));
      }
      setChannelSettings(cs);
  }

  cout << endl;
  for(unsigned int n = 0; n < channel_info_.size(); n++ )
  {
    cout << scaling_factors_[n] << ", ";
  }
  cout << endl;
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

  elem = father->FirstChildElement(hw_blocksize_,true);
  setBlocks(elem);

  elem = father->FirstChildElement(hw_daq_mode_,false);
  if(elem != elem.end())
    setAcquisitionMode(elem);

  cout << " * g.BSamp -- device filters per type set to:" << endl;
  elem = (father->FirstChildElement(hw_jumper_,true));
  setDeviceJumperSettings(elem);
  elem++;
  while(elem != elem.end() )
  {
    if(elem->Value() == hw_jumper_)
      setDeviceJumperSettings(elem);
    elem++;
  }
  cout << endl;

  setGlobalScalingValues();
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



  elem = father->FirstChildElement(hw_jumper_,false);
  if (elem != elem.end())
    setChannelJumperSettings(elem);
  else
    setGlobalScalingValues();
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

void gBSampBase::setDeviceJumperSettings(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "gBSamp: setDeviceJumperSettings" << endl;
  #endif

  unsigned int type = 0;
  bool notch = 0;
  float highpass = 0;
  float lowpass = 0;
  float sense = 0;

  checkJumperAttributes(elem);
  getJumperParams(elem, type, notch, highpass, lowpass, sense);

  global_scaling_factors_[type] = sense * GBSAMP_MU_VOLT_SCALING_FACTOR;

  tia::Constants cst;
  cout << "    ...  type: " << cst.getSignalName(type) << ", HP: " << highpass << ", LP: " << lowpass << ", sense: " << sense << ", notch: " << notch << endl;
}

//---------------------------------------------------------------------------------------

void gBSampBase::setChannelJumperSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "gBSamp: setChannelJumperSettings" << endl;
  #endif

    ticpp::Iterator<ticpp::Element> elem;
    elem = father->FirstChildElement(hw_chset_ch_,false);

    scaling_factors_.resize( channel_info_.size() );

    cout << " * g.BSamp -- channels specific jumpers set to:" << endl;

    for(  ; elem != elem.end(); elem++)
      if(elem->Value() == hw_chset_ch_)
      {
        if(!elem.Get()->HasAttribute(hw_ch_nr_))
        {
          string ex_str(type_ + " -- ");
          ex_str += "Tag <"+hw_jumper_+"> given, but channel number ("+hw_ch_nr_+") not given!";
          throw(std::invalid_argument(ex_str));


        }
        checkJumperAttributes(elem);

        uint16_t ch = 0;
        try{
          ch = lexical_cast<uint16_t>( elem.Get()->GetAttribute(hw_ch_nr_) );
        }
        catch(bad_lexical_cast &)
        {
          string ex_str(type_ + " -- ");
          ex_str += "Tag <"+ hw_jumper_ + "> : Channel is not a number!";
          throw(std::invalid_argument(ex_str));
        }
        if( channel_info_.find(ch) ==  channel_info_.end() )
        {
          string ex_str(type_ + " -- ");
          ex_str += "Tag <"+ hw_jumper_ + "> - Channel "+ lexical_cast<std::string>(ch) +" not set for recording!";
          throw(std::invalid_argument(ex_str));
        }

        unsigned int type = 0;
        bool notch = 0;
        float highpass = 0;
        float lowpass = 0;
        float sense = 0;

        getJumperParams(elem, type, notch, highpass, lowpass, sense);
        uint16_t ch_pos = 0;
        std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::iterator it;
        for(it = channel_info_.begin(); it !=channel_info_.find(ch); it++)
          ch_pos++;

        if(type != channel_types_[ch_pos])
          throw(std::invalid_argument(
              "gBSamp: Error -- Acquired signal type must match the type provided within the jumper settings" ));

        scaling_factors_.at(ch_pos) = sense * GBSAMP_MU_VOLT_SCALING_FACTOR;

        tia::Constants cst;
        cout << "  ... channel: " << "    ...  type: " << cst.getSignalName(type) << ", HP: " << highpass;
        cout << ", LP: " << lowpass << ", sense: " << sense << ", notch: " << notch << endl;
      }
      else
        throw(std::invalid_argument("gBSamp::setChannelJumperSettings -- Tag not equal to \""+hw_chset_ch_+"\"!"));

}

//---------------------------------------------------------------------------------------

void gBSampBase::setGlobalScalingValues()
{
  scaling_factors_.resize(channel_types_.size() );
  for(unsigned int n = 0; n < channel_types_.size(); n++ )
  {
    if( global_scaling_factors_.find( channel_types_.at(n) ) == global_scaling_factors_.end() )
      throw(std::invalid_argument(
          "gBSamp: Error -- Acquired signal type must match the type provided within the jumper settings" ));
    scaling_factors_[n] = global_scaling_factors_[ channel_types_[n] ];
  }
}

//---------------------------------------------------------------------------------------

void gBSampBase::checkJumperAttributes(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: checkJumperAttributes" << endl;
  #endif

  if(!elem.Get()->HasAttribute(hw_ch_type_))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_jumper_+"> given, ("+hw_ch_type_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_jumper_notch_))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_jumper_+"> given, "+hw_jumper_notch_+" not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_jumper_hp_))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_jumper_+"> given, "+hw_jumper_hp_+" frequency ("+hw_jumper_hp_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_jumper_lp_))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_jumper_+"> given, "+hw_jumper_lp_+" frequency ("+hw_jumper_lp_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_jumper_sense_))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_jumper_+"> given, parameter ("+hw_jumper_sense_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
}

//---------------------------------------------------------------------------------------

void gBSampBase::getJumperParams(ticpp::Iterator<ticpp::Element>const &elem,
  unsigned int &type, bool &notch, float &highpass, float &lowpass, float &sense)
{
  #ifdef DEBUG
    cout << "gBSamp: getJumperParams" << endl;
  #endif

  // EEG 0.1 mV.   -->.  * 20  (Saettigung: 200 muV Ampl --> 400muV SS)
  //     0.05 mV...-->   * 10  (Saettigung: 100 muV Ampl)
  // EMG 1           ;  5
  // EOG 0.1         ;  2
  // ECG 2           ;  5

  checkJumperAttributes(elem);

  tia::Constants cst;
  type = cst.getSignalFlag(elem.Get()->GetAttribute(hw_ch_type_));

  try
  {
    notch = equalsOnOrOff(elem.Get()->GetAttribute(hw_jumper_notch_));
  }
  catch(std::invalid_argument& e)
  {
    throw(std::invalid_argument( "gBSamp: Error -- Tag <"+hw_jumper_+"> - " +hw_jumper_notch_+" -- " + e.what() ));
  }

  try
  {
    highpass  = lexical_cast<float>(boost::format("%d") % elem.Get()->GetAttribute(hw_jumper_hp_));
    lowpass   = lexical_cast<float>(boost::format("%d") % elem.Get()->GetAttribute(hw_jumper_lp_));
    sense     = lexical_cast<float>(boost::format("%d") % elem.Get()->GetAttribute(hw_jumper_sense_));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str = "gBSamp: Error -- Tag <"+ hw_jumper_ + "> : Unable to convert "
        +hw_jumper_hp_+", "+hw_jumper_lp_+", or "+hw_jumper_sense_+" into a number!";
    throw(std::invalid_argument(ex_str));
  }

  if(type == SIG_EEG)
    checkEEGJumperValues(highpass, lowpass, sense);
  else if(type == SIG_EOG)
    checkEOGJumperValues(highpass, lowpass, sense);
  else if(type == SIG_EMG)
    checkEMGJumperValues(highpass, lowpass, sense);
  else if(type == SIG_ECG)
    checkECGJumperValues(highpass, lowpass, sense);
  else
    throw(std::invalid_argument( "gBSamp: Error -- Tag <"+ hw_jumper_ + ">" +hw_ch_type_+ "not recognized!"));

}

//-----------------------------------------------------------------------------

void gBSampBase::checkEEGJumperValues(float highpass, float lowpass, float sense)
{
  if( (highpass != GBSAMP_EEG_HP_FILTER[0]) && (highpass != GBSAMP_EEG_HP_FILTER[1]) )
    throwXMLErrorWrongValue(hw_jumper_, hw_jumper_hp_, "eeg", highpass, GBSAMP_EEG_HP_FILTER[0], GBSAMP_EEG_HP_FILTER[1]);

  if(lowpass != GBSAMP_EEG_LP_FILTER[0] && lowpass != GBSAMP_EEG_LP_FILTER[1])
    throwXMLErrorWrongValue(hw_jumper_, hw_jumper_lp_, "eeg",lowpass, GBSAMP_EEG_LP_FILTER[0], GBSAMP_EEG_LP_FILTER[1]);

  if(sense != GBSAMP_EEG_SENS[0] && sense != GBSAMP_EEG_SENS[1])
    throwXMLErrorWrongValue(hw_jumper_, hw_jumper_sense_, "eeg",sense, GBSAMP_EEG_SENS[0], GBSAMP_EEG_SENS[1]);
}

//-----------------------------------------------------------------------------

void gBSampBase::checkEOGJumperValues(float highpass, float lowpass, float sense)
{
  if(highpass != GBSAMP_EOG_HP_FILTER[0] && highpass != GBSAMP_EOG_HP_FILTER[1])
    throwXMLErrorWrongValue(hw_jumper_, hw_jumper_hp_, "eog",highpass, GBSAMP_EOG_HP_FILTER[0], GBSAMP_EOG_HP_FILTER[1]);

  if(lowpass != GBSAMP_EOG_LP_FILTER[0] && lowpass != GBSAMP_EOG_LP_FILTER[1])
    throwXMLErrorWrongValue(hw_jumper_, hw_jumper_lp_, "eog",lowpass, GBSAMP_EOG_LP_FILTER[0], GBSAMP_EOG_LP_FILTER[1]);

  if(sense != GBSAMP_EOG_SENS[0] && sense != GBSAMP_EOG_SENS[1])
    throwXMLErrorWrongValue(hw_jumper_, hw_jumper_sense_, "eog",sense, GBSAMP_EOG_SENS[0], GBSAMP_EOG_SENS[1]);
}

//-----------------------------------------------------------------------------

void gBSampBase::checkEMGJumperValues(float highpass, float lowpass, float sense)
{
  if(highpass != GBSAMP_EMG_HP_FILTER[0] && highpass != GBSAMP_EMG_HP_FILTER[1])
    throwXMLErrorWrongValue(hw_jumper_, hw_jumper_hp_, "emg",highpass, GBSAMP_EMG_HP_FILTER[0], GBSAMP_EMG_HP_FILTER[1]);

  if(lowpass != GBSAMP_EMG_LP_FILTER[0] && lowpass != GBSAMP_EMG_LP_FILTER[1])
    throwXMLErrorWrongValue(hw_jumper_, hw_jumper_lp_, "emg",lowpass, GBSAMP_EMG_LP_FILTER[0], GBSAMP_EMG_LP_FILTER[1]);

  if(sense != GBSAMP_EMG_SENS[0] && sense != GBSAMP_EMG_SENS[1])
    throwXMLErrorWrongValue(hw_jumper_, hw_jumper_sense_, "emg",sense, GBSAMP_EMG_SENS[0], GBSAMP_EMG_SENS[1]);
}

//-----------------------------------------------------------------------------

void gBSampBase::checkECGJumperValues(float highpass, float lowpass, float sense)
{
  if(highpass != GBSAMP_ECG_HP_FILTER[0] && highpass != GBSAMP_ECG_HP_FILTER[1])
    throwXMLErrorWrongValue(hw_jumper_, hw_jumper_hp_, "ecg",highpass, GBSAMP_ECG_HP_FILTER[0], GBSAMP_ECG_HP_FILTER[1]);

  if(lowpass != GBSAMP_ECG_LP_FILTER[0] && lowpass != GBSAMP_ECG_LP_FILTER[1])
    throwXMLErrorWrongValue(hw_jumper_, hw_jumper_lp_, "ecg",lowpass, GBSAMP_ECG_LP_FILTER[0], GBSAMP_ECG_LP_FILTER[1]);

  if(sense != GBSAMP_ECG_SENS[0] && sense != GBSAMP_ECG_SENS[1])
    throwXMLErrorWrongValue(hw_jumper_, hw_jumper_sense_, "ecg",sense, GBSAMP_ECG_SENS[0], GBSAMP_ECG_SENS[1]);
}

//-----------------------------------------------------------------------------


void gBSampBase::throwXMLErrorWrongValue(const std::string& tag_name, const std::string& attr_name,
                                         std::string type, float given, float cor1, float cor2)
{
  string ex_str;
  ex_str = "gBSamp: Error -- ";
  ex_str += "Tag <"+tag_name+"> - "+type+" - attribute: "+attr_name + " ";
  ex_str += "given value: " + boost::lexical_cast<std::string>(boost::format("%d") % given) +"; ";
  ex_str += "possible values are: "+ boost::lexical_cast<std::string>(boost::format("%d") % cor1);
  ex_str += " and " + boost::lexical_cast<std::string>(boost::format("%d") % cor2);

  throw(std::invalid_argument(ex_str));
}

//-----------------------------------------------------------------------------



// Very bad hack!

//if(type == SIG_EEG)
//{
//  try
//  {
//    f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_jumper_hp_));
//    if((f_low != .5) || (f_low != 2))
//    {
//      string ex_str;
//      ex_str = "gBSamp: Error -- ";
//      ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not '0.5' or '2'";
//      throw(ticpp::Exception(ex_str));
//    }
//  }
//  catch(bad_lexical_cast &)
//  {
//    string ex_str;
//    ex_str = "gBSamp: Error -- ";
//    ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not given";
//    throw(ticpp::Exception(ex_str));
//  }
//  try
//  {
//    f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_jumper_lp_));
//    if((f_high != 30) || (f_high != 100))
//    {
//      string ex_str;
//      ex_str = "gBSamp: Error -- ";
//      ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not '30' or '100'";
//      throw(ticpp::Exception(ex_str));
//    }
//  }
//  catch(bad_lexical_cast &)
//  {
//    string ex_str;
//    ex_str = "gBSamp: Error -- ";
//    ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not given";
//    throw(ticpp::Exception(ex_str));
//  }
//  try
//  {
//    sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_jumper_sense_));
//    if((f_low != .05) || (f_low != .1))
//    {
//      string ex_str;
//      ex_str = "gBSamp: Error -- ";
//      ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not '0.05' or '.1'";
//      throw(ticpp::Exception(ex_str));
//    }
//  }
//  catch(bad_lexical_cast &)
//  {
//    string ex_str;
//    ex_str = "gBSamp: Error -- ";
//    ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not given";
//    throw(ticpp::Exception(ex_str));
//  }
//}

//if(type == SIG_EOG)
//{
//  try
//  {
//    f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_jumper_hp_));
//    if((f_low != .5) || (f_low != 2))
//    {
//      string ex_str;
//      ex_str = "gBSamp: Error -- ";
//      ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not '0.5' or '2'";
//      throw(ticpp::Exception(ex_str));
//    }
//  }
//  catch(bad_lexical_cast &)
//  {
//    string ex_str;
//    ex_str = "gBSamp: Error -- ";
//    ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not given";
//    throw(ticpp::Exception(ex_str));
//  }
//  try
//  {
//    f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_jumper_lp_));
//    if((f_high != 30) || (f_high != 100))
//    {
//      string ex_str;
//      ex_str = "gBSamp: Error -- ";
//      ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not '30' or '100'";
//      throw(ticpp::Exception(ex_str));
//    }
//  }
//  catch(bad_lexical_cast &)
//  {
//    string ex_str;
//    ex_str = "gBSamp: Error -- ";
//    ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not given";
//    throw(ticpp::Exception(ex_str));
//  }
//  try
//  {
//    sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_jumper_sense_));
//    if((f_low != .1) || (f_low != 1))
//    {
//      string ex_str;
//      ex_str = "gBSamp: Error -- ";
//      ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not '0.1' or '1'";
//      throw(ticpp::Exception(ex_str));
//    }
//  }
//  catch(bad_lexical_cast &)
//  {
//    string ex_str;
//    ex_str = "gBSamp: Error -- ";
//    ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not given";
//    throw(ticpp::Exception(ex_str));
//  }
//}

//if(type == SIG_EMG)
//{
//  try
//  {
//    f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_jumper_hp_));
//    if((f_low != .01) || (f_low != 2))
//    {
//      string ex_str;
//      ex_str = "gBSamp: Error -- ";
//      ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not '0.01' or '2'";
//      throw(ticpp::Exception(ex_str));
//    }
//  }
//  catch(bad_lexical_cast &)
//  {
//    string ex_str;
//    ex_str = "gBSamp: Error -- ";
//    ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not given";
//    throw(ticpp::Exception(ex_str));
//  }
//  try
//  {
//    f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_jumper_lp_));
//    if((f_high != 60) || (f_high != 100))
//    {
//      string ex_str;
//      ex_str = "gBSamp: Error -- ";
//      ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not '60' or '100'";
//      throw(ticpp::Exception(ex_str));
//    }
//  }
//  catch(bad_lexical_cast &)
//  {
//    string ex_str;
//    ex_str = "gBSamp: Error -- ";
//    ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not given";
//    throw(ticpp::Exception(ex_str));
//  }
//  try
//  {
//    sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_jumper_sense_));
//    if((f_low != 1) || (f_low != 5))
//    {
//      string ex_str;
//      ex_str = "gBSamp: Error -- ";
//      ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not '1' or '5'";
//      throw(ticpp::Exception(ex_str));
//    }
//  }
//  catch(bad_lexical_cast &)
//  {
//    string ex_str;
//    ex_str = "gBSamp: Error -- ";
//    ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not given";
//    throw(ticpp::Exception(ex_str));
//  }
//}

//if(type == SIG_ECG)
//{
//  try
//  {
//    f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_jumper_hp_));
//    if((f_low != .01) || (f_low != 2))
//    {
//      string ex_str;
//      ex_str = "gBSamp: Error -- ";
//      ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not '0.01' or '2'";
//      throw(ticpp::Exception(ex_str));
//    }
//  }
//  catch(bad_lexical_cast &)
//  {
//    string ex_str;
//    ex_str = "gBSamp: Error -- ";
//    ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not given";
//    throw(ticpp::Exception(ex_str));
//  }
//  try
//  {
//    f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_jumper_lp_));
//    if((f_high != 60) || (f_high != 100))
//    {
//      string ex_str;
//      ex_str = "gBSamp: Error -- ";
//      ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not '60' or '100'";
//      throw(ticpp::Exception(ex_str));
//    }
//  }
//  catch(bad_lexical_cast &)
//  {
//    string ex_str;
//    ex_str = "gBSamp: Error -- ";
//    ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not given";
//    throw(ticpp::Exception(ex_str));
//  }
//  try
//  {
//    sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_jumper_sense_));
//    if((f_low != 2) || (f_low != 5))
//    {
//      string ex_str;
//      ex_str = "gBSamp: Error -- ";
//      ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not '2' or '5'";
//      throw(ticpp::Exception(ex_str));
//    }
//  }
//  catch(bad_lexical_cast &)
//  {
//    string ex_str;
//    ex_str = "gBSamp: Error -- ";
//    ex_str += "Tag <"+hw_jumper_+"> given, but lower filter is not given";
//    throw(ticpp::Exception(ex_str));
//  }
//}

}

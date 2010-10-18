#include "definitions/constants.h"

#include <boost/algorithm/string.hpp>

#include "definitions/defines.h"
#include "ticpp/ticpp.h"

namespace tobiss
{

using namespace std;
using boost::uint32_t;
using boost::algorithm::to_lower_copy;

//-----------------------------------------------------------------------------

const string Constants::tobi("tobi-config");

const string Constants::subject("subject");
const string Constants::s_id("id");
const string Constants::s_first_name("first_name");
const string Constants::s_surname("surname");
const string Constants::s_sex("sex");
const string Constants::s_birthday("birthday");

const string Constants::ss("server_settings");
const string Constants::ss_ctl_port("ctl_port");
const string Constants::ss_udp_bc_addr("udp_bc_addr");
const string Constants::ss_udp_port("udp_port");

const string Constants::ss_store_data("store-data");
const string Constants::ss_filename("filename");
const string Constants::ss_filetype("filetype");
const string Constants::ss_filepath("filepath");
const string Constants::ss_filepath_default("rec");
const string Constants::ss_file_overwrite("overwrite");
const string Constants::ss_file_overwrite_default("ask");

const string Constants::file_reader("file-reader");
  // filepath, name and type from store_data
const string Constants::fr_speedup("speedup");
const string Constants::fr_stop("stop_at_end");


const string Constants::hardware("hardware");
const string Constants::hardware_name("name");
const string Constants::hardware_version("version");
const string Constants::hardware_serial("serial");

const string Constants::hw_mode("mode");
const string Constants::hw_ds("device_settings");
const string Constants::hw_fs("sampling_rate");

const string Constants::hw_channels("measurement_channels");
const string Constants::hw_ch_nr("nr");
const string Constants::hw_ch_names("names");
const string Constants::hw_ch_type("type");

const string Constants::hw_buffer("blocksize");

const string Constants::hw_fil("filter");
const string Constants::hw_fil_type("type");
const string Constants::hw_fil_order("order");
const string Constants::hw_fil_low("f_low");
const string Constants::hw_fil_high("f_high");
const string Constants::hw_fil_sense("sense");

const string Constants::hw_notch("notch");
const string Constants::hw_notch_center("f_center");

//USBamp specific start
const string Constants::hw_opmode("operation_mode");
const string Constants::hw_sc("shortcut");
const string Constants::hw_trigger_line("trigger_line");
const string Constants::hw_usbampmaster("usbamp_master");
const string Constants::hw_comgnd("common_ground");
const string Constants::hw_gnd("gnd");
const string Constants::hw_gnd_block("block");
const string Constants::hw_gnd_value("value");

const string Constants::hw_comref("common_reference");
const string Constants::hw_cr("cr");
const string Constants::hw_cr_block("block");
const string Constants::hw_cr_value("value");
//USBamp specific end

const string Constants::hw_cs("channel_settings");
const string Constants::hw_sel("selection");
const string Constants::hw_cs_ch("ch");
const string Constants::hw_cs_nr("nr");
const string Constants::hw_cs_name("name");
//naming of filter and others equal to global_settings

//USBamp specific start
const string Constants::hw_bip("bipolar");
const string Constants::hw_bip_with("with");
const string Constants::hw_drl("driven_right_leg");
const string Constants::hw_drl_value("value");

//-----------------------------------------------------------------------------

Constants::Constants()
{
  supported_hardware.insert(pair <string,int>("sinegenerator", SINEGEN));
  supported_hardware.insert(pair <string,int>("sinegen", SINEGEN));

  supported_hardware.insert(pair <string,int>("bsamp", BSAMP));
  supported_hardware.insert(pair <string,int>("g.bsamp", BSAMP));

  supported_hardware.insert(pair <string,int>("usbamp", USBAMP));
  supported_hardware.insert(pair <string,int>("g.usbamp", USBAMP));

  supported_hardware.insert(pair <string,int>("mobilab", MOBILAB));
  supported_hardware.insert(pair <string,int>("mobilab+", MOBILAB));
  supported_hardware.insert(pair <string,int>("g.mobilab", MOBILAB));
  supported_hardware.insert(pair <string,int>("g.mobilab+", MOBILAB));

  supported_hardware.insert(pair <string,int>("jstick", JOYSTICK));
  supported_hardware.insert(pair <string,int>("joystick", JOYSTICK));
  supported_hardware.insert(pair <string,int>("joycable", JOYSTICK));

  signaltypes.insert(pair <string,uint32_t>("eeg", SIG_EEG));
  signaltypes.insert(pair <string,uint32_t>("emg", SIG_EMG));
  signaltypes.insert(pair <string,uint32_t>("eog", SIG_EOG));
  signaltypes.insert(pair <string,uint32_t>("ecg", SIG_ECG));
  signaltypes.insert(pair <string,uint32_t>("hr",  SIG_HR));
  signaltypes.insert(pair <string,uint32_t>("bp", SIG_BP));
  signaltypes.insert(pair <string,uint32_t>("button", SIG_BUTTON));
  signaltypes.insert(pair <string,uint32_t>("joystick", SIG_JOYSTICK));
  signaltypes.insert(pair <string,uint32_t>("sensor", SIG_SENSOR));
  signaltypes.insert(pair <string,uint32_t>("nirs",  SIG_NIRS));
  signaltypes.insert(pair <string,uint32_t>("fmri",  SIG_FMRI));

  signaltypes.insert(pair <string,uint32_t>("user_1", SIG_USER_1));
  signaltypes.insert(pair <string,uint32_t>("user_2", SIG_USER_2));
  signaltypes.insert(pair <string,uint32_t>("user_3", SIG_USER_3));
  signaltypes.insert(pair <string,uint32_t>("user_4", SIG_USER_4));
  signaltypes.insert(pair <string,uint32_t>("user1", SIG_USER_1));
  signaltypes.insert(pair <string,uint32_t>("user2", SIG_USER_2));
  signaltypes.insert(pair <string,uint32_t>("user3", SIG_USER_3));
  signaltypes.insert(pair <string,uint32_t>("user4", SIG_USER_4));
  signaltypes.insert(pair <string,uint32_t>("undefined", SIG_UNDEFINED));
  signaltypes.insert(pair <string,uint32_t>("undef", SIG_UNDEFINED));
  signaltypes.insert(pair <string,uint32_t>("", SIG_UNDEFINED));
  signaltypes.insert(pair <string,uint32_t>("event", SIG_EVENT));

  usbamp_filterTypes.insert(pair <string,int>("chebyshev", 1));
  usbamp_filterTypes.insert(pair <string,int>("cheby", 1));
  usbamp_filterTypes.insert(pair <string,int>("butterworth", 2));
  usbamp_filterTypes.insert(pair <string,int>("butter", 2));

  usbamp_opModes.insert(pair <string,string>("normal", "M_NORMAL"));
  usbamp_opModes.insert(pair <string,string>("calibrate", "M_CALIBRATE"));
  usbamp_opModes.insert(pair <string,string>("impedance", "M_IMPEDANCE"));

  usbamp_blockNames.insert(pair <string,int>("a", 0));
  usbamp_blockNames.insert(pair <string,int>("b", 1));
  usbamp_blockNames.insert(pair <string,int>("c", 2));
  usbamp_blockNames.insert(pair <string,int>("d", 3));
}

//-----------------------------------------------------------------------------

int Constants::isSupportedHardware(const string& s)
{
  map<string, unsigned int>::iterator it;
  it = supported_hardware.find(to_lower_copy(s));
  if(it == supported_hardware.end())
  {
    string e = "Hardware \"" + s + "\" not supported -- please also check spelling!";
    throw ticpp::Exception(e);
  }
  return(it->second);
}

//-----------------------------------------------------------------------------

bool Constants::isSineGen(const string& s)
{
  unsigned int hw_nr = 0;
  if( !(hw_nr = isSupportedHardware(s)) )
    return(0);
  if(hw_nr == SINEGEN)
    return(1);

  return(0);
}

//-----------------------------------------------------------------------------

bool Constants::isUSBamp(const string& s)
{
  unsigned int hw_nr = 0;
  if( !(hw_nr = isSupportedHardware(s)) )
    return(0);
  if(hw_nr == USBAMP)
    return(1);

  return(0);
}

//-----------------------------------------------------------------------------

bool Constants::isMobilab(const string& s)
{
  unsigned int hw_nr = 0;
  if( !(hw_nr = isSupportedHardware(s)) )
    return(0);
  if(hw_nr == MOBILAB)
    return(1);

  return(0);
}

//-----------------------------------------------------------------------------

bool Constants::isJoystick(const string& s)
{
  unsigned int hw_nr = 0;
  if( !(hw_nr = isSupportedHardware(s)) )
    return(0);
  if(hw_nr == JOYSTICK)
    return(1);

  return(0);
}

//-----------------------------------------------------------------------------

bool Constants::isBSamp(const string& s)
{
  unsigned int hw_nr = 0;
  if( !(hw_nr = isSupportedHardware(s)) )
    return(0);
  if(hw_nr == BSAMP)
    return(1);

  return(0);
}

//-----------------------------------------------------------------------------

bool Constants::equalsOnOrOff(const string& s)
{
  if(to_lower_copy(s) == "on"  || s == "1")
    return(true);
  if(to_lower_copy(s) == "off" || s == "0")
    return(false);
  else
  {
    string e = s + " -- Value equals neiter \"on, off, 0 or 1\"!";
    throw ticpp::Exception(e);
  }
}

//-----------------------------------------------------------------------------

bool Constants::equalsYesOrNo(const string& s)
{
  if(to_lower_copy(s) == "yes"  || s == "1")
    return(true);
  if(to_lower_copy(s) == "no" || s == "0")
    return(false);
  else
  {
    string e = s + " -- Value equals neiter \"yes, no, 0 or 1\"!";
    throw ticpp::Exception(e);
  }
}

//-----------------------------------------------------------------------------

bool Constants::equalsMaster(const string& s)
{
  return(to_lower_copy(s) == "master");
}

//-----------------------------------------------------------------------------

bool Constants::equalsSlave(const string& s)
{
  return(to_lower_copy(s) == "slave");
}

//-----------------------------------------------------------------------------

bool Constants::equalsAperiodic(const string& s)
{
  return(to_lower_copy(s) == "aperiodic");
}

//-----------------------------------------------------------------------------

int Constants::getUSBampFilterType(const string& s)
{
  map<string, unsigned int>::iterator it;
  it = usbamp_filterTypes.find(to_lower_copy(s));
  if(it == usbamp_filterTypes.end())
  {
    string e = "USBamp filter type \"" + s + "\" not found -- please also check spelling!";
    throw ticpp::Exception(e);
  }
  return(it->second);
}

//-----------------------------------------------------------------------------

string Constants::getUSBampOpMode(const string& s)
{
  map<string, string>::iterator it;
  it = usbamp_opModes.find(to_lower_copy(s));
  if(it == usbamp_opModes.end())
  {
    string e = "USBamp operation mode \"" + s + "\" not found -- please also check spelling!";
    throw ticpp::Exception(e);
  }
  return(it->second);
}

//-----------------------------------------------------------------------------

int Constants::getUSBampBlockNr(const string& s)
{
  map<string, unsigned int>::iterator it;
  it = usbamp_blockNames.find(to_lower_copy(s));
  if(it == usbamp_blockNames.end())
  {
    string e = "USBamp channel block \"" + s + "\" wrong -- must be \"a, b, c or d\"!";
    throw ticpp::Exception(e);
  }
  return(it->second);
}

//-----------------------------------------------------------------------------

uint32_t Constants::getSignalFlag(const string& s)
{
  map<string, uint32_t>::iterator it;
  it = signaltypes.find(to_lower_copy(s));
 if(it == signaltypes.end())
 {
   string e = "Signal type not found!";
   throw ticpp::Exception(e);
 }
  return(it->second);
}

//-----------------------------------------------------------------------------

string Constants::getSignalName(const uint32_t& flag)
{
  for(map<string, uint32_t>::iterator it(signaltypes.begin()); it != signaltypes.end(); it++)
    if(it->second == flag)
      return(it->first);

  string e = "Signal type not found!";
  throw ticpp::Exception(e);
}

//-----------------------------------------------------------------------------

} // Namespace tobiss

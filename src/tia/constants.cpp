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
* @file constants.cpp
**/

#include "tia/constants.h"

#include <boost/algorithm/string.hpp>

#include "tia/defines.h"
#include "ticpp/ticpp.h"

namespace tobiss
{
using std::string;
using std::map;
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
const string Constants::ss_tid_port("tid_port");

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

//Mouse specific start
//const string Constants::hw_vid("vendorid");
//const string Constants::hw_pid("productid");
//const string Constants::usb_port("usb_port");
//Mouse specific end

//-----------------------------------------------------------------------------

Constants::Constants()
{
  signaltypes.insert(pair <string,uint32_t>("mouse", SIG_MOUSE));
  signaltypes.insert(pair <string,uint32_t>("mouse-button", SIG_MBUTTON));

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
  signaltypes.insert(pair <string,uint32_t>("event", SIG_EVENT));

}

//-----------------------------------------------------------------------------

uint32_t Constants::getSignalFlag(const std::string& s)
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

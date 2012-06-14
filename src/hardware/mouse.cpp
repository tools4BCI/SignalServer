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

#include "hardware/mouse.h"
#include <boost/current_function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

namespace tobiss
{
using boost::lexical_cast;
using boost::bad_lexical_cast;

using std::vector;
using std::string;
using std::pair;
using std::cout;
using std::endl;
using std::set;

set<boost::uint16_t> MouseBase::used_ids_;

const string MouseBase::str_detach_from_os_("detach_from_os");
const string MouseBase::str_hw_vid_("vendorid");
const string MouseBase::str_hw_pid_("productid");
const string MouseBase::str_usb_port_("usb_port");

//-----------------------------------------------------------------------------
MouseBase::MouseBase(ticpp::Iterator<ticpp::Element> hw)
  : HWThread(), dirty_(1), vid_(0), pid_(0), usb_port_(0), async_acqu_thread_(0),
    id_(0)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  checkMandatoryHardwareTags(hw);
  if(mode_ != APERIODIC)
    throw(std::invalid_argument("Mouse has to be started as aperiodic device!"));
  initMouse();

  id_ = 128;
  while(used_ids_.find(id_) != used_ids_.end() )
    id_++;
  used_ids_.insert(id_);

  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(hw_devset_, true));
  setDeviceSettings(ds);
  data_.init(1, channel_types_.size() , channel_types_);

  vector<boost::uint32_t> v;
  empty_block_.init(0,0, v);

  //	cout << " * Mouse sucessfully initialized -- running as aperiodic: ";
  //	cout << (mode_ == APERIODIC) << ";  ";
}

//-----------------------------------------------------------------------------

MouseBase::~MouseBase()
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif
}

//---------------------------------------------------------------------------------------

void MouseBase::setDetachFromOS(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  detach_from_os_ = equalsOnOrOff(elem->GetText(true));
}

//---------------------------------------------------------------------------------------

void MouseBase::setVendorId(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  try
  {
    vid_ = giveDecimalRepresentation(elem->GetText(true));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- VendorId: value is not a decimal or hxadecimal number!");
    throw(std::invalid_argument(ex_str));
  }
  if(vid_ == 0)
  {
    string ex_str(type_ + " -- VendorId: value is 0!");
    throw(std::invalid_argument(ex_str));
  }
}
//-----------------------------------------------------------------------------

void MouseBase::setProductId(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  try
  {
    pid_ = giveDecimalRepresentation( elem->GetText(true) );
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- ProductId: value is not a decimal or hxadecimal number!");
    throw(std::invalid_argument(ex_str));
  }
  if(pid_ == 0)
  {
    string ex_str(type_ + " -- ProductId: value is 0!");
    throw(std::invalid_argument(ex_str));
  }
}

//-----------------------------------------------------------------------------

void MouseBase::setUsbPort(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  try
  {
    usb_port_ = giveDecimalRepresentation( elem->GetText(true) );
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- Usb port: value is not a decimal or hxadecimal number!");
    throw(std::invalid_argument(ex_str));
  }
  if(usb_port_ == 0)
  {
    string ex_str(type_ + " -- Usb port: value is 0!");
    throw(std::invalid_argument(ex_str));
  }
}

//-----------------------------------------------------------------------------

void MouseBase::setDeviceSettings(ticpp::Iterator<ticpp::Element>const& father)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(str_detach_from_os_,true));
  setDetachFromOS(elem);

  if(detach_from_os_)
  {
    ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(str_hw_vid_,true));
    setVendorId(elem);

    ticpp::Iterator<ticpp::Element> elem2(father->FirstChildElement(str_hw_pid_,true));
    setProductId(elem2);

    ticpp::Iterator<ticpp::Element> elem3(father->FirstChildElement(str_usb_port_,true));
    setUsbPort(elem3);
  }

  if(buttons_)
    channel_types_.push_back(SIG_BUTTON);
  for(boost::uint32_t n = 0; n < buttons_; n++)
    channel_types_.push_back(SIG_BUTTON);

  if(axes_)
    channel_types_.push_back(SIG_AXIS);
  for(boost::uint32_t n = 0; n < axes_; n++)
   channel_types_.push_back(SIG_AXIS);
  nr_ch_= channel_types_.size();

  std::string naming("Mouse Button ");
  boost::uint16_t n = 1;
  if(buttons_)
    for( ; n <= buttons_ +1; n++)
    {
      naming += boost::lexical_cast<std::string>(n);
      channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(n, pair<string, boost::uint32_t>(naming, SIG_BUTTON)));
    }
  naming = "Mouse Axis ";
  if(axes_)
    for( ; n <= axes_ + buttons_ +2; n++)
    {
      naming += boost::lexical_cast<std::string>(n - buttons_);
      channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(n, pair<string, boost::uint32_t>(naming, SIG_AXIS)));
    }


  homogenous_signal_type_ = 0;

}

//---------------------------------------------------------------------------------------

void MouseBase::setChannelSettings(ticpp::Iterator<ticpp::Element>const& )
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

}

//---------------------------------------------------------------------------------------

SampleBlock<double> MouseBase::getAsyncData()
{
  if(!running_)
    return(empty_block_);

  boost::shared_lock<boost::shared_mutex> lock(rw_);

  if(!dirty_)
    return(empty_block_);

  vector<double> v;

  #ifdef DEBUG
    if(dirty_)
    {
      cout << "Mouse Corodinates: "<<  axes_values_[0] << "," << axes_values_[1] << " -- ";
      cout << buttons_values_[0] << ",";
      cout << buttons_values_[1] << ",";
      cout << buttons_values_[2];
      cout << endl << std::flush;
    }
  #endif

  if(buttons_)
    v.push_back(id_);
  for(boost::uint8_t n = 0; n < buttons_values_.size(); n++)
    v.push_back(buttons_values_[n]);

  if(axes_)
    v.push_back(id_);
  for(boost::uint8_t n = 0; n < axes_values_.size(); n++)
    v.push_back(axes_values_[n]);

  lock.unlock();

  data_.setSamples(v);

  dirty_ = false;
  return(data_);
}

//-----------------------------------------------------------------------------

void MouseBase::run()
{
  running_ = true;
  if(mode_ == APERIODIC)
  {
    async_acqu_thread_ = new boost::thread( boost::bind(&MouseBase::acquireData, this) );
  }
}

//-----------------------------------------------------------------------------

void MouseBase::stop()
{
  running_ =false;
}

//-----------------------------------------------------------------------------

void MouseBase::initMouse()
{
  set<boost::uint16_t>::iterator it(used_ids_.begin());

  axes_    = PRE_DEFINED_NR_MOUSE_AXIS_;
  buttons_ = PRE_DEFINED_NR_MOUSE_BUTTONS_;

  axes_values_.resize(axes_,0);
  buttons_values_.resize(buttons_,0);
}

//-----------------------------------------------------------------------------


int MouseBase::giveDecimalRepresentation(std::string str)
{
  if( !str.compare(0, 2, "0x") )
  {
    int val = 0;
    if(!fromString(val, str.substr( 2, std::string::npos ),  std::hex))
      throw boost::bad_lexical_cast();

    return(val);
  }
  else
     return( lexical_cast<int>(str) );
}

//-----------------------------------------------------------------------------




} // Namespace tobiss

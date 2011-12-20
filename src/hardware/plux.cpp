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
* @file plux.cpp
**/

#include "hardware/plux.h"

#include <iostream>

namespace tobiss
{

using std::cout;
using std::endl;
using std::pair;
using std::string;

//-----------------------------------------------------------------------------

const HWThreadBuilderTemplateRegistratorWithoutIOService<Plux> Plux::FACTORY_REGISTRATOR_ ("plux", "bioplux");

//-----------------------------------------------------------------------------

Plux::Plux(ticpp::Iterator<ticpp::Element> hw)
  : HWThread(), device_(NULL)
{
  #ifdef DEBUG
    cout << "Plux: Constructor" << endl;
  #endif

  setHardware(hw);

  PLUX_TRY {

    std::map<std::string, std::string>::const_iterator it = m_.find( "mac" );

    if( it == m_.end() )
      device_ = BP::Device::Create( acquireDevice( ) );
    else
      device_ = BP::Device::Create( it->second );

    std::string dev_info;
    device_->GetDescription( dev_info );
    setType( dev_info );
  } PLUX_CATCH
}

//-----------------------------------------------------------------------------

Plux::~Plux( )
{
  #ifdef DEBUG
    cout << "Plux: Destructor" << endl;
  #endif

  if( device_ )
    delete device_;
}

//-----------------------------------------------------------------------------

void Plux::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{
  #ifdef DEBUG
    cout << "Plux: setHardware" << endl;
  #endif

  checkMandatoryHardwareTags(hw);

  // parse hardware attributes
  ticpp::Iterator<ticpp::Element> elem(hw);
  ticpp::Iterator< ticpp::Attribute > attribute;
  for(attribute = attribute.begin(elem.Get()); attribute != attribute.end();
      attribute++)
      m_.insert(pair<string, string>(attribute->Name(), attribute->Value()));

  // parse device settings
  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(hw_devset_, true));
  setDeviceSettings(ds);

  // parse channel selection
  ticpp::Iterator<ticpp::Element> cs(hw->FirstChildElement(hw_chset_, false));
  if (cs != cs.end())
  {
    for(ticpp::Iterator<ticpp::Element> it(cs); ++it != it.end(); )
      if(it->Value() == hw_chset_)
      {
      string ex_str(type_ + " -- ");
        ex_str += "Multiple channel_settings found!";
        throw(std::invalid_argument(ex_str));
      }
      setChannelSettings(cs);
  }
}

//-----------------------------------------------------------------------------

void Plux::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "Plux: setDeviceSettings" << endl;
  #endif
  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_fs_,true));
  setSamplingRate(elem);

  elem = father->FirstChildElement(hw_channels_,false);
  if(elem != elem.end())
    setDeviceChannels(elem);

  elem = father->FirstChildElement(hw_blocksize_,false);
  if(elem != elem.end())
    setBlocks(elem);
}

//---------------------------------------------------------------------------------------

void Plux::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father )
{
  #ifdef DEBUG
    cout << "Plux: setChannelSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_chset_sel_,false));
  if (elem != elem.end())
    setChannelSelection(elem);
}

//---------------------------------------------------------------------------------------

SampleBlock<double> Plux::getSyncData()
{
	throw std::invalid_argument( "Plux::getSyncData() - Plux class not yet implemented." );
}

//---------------------------------------------------------------------------------------

SampleBlock<double> Plux::getAsyncData()
{
	throw std::invalid_argument( "Plux::getAsyncData() - Plux class not yet implemented." );
}

//-----------------------------------------------------------------------------

void Plux::run()  
{
	throw std::invalid_argument( "Plux::run() - Plux class not yet implemented." );
}

//-----------------------------------------------------------------------------

void Plux::stop() 
{
	throw std::invalid_argument( "Plux::stop() - Plux class not yet implemented." );
}

//-----------------------------------------------------------------------------

void Plux::rethrowPluxException(  BP::Err &err )
{
    string message;
    switch( err.GetType() )
    {
    case BP::Err::TYP_NOTIFICATION:
      message = "BioPlux Notification: "; break;
    case BP::Err::TYP_ERROR:
      message = "BioPlux Error: "; break;
    default:
      message = "BioPlux Unknown Exception: "; break;
    }

    const char *tmp = err.GetDescription( );
    message += tmp;

    throw(std::runtime_error( message ));
}

//-----------------------------------------------------------------------------

} // Namespace tobiss

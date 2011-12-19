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

//-----------------------------------------------------------------------------

const HWThreadBuilderTemplateRegistratorWithoutIOService<Plux> Plux::FACTORY_REGISTRATOR_ ("plux", "bioplux");

//-----------------------------------------------------------------------------

Plux::Plux(ticpp::Iterator<ticpp::Element> hw)
  : HWThread()
{
  #ifdef DEBUG
    cout << "Plux: Constructor" << endl;
  #endif
  
  checkMandatoryHardwareTags(hw);

	throw std::invalid_argument( "Plux::Plux() - Plux class not yet implemented." );
}

//-----------------------------------------------------------------------------

Plux::~Plux( )
{
}

//-----------------------------------------------------------------------------

void Plux::setDeviceSettings(ticpp::Iterator<ticpp::Element>const&)
{
	throw std::invalid_argument( "Plux::setDeviceSettings() - Plux class not yet implemented." );
}

//---------------------------------------------------------------------------------------

void Plux::setChannelSettings(ticpp::Iterator<ticpp::Element>const& )
{
	throw std::invalid_argument( "Plux::setChannelSettings() - Plux class not yet implemented." );
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

} // Namespace tobiss

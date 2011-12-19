
#include "hardware/plux.h"

namespace tobiss
{

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

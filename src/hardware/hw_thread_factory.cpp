/*
    This file is part of the TOBI signal server.

    The TOBI signal server is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI signal server is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI signal server.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christoph Eibel
    Contact: christoph.eibel@tugraz.at
*/

#include "hardware/hw_thread_factory.h"
#include "hardware/hw_thread_builder.h"

namespace tobiss
{

//-----------------------------------------------------------------------------
HWThreadFactory& HWThreadFactory::instance ()
{
    static HWThreadFactory instance_obj;
    return instance_obj;
}


//-----------------------------------------------------------------------------
HWThreadFactory::~HWThreadFactory ()
{
    for (std::map<std::string, HWThreadBuilder*>::iterator it = builders_.begin();
         it != builders_.end(); ++it)
    {
        delete it->second;
    }
}


//-----------------------------------------------------------------------------
void HWThreadFactory::registerBuilder (std::string const& key, HWThreadBuilder* builder)
{
    builders_[key] = builder;
}

//-----------------------------------------------------------------------------
HWThread* HWThreadFactory::createHWThread (std::string const& key, boost::asio::io_service& io, XMLParser& parser, ticpp::Iterator<ticpp::Element> hw)
{
    if (builders_.find (key) == builders_.end())
        return 0;
    return builders_[key]->createInstance (io, parser, hw);
}



} // namespace tobiss

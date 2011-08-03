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
* @file hw_thread_factory.cpp
**/

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
HWThread* HWThreadFactory::createHWThread (std::string const& key, boost::asio::io_service& io, ticpp::Iterator<ticpp::Element> hw)
{
    if (builders_.find (key) == builders_.end())
        return 0;
    return builders_[key]->createInstance (io, hw);
}

//-----------------------------------------------------------------------------

std::vector<std::string> HWThreadFactory::getPossibleHardwareNames()
{
  std::vector<std::string> hw_names;

  for (std::map<std::string, HWThreadBuilder*>::iterator it = builders_.begin();
       it != builders_.end(); ++it)
  {
      hw_names.push_back(it->first);
  }

  return(hw_names);
}


} // namespace tobiss

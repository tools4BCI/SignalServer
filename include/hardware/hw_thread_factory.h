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
* @file hw_thread_factory.h
**/

#ifndef HW_THREAD_FACTORY_H
#define HW_THREAD_FACTORY_H

#include "hw_thread.h"

#include <boost/asio/io_service.hpp>

#include <map>
#include <string>
#include <vector>

namespace tobiss
{

class HWThreadBuilder;

//-----------------------------------------------------------------------------
/**
* @class HWThreadFactory
*
* @brief A factory for HWThreads
*
*/
class HWThreadFactory
{
public:

    static HWThreadFactory& instance ();

    /**
    * @brief deletes all prototypes
    */
    ~HWThreadFactory ();

    /**
    * @brief takes ownership of the HWThreadBuilder pointer (it will be destroyed
    *        if the factory is destroyed)
    */
    void registerBuilder (std::string const& key, HWThreadBuilder* builder);

    /**
    * the caller has to care for destruction of the created HWThread
    */
    HWThread* createHWThread (std::string const& key, boost::asio::io_service& io, ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Get the names to build the respective hardware objects.
    */
    std::vector<std::string> getPossibleHardwareNames();

private:
    std::map<std::string, HWThreadBuilder*> builders_;

    HWThreadFactory () {}
    HWThreadFactory (HWThreadFactory const& src);
    HWThreadFactory& operator= (HWThreadFactory const& src);

};

} // namespace tobiss

#endif // HW_THREAD_FACTORY_H

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

/**
* @file hw_thread_factory.h
**/

#ifndef HW_THREAD_FACTORY_H
#define HW_THREAD_FACTORY_H

#include "hw_thread.h"

#include <boost/asio/io_service.hpp>

#include <map>
#include <string>


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

private:
    std::map<std::string, HWThreadBuilder*> builders_;

    HWThreadFactory () {}
    HWThreadFactory (HWThreadFactory const& src);
    HWThreadFactory& operator= (HWThreadFactory const& src);

};

} // namespace tobiss

#endif // HW_THREAD_FACTORY_H

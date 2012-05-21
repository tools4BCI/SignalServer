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
* @file hw_thread_builder.h
**/

#ifndef HW_THREAD_BUILDER_H
#define HW_THREAD_BUILDER_H

#include "hw_thread.h"
#include "hw_thread_factory.h"

namespace tobiss
{

//-----------------------------------------------------------------------------
/**
* @class HWThreadBuilder
*
* @brief Builder base class for HWThreads... it provides a factory method
*
* instances of subclasses of this class should be registered in the HWThreadFactory;
* this factory calls the createInstance method to create concrete instances of HWThread
*/
class HWThreadBuilder
{
  public:
    virtual HWThread* createInstance (boost::asio::io_service& io, ticpp::Iterator<ticpp::Element> hw) const = 0;
    virtual ~HWThreadBuilder() {}
  protected:
    HWThreadBuilder () {}
};


//-----------------------------------------------------------------------------
/**
* @class HWThreadBuilderTemplateRegistrator
*
* @brief A template for the HWThreadBuilder which also registrates concrete builders
*        in the HWThreadFactory
*
* usage: create an instance of this class (static class member or a global variable)
*        HWThreadBuilderTemplateRegistrator<ConcreteHWThread> my_concrete_hw_thread_builder ("blub", "bla", ...);
*/
template<typename T>
class HWThreadBuilderTemplateRegistrator : public HWThreadBuilder
{
public:
    /**
      * as far as C++ doesn't support initializer lists, for any number of
      * keys a constructor is necessary
      */
    HWThreadBuilderTemplateRegistrator (std::string key)
    {
        registerKey (key);
    }

    HWThreadBuilderTemplateRegistrator (std::string key_1, std::string key_2)
    {
        registerKey (key_1);
        registerKey (key_2);
    }
    HWThreadBuilderTemplateRegistrator (std::string key_1, std::string key_2, std::string key_3)
    {
        registerKey (key_1);
        registerKey (key_2);
        registerKey (key_3);
    }
    HWThreadBuilderTemplateRegistrator (std::string key_1, std::string key_2, std::string key_3, std::string key_4)
    {
        registerKey (key_1);
        registerKey (key_2);
        registerKey (key_3);
        registerKey (key_4);
    }
    HWThreadBuilderTemplateRegistrator (std::string key_1, std::string key_2, std::string key_3, std::string key_4, std::string key_5)
    {
        registerKey (key_1);
        registerKey (key_2);
        registerKey (key_3);
        registerKey (key_4);
        registerKey (key_5);
    }
    HWThreadBuilderTemplateRegistrator (std::string key_1, std::string key_2, std::string key_3, std::string key_4, std::string key_5, std::string key_6)
    {
        registerKey (key_1);
        registerKey (key_2);
        registerKey (key_3);
        registerKey (key_4);
        registerKey (key_5);
        registerKey (key_6);
    }

    virtual HWThread* createInstance (boost::asio::io_service& io, ticpp::Iterator<ticpp::Element> hw) const
    {
        return new T (io, hw);
    }

private:
    HWThreadBuilderTemplateRegistrator () {}

    void registerKey (std::string const& key)
    {
        HWThreadFactory::instance().registerBuilder (key, new HWThreadBuilderTemplateRegistrator<T>);
    }
};

//-----------------------------------------------------------------------------
/**
* @class HWThreadBuilderTemplateRegistratorWithoutIOService
*
* @brief A template for the HWThreadBuilder which also registrates concrete builders
*        in the HWThreadFactory
*
* usage: create an instance of this class (static class member or a global variable)
*        HWThreadBuilderTemplateRegistratorWithoutIOService<ConcreteHWThread> my_concrete_hw_thread_builder ("blub", "bla", ...);
*/
template<typename T>
class HWThreadBuilderTemplateRegistratorWithoutIOService : public HWThreadBuilder
{
public:
    /**
      * as far as C++ doesn't support initializer lists, for any number of
      * keys a constructor is necessary
      */
    HWThreadBuilderTemplateRegistratorWithoutIOService (std::string key)
    {
        registerKey (key);
    }

    HWThreadBuilderTemplateRegistratorWithoutIOService (std::string key_1, std::string key_2)
    {
        registerKey (key_1);
        registerKey (key_2);
    }
    HWThreadBuilderTemplateRegistratorWithoutIOService (std::string key_1, std::string key_2, std::string key_3)
    {
        registerKey (key_1);
        registerKey (key_2);
        registerKey (key_3);
    }
    HWThreadBuilderTemplateRegistratorWithoutIOService (std::string key_1, std::string key_2, std::string key_3, std::string key_4)
    {
        registerKey (key_1);
        registerKey (key_2);
        registerKey (key_3);
        registerKey (key_4);
    }
    HWThreadBuilderTemplateRegistratorWithoutIOService (std::string key_1, std::string key_2, std::string key_3, std::string key_4, std::string key_5)
    {
        registerKey (key_1);
        registerKey (key_2);
        registerKey (key_3);
        registerKey (key_4);
        registerKey (key_5);
    }
    HWThreadBuilderTemplateRegistratorWithoutIOService (std::string key_1, std::string key_2, std::string key_3, std::string key_4, std::string key_5, std::string key_6)
    {
        registerKey (key_1);
        registerKey (key_2);
        registerKey (key_3);
        registerKey (key_4);
        registerKey (key_5);
        registerKey (key_6);
    }

    virtual HWThread* createInstance (boost::asio::io_service&, ticpp::Iterator<ticpp::Element> hw) const
    {
//      boost::shared_ptr<HWThread> shrd_ptr = boost::shared_ptr<HWThread>(new T (hw));
//      return ( shrd_ptr.get() );
      return ( new T (hw) );
    }

private:
    HWThreadBuilderTemplateRegistratorWithoutIOService () {}

    void registerKey (std::string const& key)
    {
        HWThreadFactory::instance().registerBuilder (key, new HWThreadBuilderTemplateRegistratorWithoutIOService<T>);
    }
};



} // namespace tobiss

#endif // HW_THREAD_BUILDER_H

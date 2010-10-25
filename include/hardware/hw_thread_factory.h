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
    HWThread* createHWThread (std::string const& key, boost::asio::io_service& io, XMLParser& parser, ticpp::Iterator<ticpp::Element> hw);

private:
    std::map<std::string, HWThreadBuilder*> builders_;

    HWThreadFactory () {}
    HWThreadFactory (HWThreadFactory const& src);
    HWThreadFactory& operator= (HWThreadFactory const& src);

};

} // namespace tobiss

#endif // HW_THREAD_FACTORY_H

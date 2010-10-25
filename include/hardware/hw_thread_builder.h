#ifndef HW_THREAD_BUILDER_H
#define HW_THREAD_BUILDER_H

#include "hw_thread.h"
#include "hw_thread_factory.h"

namespace tobiss { namespace registrators {
class HWThreadFactoryRegistrator;
} }

#define HW_THREAD_FACTORY_REGISTRATION(key, hw_thread_class) \
    namespace tobiss { \
    class hw_thread_class ## key ## Builder : public HWThreadBuilder { public: \
        virtual HWThread* createInstance (boost::asio::io_service& io, XMLParser& parser, ticpp::Iterator<ticpp::Element> hw) const \
        { return new hw_thread_class (io, parser, hw);}}; \
    namespace registrators { \
    HWThreadFactoryRegistrator hw_thread_registrator ## hw_thread_class ## key (#key, new hw_thread_class ## key ## Builder); \
    } }

namespace tobiss
{

//-----------------------------------------------------------------------------
/**
* @class HWThreadBuilder
*
* @brief Builder base class for HWThreads... it provides a factory method
*
*/
class HWThreadBuilder
{
public:
    virtual HWThread* createInstance (boost::asio::io_service& io, XMLParser& parser, ticpp::Iterator<ticpp::Element> hw) const = 0;
protected:
    HWThreadBuilder () {}
};



namespace registrators
{

class HWThreadFactoryRegistrator
{
public:
    HWThreadFactoryRegistrator (std::string const& key, HWThreadBuilder* builder)
    {
        HWThreadFactory::instance().registerBuilder (key, builder);
    }
};

} // namespace registrators


} // namespace tobiss

#endif // HW_THREAD_BUILDER_H

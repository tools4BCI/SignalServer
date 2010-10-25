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

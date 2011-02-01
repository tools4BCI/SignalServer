#ifndef FUSTY_HARDWARE_INTERFACE_IMPL_H
#define FUSTY_HARDWARE_INTERFACE_IMPL_H

#include "hardware_interface.h"
#include "tia-private/network/control_connection_server.h"

namespace tia
{

class FustyHardwareInterfaceImpl : public HardwareInterface
{
public:
    FustyHardwareInterfaceImpl (tobiss::ControlConnectionServer& cc_server)
        : cc_server_ (cc_server)
    {}

    tobiss::SSConfig getTiAMetaInfo () const
    {
        // really ugly code!!!!! :(
        tobiss::SSConfig tia_meta_info;
        tobiss::ConfigMsg config_msg;
        cc_server_.getConfig (config_msg);
        tia_meta_info.subject_info = config_msg.subject_info;
        tia_meta_info.signal_info = config_msg.signal_info;
        return tia_meta_info;
    }

private:
    tobiss::ControlConnectionServer& cc_server_;
};

}

#endif // FUSTY_HARDWARE_INTERFACE_IMPL_H

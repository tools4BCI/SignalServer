#ifndef HARDWARE_INTERFACE_H
#define HARDWARE_INTERFACE_H

#include "tia/ssconfig.h"

namespace tia
{

class HardwareInterface
{
public:
    virtual ~HardwareInterface () {}

    virtual tobiss::SSConfig getTiAMetaInfo () const = 0;

    // TODO: setHardwareConfig
};

}

#endif // HARDWARE_INTERFACE_H

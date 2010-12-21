
/**
* @file mouse.h
*
* @brief
*
**/

#ifdef _WIN32

#ifndef MOUSE_WIN_H
#define MOUSE_WIN_H

#include <cmath>

#include "mouse.h"
#include "extern\include\libusb\usb.h"

using namespace std;

namespace tobiss
{
//-----------------------------------------------------------------------------

class Mouse : public MouseBase
    {
        public:

        Mouse(ticpp::Iterator<ticpp::Element> hw);

        /**
        * @brief Destructor
        */
        ~Mouse();

    protected:
        virtual int blockKernelDriver();
        virtual int freeKernelDriver();
        virtual void acquireData();		
        static const string dc_path;
		
		struct usb_device **devs_; //pointer to pointer of device, used to retrieve a list of devices
        usb_dev_handle *dev_handle_; //a device handle
        string hw_id_;
		string devcon_path_;

        static const HWThreadBuilderTemplateRegistratorWithoutIOService<Mouse> FACTORY_REGISTRATOR_;
    };

} // Namespace tobiss

#endif // MOUSE_WIN_H
#endif // _WIN32
//-----------------------------------------------------------------------------

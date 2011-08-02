
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

  /**
  * @brief Constructor
  */
    Mouse(ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Destructor
    */
    virtual ~Mouse();

  protected:
    /**
    * @brief Block Windows kernel driver using devcon tool of WinDDK, load driver of libusb.
    */
    virtual int blockKernelDriver();

    /**
    * @brief Reactivate Windows kernel driver using devcon tool of WinDDK.
    */
    virtual int freeKernelDriver();

    /**
    * @brief Acquire data from usb-port.
    */
    virtual void acquireData();

    static const string dc_path_;

    usb_dev_handle *dev_handle_; //a device handle
    string hw_id_;
    string devcon_path_;

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<Mouse> FACTORY_REGISTRATOR_;
  };

} // Namespace tobiss

#endif // MOUSE_WIN_H
#endif // _WIN32
//-----------------------------------------------------------------------------
//

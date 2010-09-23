
/**
* @file mouse.h
*
* @brief
*
**/

#ifndef MOUSE_H
#define MOUSE_H

#ifdef WIN32
    #pragma comment(lib, "SDL.lib")
    #pragma comment(lib, "SDLmain.lib")
#endif

#include <boost/cstdint.hpp>
#include <set>
#include <SDL/SDL.h>
#include <cmath>

#include "hw_thread.h"
#include "extern/lib/libusb/linux/libusb-1.0.8/libusb/libusb.h"


using namespace std;

namespace tobiss
{
//-----------------------------------------------------------------------------

class Mouse : public HWThread
    {
    public:
      Mouse(XMLParser& parser, ticpp::Iterator<ticpp::Element> hw);

      /**
      * @brief Destructor
      */
      virtual ~Mouse();

      /**
      * @brief Method to achieve asynchronous data acquisition (method is non-blocking).
      * @return SampleBlock<double>
      *
      * This method return immediately after calling with a copy of the data stored in the device's
      * buffer.
      * It is called from all slave devices.
      */
      virtual SampleBlock<double> getAsyncData();
      /**
      * @brief Method to start data acquisition.
      */
      virtual void run();
      /**
      * @brief Method to stop data acquisition.
      */
      virtual void stop();

      //-----------------------------------------------
    private:

  //     void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

      virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);

      virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);

      virtual SampleBlock<double> getSyncData()   {return data_; }

      void initMouse();
      int blockKernelDriver();
      int freeKernelDriver();

      //-----------------------------------------------

    private:
    static set<boost::uint16_t> used_ids_;

      boost::uint16_t id_;

      boost::uint16_t buttons_;
      vector<bool> buttons_values_;
  
      boost::uint16_t axes_;
      vector<boost::int16_t> axes_values_;

      string name_;

      bool user_interrupt_;

      SampleBlock<double> empty_block_;

      int x_,y_;
      libusb_device **devs_; //pointer to pointer of device, used to retrieve a list of devices
      libusb_device_handle *dev_handle_; //a device handle
      libusb_context *ctx_; //a libusb session
  };



} // Namespace tobiss

#endif // MOUSE_H

//-----------------------------------------------------------------------------

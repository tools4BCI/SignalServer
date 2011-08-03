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
* @file mouse.h
*
* @brief
*
**/

#ifndef MOUSE_H
#define MOUSE_H

#include <boost/cstdint.hpp>
#include <set>

#include "hw_thread.h"
#include "hw_thread_builder.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class MouseBase
*
* @brief Base class to connected mouse devices.
*/
class MouseBase : public HWThread
{
public:

  /**
  * @brief Constructor
  */
  MouseBase(ticpp::Iterator<ticpp::Element> hw);

  /**
  * @brief Destructor
  */
  virtual ~MouseBase();

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
  protected:

  boost::uint32_t vid_;
  boost::uint32_t pid_;
  boost::uint32_t usb_port_;
    ticpp::Iterator<ticpp::Element> DS;

  static const std::string str_hw_vid_;
  static const std::string str_hw_pid_;
  static const std::string str_usb_port_;

  /**
  * @brief Sets vendorId for Mousedevice.
  */
  void setVendorId(ticpp::Iterator<ticpp::Element>const &elem);

  /**
  * @brief Sets productId for Mousedevice.
  */
  void setProductId(ticpp::Iterator<ticpp::Element>const &elem);

  /**
  * @brief Sets setUsbPort for Mousedevice.
  */
  void setUsbPort(ticpp::Iterator<ticpp::Element>const &elem);

  virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);

  virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);

  virtual SampleBlock<double> getSyncData()   {return data_; }

  /**
  * @brief Init Mouse settings.
  */
  virtual void initMouse();

  /**
  * @brief Method to detach kernel driver from the system and open connection to the mouse device.
  */
  virtual int blockKernelDriver();

  /**
  * @brief Method to attatch kernel driver to the system and close connection to the mouse device.
  */
  virtual int freeKernelDriver();

  /**
  * @brief Method to acquire data from the mouse device.
  */
  virtual void acquireData();

  //-----------------------------------------------

  static std::set<boost::uint16_t> used_ids_;

  boost::uint16_t id_;

  boost::uint16_t buttons_;
  std::vector<bool> buttons_values_;

  boost::uint16_t axes_;
  std::vector<boost::int16_t> axes_values_;

  std::string name_;

  bool user_interrupt_;

  SampleBlock<double> empty_block_;

  boost::thread*  async_acqu_thread_;
  std::vector<boost::int16_t>  raw_data_;
  int async_data_x_;
  int async_data_y_;
  int async_data_buttons_;
};

} // Namespace tobiss

#endif // MOUSE_H

//-----------------------------------------------------------------------------

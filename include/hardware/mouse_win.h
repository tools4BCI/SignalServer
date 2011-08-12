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
* @file mouse_win.h
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

  private:
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




  private:
    usb_dev_handle    *dev_handle_; //a device handle
    std::string       hw_id_;
    std::string       devcon_path_;
    std::string       inf_file_path_;

    static const std::string      hw_dc_path_;
    static const std::string      hw_inf_file_path_;

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<Mouse> FACTORY_REGISTRATOR_;
  };

} // Namespace tobiss

#endif // MOUSE_WIN_H
#endif // _WIN32
//-----------------------------------------------------------------------------
//

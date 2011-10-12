/*
    This file is part of the TOBI Interface A (TiA) library.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file lgpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/lgpl.html.

    In case of GNU Lesser General Public License Usage ,the TiA library
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TiA library. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: TiA@tobi-project.org
*/

#ifndef FUSTY_HARDWARE_INTERFACE_IMPL_H
#define FUSTY_HARDWARE_INTERFACE_IMPL_H

#include "hardware_interface.h"


namespace tia
{

//-----------------------------------------------------------------------------
/// hardware interface implementation as adapter to old (tobiss) stuff
class FustyHardwareInterfaceImpl : public HardwareInterface
{
public:
    FustyHardwareInterfaceImpl (ControlConnectionServer& cc_server)
//        : cc_server_ (cc_server)
    {
      // really ugly code!!!!! :(
      ConfigMsg config_msg;
      cc_server.getConfig (config_msg);
      tia_meta_info_.subject_info = config_msg.subject_info;
      tia_meta_info_.signal_info = config_msg.signal_info;
    }

    FustyHardwareInterfaceImpl (SubjectInfo subj_info, SignalInfo sig_info)
    {
      tia_meta_info_.subject_info = subj_info;
      tia_meta_info_.signal_info = sig_info;
    }

    SSConfig getTiAMetaInfo () const
    {
        return tia_meta_info_;
    }

private:
//    tobiss::ControlConnectionServer& cc_server_;
    SSConfig tia_meta_info_;
};

}

#endif // FUSTY_HARDWARE_INTERFACE_IMPL_H

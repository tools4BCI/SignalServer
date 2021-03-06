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

#ifndef GBSAMP_UNIX_H
#define GBSAMP_UNIX_H

#include <vector>
#include <map>

#include <boost/thread/condition.hpp>  // for mutex and cond. variables
#include <boost/thread/shared_mutex.hpp>
#include <boost/cstdint.hpp>
#include <comedilib.h>

#include "hardware/hw_thread_builder.h"
#include "hardware/gBSamp_base.h"

namespace tobiss
{
//-----------------------------------------------------------------------------
/**
* @class gBSamp
* @brief This class is used to gain access to PCMCIA-cards.
*
* This file is for Linux and uses the comedi-lib
*
*/

class gBSamp : public gBSampBase
{
  public:
    /**
    * @brief Constructor for initialization with an XML object
    * @param[in] hw ticpp::Element pointing to an \<hardware\> tag in the config file
    */
    gBSamp(ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Destructor
    */
    virtual ~gBSamp();

    /**
    * @brief Method to achieve synchronous data acquisition (method is blocking).
    * @return SampleBlock<double>
    *
    * This method returns after new data has been acquired, it is operating in blocking mode.
    * It is called only from the master device.
    */
    virtual SampleBlock<double> getSyncData();

    /**
    * @brief Method to achieve asynchronous data acquisition (method is non-blocking).
    * @return SampleBlock<double>
    *
    * This method return immediately after calling with a copy of the data stored in the device's
    * buffer.
    * It is called from all slave devices.
    * @warning If hardware clocks are not running with the same sampling rate or drifting
    * (e.g. master - 128Hz, slave 129Hz), this method could be called although no new data
    * is available (master faster than slave) or one sample has been lost (master slower than slave).
    * Up to now no compensation method (e.g. interpolation) has been implemented!
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

    /**
    * @brief Initialize device
    */
    int initCard();

//-----------------------------------------------

  private:
    boost::uint32_t expected_values_;

    std::vector<double> samples_; ///< temporary vector holding recent samples of the sine (1 element per channel)

    boost::int32_t error_;
    boost::int32_t first_run_;

    comedi_t* device_;
    comedi_cmd comedi_cmd_;
    comedi_range* range_info_;
    int maxdata_;
    unsigned int *channel_list_;
    //    sampl_t* data_buffer_[10000];
    //    unsigned int channel_list[0];
    //    comedi_insn comedi_insn_;
    //    comedi_insnlist insn_list_;
    //    lsampl_t data_buf_[1000];

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<gBSamp> factory_registrator_;

};

} // Namespace tobiss

#endif // GBSAMP_UNIX_H

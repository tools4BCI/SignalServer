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

#ifndef DATAQ_H
#define DATAQ_H

/**
* @file dataq.h
* @brief This file includes a class to gain access to DataQ DAQ devices.
**/

#include <vector>
#include <string>

#include "misc/databuffer.h"
#include "hardware/hw_thread.h"
#include "hardware/hw_thread_builder.h"
#include "hardware/dataq_wrapper.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/deadline_timer.hpp>

namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class DataQ
* @brief This class is used to gain access to DataQ DAQ devices.
*/
class DataQ : public HWThread
{
  public:
    /**
    * @brief Destructor
    */
    DataQ(ticpp::Iterator<ticpp::Element> hw);
    /**
    * @brief Destructor
    */
    virtual ~DataQ();

    virtual SampleBlock<double> getSyncData();
    virtual SampleBlock<double> getAsyncData();

    /**
    * @brief Method to start data acquisition.
    */
    virtual void run();
    /**
    * @brief Method to stop data acquisition.
    */
    virtual void stop();

  private:
    /**
    * @brief Set configuration defined in the device settings xml section.
    */
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);
    /**
    * @brief Set configuration defined in the channel settings xml section.
    */
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);
    /**
    * @brief Initialize the g.Mobilab device.
    */
    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

    void setDeviceRange(ticpp::Iterator<ticpp::Element>const &father);
    void setDeviceBipolar(ticpp::Iterator<ticpp::Element>const &father);

    void setChannelBipolar(ticpp::Iterator<ticpp::Element>const &father);
    void setChannelRange(ticpp::Iterator<ticpp::Element>const &father);


    int checkErrorCode(int error_code, bool throw_on_error);
    void printDataQInfo();
    int openDev(bool throw_on_error);
    int closeDev(bool throw_on_error);
    int startAcqu(bool throw_on_error);
    int stopAcqu(bool throw_on_error);
    void setInList();
    void setDaqMode();
    double getVoltageValue(short val, boost::uint16_t ch_nr);
    void acquireData();
    void checkBiploarConfig();
    void checkChannels();

  private:
    boost::thread*      acq_thread_;
    DataQWrapper*       dataq_dev_ptr_;
    unsigned int        dev_nr_;
    short*              dev_buffer_;

    bool                device_opened_;
    bool                acqu_running_;

    double              burst_rate_count_;
    unsigned int        last_pos_;

    di_inlist_struct*   inlist_ptr_;
    di_info_struct      info_;
    di_mode_struct      mode_;

    std::vector<bool>               bipolar_list_;
    std::vector<unsigned int>       gain_index_;
    std::vector<double>             v_max_;
    std::vector<double>             v_min_;
    DataBuffer<short>               buffer_;

    boost::asio::io_service         io_service_;
    boost::asio::deadline_timer     sleep_timer_;
    boost::posix_time::microseconds sleep_time_;
    boost::system::error_code       error_;

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<DataQ> factory_registrator_;

    //--------------------------------------
    // Constants:

    static const unsigned int           DMA_IN_SIZ;
    static const unsigned int           RING_BUFFER_SIZE;

    static const std::string            hw_dataq_range_;
    static const std::string            hw_dataq_bipolar_;
    static const std::string            hw_dataq_value_;

    int sample_count_;

};

//-----------------------------------------------------------------------------

}  // Namespace tobiss


#endif // DATAQ_H

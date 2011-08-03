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
* @file artificial_signal_source.h
* @brief This file includes a base class that can be used creating artificial signals.
**/

#ifndef ARTIFICIAL_SIGNAL_SOURCE
#define ARTIFICIAL_SIGNAL_SOURCE

#include <vector>
#include <map>

#include <boost/asio.hpp>
#include <boost/thread/condition.hpp>               // for mutex and cond. variables
#include <boost/thread/shared_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/cstdint.hpp>

#include "hw_thread.h"
#include "hardware/hw_thread_builder.h"

namespace tobiss
{
//-----------------------------------------------------------------------------
/**
* @class ArtificialSignalSource
*
* @brief A base class providing basic functionallity for artificial signal creation.
*/
class ArtificialSignalSource : public HWThread
{
  public:
    /**
    * @brief Destructor
    */
    virtual ~ArtificialSignalSource();

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


//-----------------------------------------------
  protected:
    /**
    * @brief Constructor for initialization with an XML object
    * @param[in] io_service boost::asio::io_service to create a proper timing
    * @param[in] parser Reference to XMLParser object
    * @param[in] hw ticpp::Element pointing to an \<hardware\> tag in the config file
    */
    ArtificialSignalSource(boost::asio::io_service& io, ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Method to start data acquisition.
    */
    virtual void run();
    /**
    * @brief Method to stop data acquisition.
    */
    virtual void stop();

    /**
    * @brief Method initializing the artificial signal source object.
    *
    *  This method initializes the artificial signal source object. Member variables
    *  like the stepsize and the timer used to assure appropriate timing are initialized.
    */
    void init();

    /**
    * @brief Set the configuration of the SineGenerator with a XML object
    * @param[in] hw ticpp::Element pointing to an \<hardware\> tag in the config file
    * @throw ticpp::exception if \<channel_settings\> is defined multiple times.
    */
    virtual void setHardware(ticpp::Iterator<ticpp::Element>const &hw) = 0;
    /**
    * @brief Abstract method generating the artificial signal.
    */
    virtual void generateSignal() = 0;
    /**
    * @brief Set configuration listed in the \<device_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    * @throw ticpp::exception
    */
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father) = 0;
    /**
    * @brief Set configuration listed in the \<channel_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<channel_settings\> tag in the config file
    * @throw ticpp::exception
    */
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father) = 0;

//-----------------------------------------------

  protected:
    bool acquiring_;   ///< to check, if data acquisition has started (needed if used as master)
    boost::asio::io_service&     io_;
    boost::asio::deadline_timer* t_;   ///< timer object for accurate timing
    double step_;        ///< needed for sine generation
    double cycle_dur_;   ///< needed for sine generation
    boost::uint16_t current_block_;     ///< counter variable -- only used if blocks >1
    boost::posix_time::microseconds td_;   ///< time period for the timer

    boost::mutex sync_mut_;  ///< mutex neede for synchronisation
    boost::condition_variable_any cond_;   ///< condition variable to wake up getSyncData()

    std::vector<double> samples_; ///< temporary vector holding recent samples of the sine (1 element per channel)

    /**
    * @brief Buffer object used if blockwise data generation is set.
    *
    * Samples are appended to the sample block.
    * This buffer is only for internal use and must not be accessible from outside!
    * For more information, read the SampleBlock documentation.
    */
    SampleBlock<double> buffer_;
};

} // Namespace tobiss

#endif // ARTIFICIAL_SIGNAL_SOURCE

//-----------------------------------------------------------------------------

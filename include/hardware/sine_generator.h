
/**
* @file sine_generator.h
*
* @brief sine_generator is a dummy hardware device for simulation purposes.
*
* sine_generator can be used the same way like a normal data acquisition device with reduced
* configuration possibilities.
* It produces a sine with a frequency of 1Hz on a selectable amount of channels in a buffered
* or unbuffered way. Generating a sine on multiple channels, an incrementing, channel dependent
* phase value is added to the sine to facilitate distinction.
* The sine generator can be operated in synchronous an asynchronous manner.
* It uses boost::asio and boost::thread to ensure proper timing.
*
**/

#ifndef SINEGENERATOR_H
#define SINEGENERATOR_H

#include <vector>
#include <map>

#include <boost/asio.hpp>
#include <boost/thread/condition.hpp>  // for mutex and cond. variables
#include <boost/thread/shared_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/cstdint.hpp>

#include "hw_thread.h"

namespace tobiss
{
//-----------------------------------------------------------------------------
/**
* @class SineGenerator
*
* @brief A dummy hardware device for simulation and testing purposes.
*
* SineGenerator provides a fully functional and equivalent dummy hardware device for testing
* purposes without any external data acquisition device.
* It supports synchronous and asynchronous data acquisition. Data can be produced with freely
* selectable frequencies in a block-based or non-block based way.
* To achive a consistent access to hardware devices, this class is a derived class.
*/
class SineGenerator : public HWThread
{
  public:
    /**
    * @brief Constructor for direct initialization
    *
    * @param[in] io boost::asio::io_service to create a proper timing
    * @param[in] sampling_rate
    * @param[in] nr_ch  Number of channels to be simulated by the SineGen.
    * @param[in] blocks  The blocksize, to create the data with.
    */
    SineGenerator(boost::asio::io_service& io, XMLParser& parser, const int sampling_rate, const int nr_ch, const int blocks)
      : HWThread(parser, sampling_rate, nr_ch, blocks), step_(1/static_cast<float>(fs_)),   \
        cycle_dur_(1/static_cast<float>(fs_)), current_block_(0), td_(1000000/fs_), samples_(1,0)
    {
      t_ = new boost::asio::deadline_timer(io, td_);
      genSine();
    }
    /**
    * @brief Constructor for initialization with an XML object
    * @param[in] io_service boost::asio::io_service to create a proper timing
    * @param[in] parser Reference to XMLParser object
    * @param[in] hw ticpp::Element pointing to an \<hardware\> tag in the config file
    */
    SineGenerator(boost::asio::io_service& io, XMLParser& parser, ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Destructor
    */
    virtual ~SineGenerator()    {  delete t_;  }

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
    * @brief Set the configuration of the SineGenerator with a XML object
    * @param[in] hw ticpp::Element pointing to an \<hardware\> tag in the config file
    * @throw ticpp::exception if \<channel_settings\> is defined multiple times.
    */
    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);
    /**
    * @brief Generate a sine curve and store it into the objects data vector.
    *
    * This method is used to gnerate a sine with 1Hz on eligible channels wether in
    * block oriented mode or not. Settings have to be given by initialization of the
    * object.
    * In non block oriented mode (buffersize = 1) data will be directly stored in the
    * "data" SampleBlock. Using block oriented data creation (buffersize > 1) data is
    * stored in the buffer first and if the desired number of blocks to store is achieved,
    * the buffer is copied into "data".
    * If new data is set, "samples_available" is set to true.
    */
    void genSine();
    /**
    * @brief Set configuration listed in the \<device_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    * @throw ticpp::exception
    */
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);
    /**
    * @brief Set configuration listed in the \<channel_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<channel_settings\> tag in the config file
    * @throw ticpp::exception
    */
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);

//-----------------------------------------------

  private:
    bool acquiring_;   ///< to check, if data acquisition has started (needed if used as master)
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

#endif // SINEGENERATOR_H

//-----------------------------------------------------------------------------

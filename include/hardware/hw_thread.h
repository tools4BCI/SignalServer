
/**
* @file hw_thread.h
*
* @brief hw_thread is a base class for all hardware objects ( -- bad naming --> object does not have to run in a thread)
*
* hw_thread provides a base class for all hardware objects to allow consistent access
* to different types of hardware.
* REMARK: The naming "hw_thread" was badly chosen, as it was planned at the beginning, to run every
* hardware object in it's own thread. Using synchronous and asynchronous data access, this is not
* needed all the time.
* File and class name are planned to be renamed in the future! (e.g. hw_base)
*
**/

#ifndef HWThread_H
#define HWThread_H

#define SLAVE       0
#define MASTER      1
#define APERIODIC   2

#include <map>
#include <string>

#include <boost/thread.hpp>
#include <boost/cstdint.hpp>

#include "ticpp/ticpp.h"

#include "config/xml_parser.h"
#include "definitions/defines.h"
#include "sampleblock/sample_block.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class HWThread
*
* @brief Base class for all hardware objects
*
* HWThread is the base class for all hardware objects providing abstract methods needed for
* a common interface to gather access to different types of hardware.
* It also provides methods and members needed for every data acquisition device (channel types, ...).
*
*/
class HWThread
{
  public:

    /**
    * @brief Default destructor
    */
    virtual ~HWThread()
    {    }

    /**
    * @brief Get the name of the device.
    * @return string device name
    */
    string getName()  { return(m_.find("name")->second); }

    /**
    * @brief Check if device is the master.
    * @return bool
    */
    bool isMaster()   { return(mode_ == MASTER); }

    /**
    * @brief Check if device is a slave.
    * @return bool
    */
    bool isSlave()   { return(mode_ == SLAVE); }

        /**
    * @brief Check if device is an aperiodic device (only delivers, if new data is available).
    * @return bool
    */
    bool isAperiodic()   { return(mode_ == APERIODIC); }

    /**
    * @brief Check if device is delivering only one SignalType (e.g. only EEG or only EMG)
    * @return bool
    */
    bool hasHomogenousSignalType()  { return(homogenous_signal_type_); }

    /**
    * @brief Get the blocksize set for the device.
    * @return uint16_t blocksize
    */
    boost::uint16_t getNrOfBlocks()    { return(blocks_); }
    /**
    * @brief Get the sampling rate set for the device.
    * @return uint16_t sampling_rate
    */
    boost::uint32_t getSamplingRate()  { return(fs_); }
    /**
    * @brief Get the number of channels the device is acquiering.
    * @return uint16_t nr_of_ch
    */
    boost::uint16_t getNrOfChannels()  { return(nr_ch_); }
    /**
    * @brief Get a vector containing channel types from acquired channels.
    * @return std::vector<uint16_t> channel_types
    */
    std::vector<boost::uint32_t> getChannelTypes()  { return(channel_types_); }
    /**
    * @brief Get a map containing all channel information
    * @return std::map<uint16_t channel, pair<string channel name, uint32_t channel type> > channel info
    */
    std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> > getChannelInfoMap()  { return(channel_info_); }
    /**
    * @brief Check if new samples are available.
    * @return bool
    */
    bool samplesAvailable();
    /**
    * @brief Check if all needed hardware tags from xml config are correctly set.
    * @param hw pointing to the \<hardware\> node in the xml config file
    * @throw ticpp::Exception if mandatory hardware tags not correctly set!
    */
    void checkMandatoryHardwareTags(ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Abstract method to achieve synchronous data acquisition (method is blocking).
    * @return SampleBlock<double>
    *
    * This method returns after new data has been acquired, it is operating in blocking mode.
    * It is called only from the master device.
    */
    virtual SampleBlock<double> getSyncData() = 0;
    /**
    * @brief Abstract method to achieve asynchronous data acquisition (method is non-blocking).
    * @return SampleBlock<double>
    *
    * This method return immediately after calling with a copy of the data stored in the device's
    * buffer.
    * It is called from all slave devices.
    * @warning If hardware clocks are not running with the same sampling rate or drifting
    * (e.g. master - 128Hz, slave 129Hz), this method could be called although no new data
    * is available (master faster than slave) or one sample has been lost (master slower than slave).
    * Up to now no compensation method (e.g. interpolation) has been implemented!
    * @todo Implement a compensation method, if clocks are drifting or similar (e.g. interpolation).
    */
    virtual SampleBlock<double> getAsyncData() = 0;
    /**
    * @brief Abstract method to start data acquisition.
    */
    virtual void run() = 0;

    /**
    * @brief Abstract method to stop data acquisition.
    */
    virtual void stop() = 0;

  protected:

    /**
    * @brief Constructor
    *
    * @param[in] sampling_rate Sampling rate of the device.
    * @param[in] channels Number of channels the device acquires.
    * @param[in] blocks   Blocksize used by the device.
    */
    HWThread(XMLParser& parser, boost::uint32_t sampling_rate, boost::uint16_t channels, boost::uint16_t blocks)
    : nr_ch_(channels),fs_(sampling_rate), samples_available_(0), blocks_(blocks),
    mode_(SLAVE), running_(0), parser_(parser)
    {    }
    /**
    * @brief Default constructor
    *
    * Sets the device to 0 channel, sampling_rate 0 and blocksize 1.
    */
    HWThread(XMLParser& parser)
    : nr_ch_(0),fs_(0), samples_available_(0), blocks_(1), mode_(SLAVE),
    running_(0), parser_(parser)
    {    }

    /**
    * @brief Set configuration listed in the \<device_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    * @throw ticpp::exception
    */
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father) = 0;
    /**
    * @brief Set configuration listed in the \<channel_settings\> section in the XML file.
    * @param[in] father ticpp::Element pointing to an \<channel_settings\> tag in the config file
    * @throw ticpp::exception
    */
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father) = 0;


    /**
    * @brief Set the sampling rate of the device.
    * @param elem pointing to the \<sampling_rate\> node in the xml config file
    * @throw ticpp::Exception if sampling rate is not a number or is zero.
    */
    void setSamplingRate(ticpp::Iterator<ticpp::Element>const &elem);
    /**
    * @brief Set parameters for all channels.
    * @param elem pointing to the \<measurement_channels\> node in the xml config file
    * @throw ticpp::Exception if attributes are not correctly set or nr of channels is not a number or zero.
    */
    void setDeviceChannels(ticpp::Iterator<ticpp::Element>const &elem);
    /**
    * @brief Set the blocksize of the device.
    * @param elem pointing to the \<buffersize\> node in the xml config file
    * @throw ticpp::Exception if sampling rate is not a number or is zero.
    */
    void setBlocks(ticpp::Iterator<ticpp::Element>const &elem);
    /**
    * @brief Set different parameters for channels (and overwrite settings from \<measurement_channels\>).
    * @param elem pointing to the \<channel_selection\> node in the xml config file
    * @throw ticpp::Exception if attributes are not correctly set or nr of channels is not a number or zero.
    *
    * This method also reports, if settings for all channels together have been defined
    * before and overwrites those settings.
    */
    void setChannelSelection(ticpp::Iterator<ticpp::Element>const &elem);
    /**
    * @brief Sets the vector channel_types.
    */
    void setChannelTypes();

    /**
    * @brief Sets vendorId for Mousedevice.
    */
    void setVendorId(ticpp::Iterator<ticpp::Element>const &elem);

    /**
    * @brief Sets productId for Mousedevice.
    */
    void setProductId(ticpp::Iterator<ticpp::Element>const &elem);

    /**
    * @brief Check, if a string represents a valid number.
    * @return bool
    */
    bool isNumber(const std::string& s)
    {
      for (unsigned int i = 0; i < s.length(); i++)
        if (!isdigit(s[i]))
          return false;
      return true;
    }

//-----------------------------------------------

  protected:
    boost::uint16_t nr_ch_;   ///< number of channels
    boost::uint32_t fs_;      ///< sampling rate
    bool samples_available_;
    boost::uint16_t blocks_;   ///< blocksize (or also called buffersize)
    boost::uint8_t mode_;       ///< master, slave or aperiodic
    bool running_;      ///< variable, to stop hardware
    boost::uint32_t vid_;
    boost::uint32_t pid_;

    boost::shared_mutex rw_;     ///< mutex to lock data

    std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> > channel_info_;  ///< map containing ( ch_nr,  (name,  type) )
    bool homogenous_signal_type_;  ///< true, if device delivers different signal types -- e.g. EEG and EMG
    std::vector<boost::uint32_t> channel_types_;   ///< vector containing signal types of channels (for faster access)
    std::map<std::string, std::string> m_;   ///< map with generic hardware information  ...  mandatory

    Constants cst_;  ///< A static object containing constants.
    XMLParser& parser_;   ///< Reference pointing to the XMLParser.

    /**
    * @brief Data object representing the last available samples from the SineGenerator.
    *
    * This SampleBlock contains the last affordable data for external components. If the
    * buffersize is 1 , than samples will directly be inserted into data, in blockwise mode,
    * "buffer" will be copied into "data", if the desired number of blocks is achieved.
    * Samples are in ascending order by their signal type.
    * "data" has to be secured by a mutex to cope with the reader/writer problem, as asynchronous
    * data access is also possible.
    * For more information, read the SampleBlock documentation.
    */
    SampleBlock<double> data_;
};

} // Namespace tobiss

#endif // HWThread_H

//-----------------------------------------------------------------------------

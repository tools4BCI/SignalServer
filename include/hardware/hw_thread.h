/*
    This file is part of the TOBI signal server.

    The TOBI signal server is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI signal server is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

/**
* @file hw_thread.h
* @brief hw_thread is a base class for all hardware objects ( -- bad naming --> object does not have to run in a thread)
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
#include "tia/defines.h"
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
* REMARK: The naming "hw_thread" was badly chosen, as it was planned at the beginning, to run every
* hardware object in it's own thread. Using synchronous and asynchronous data access, this is not
* needed all the time.
* File and class name are planned to be renamed in the future! (e.g. hw_base)
*
* @todo Write instruction how to derive from HWThread
* @todo Maybe rename HWTHread to HWBase
* @todo Implement unique device ID mechanism
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
    std::string getType()  { return(type_); }

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
    HWThread(boost::uint32_t sampling_rate, boost::uint16_t channels, boost::uint16_t blocks)
    : nr_ch_(channels),fs_(sampling_rate), samples_available_(0), blocks_(blocks),
    mode_(SLAVE), running_(0)
    {    }
    //    , parser_(parser)
    /**
    * @brief Default constructor
    *
    * Sets the device to 0 channel, sampling_rate 0 and blocksize 1.
    */
    HWThread()  // XMLParser& parser
    : nr_ch_(0),fs_(0), samples_available_(0), blocks_(1), mode_(SLAVE), running_(0)
    {    }
    //    , parser_(parser)

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
    /**
    * @brief Set the type (name) of the respective hardware device.
    * @todo Check if the type is set.
    */
    void setType(std::string s)
    {
      type_ = s;
    }

//-----------------------------------------------

  protected:
    boost::uint16_t nr_ch_;   ///< number of channels
    boost::uint32_t fs_;      ///< sampling rate
    bool samples_available_;
    boost::uint16_t blocks_;   ///< blocksize (or also called buffersize)
    boost::uint8_t mode_;       ///< master, slave or aperiodic
    bool running_;      ///< variable, to stop hardware

    boost::shared_mutex rw_;     ///< mutex to lock data

    std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> > channel_info_;  ///< map containing ( ch_nr,  (name,  type) )
    bool homogenous_signal_type_;  ///< true, if device delivers different signal types -- e.g. EEG and EMG
    std::vector<boost::uint32_t> channel_types_;   ///< vector containing signal types of channels (for faster access)

    // Constants cst_;  ///< A static object containing constants.
    // XMLParser& parser_;   ///< Reference pointing to the XMLParser.

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

    std::string   type_;

    //-----------------------------------------------
    // Constant variables & methods:

//    static const std::string hardware_;
//    static const std::string hardware_name_;
    static const std::string hardware_version_;
    static const std::string hardware_serial_;

    static const std::string hw_mode_;
    static const std::string hw_devset_;        ///< xml-tag hardware: device_settings
    static const std::string hw_fs_;            ///< xml-tag hardware: sampling_rate

    static const std::string hw_channels_;      ///< xml-tag hardware: measurement_channels
    static const std::string hw_ch_nr_;         ///< xml-tag hardware: nr
    static const std::string hw_ch_names_;      ///< xml-tag hardware: names
    static const std::string hw_ch_type_;       ///< xml-tag hardware: channel type

    static const std::string hw_blocksize_;     ///< xml-tag hardware: blocksize

    static const std::string hw_chset_;       ///< xml-tag hardware -- channel_settings
    static const std::string hw_chset_sel_;           ///< xml-tag hardware: selection
    static const std::string hw_chset_ch_;      ///< xml-tag hardware: ch
    static const std::string hw_chset_nr_;     ///< xml-tag hardware: nr
    static const std::string hw_chset_name_;   ///< xml-tag hardware: name


    //---------------------------------------------------------------------------------------

    bool equalsOnOrOff(const std::string& s);
    bool equalsYesOrNo(const std::string& s);
    bool equalsMaster(const std::string& s);
    bool equalsSlave(const std::string& s);
    bool equalsAperiodic(const std::string& s);
//    uint32_t getSignalFlag(const std::string& s);
//    std::string getSignalName(const boost::uint32_t& flag);

    void parseDeviceChannels(ticpp::Iterator<ticpp::Element>const &elem, boost::uint16_t& nr_ch,
                                 std::string& naming, std::string& type);

    void parseChannelSelection(ticpp::Iterator<ticpp::Element>const &elem, boost::uint16_t& ch,
                                   std::string& name, std::string& type);

    void checkMandatoryHardwareTagsXML(ticpp::Iterator<ticpp::Element> hw);


};

} // Namespace tobiss

#endif // HWThread_H

//-----------------------------------------------------------------------------

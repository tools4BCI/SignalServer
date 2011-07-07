#ifndef GBSAMP_UNIX_H
#define GBSAMP_UNIX_H

#include <vector>
#include <map>

#include <boost/asio.hpp>
#include <boost/thread/condition.hpp>  // for mutex and cond. variables
#include <boost/thread/shared_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/cstdint.hpp>
#include <comedilib.h>

#include "hw_thread.h"
#include "hardware/hw_thread_builder.h"

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

class gBSamp : public HWThread
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
    * @brief Set the configuration of the device with a XML object
    * @param[in] hw ticpp::Element pointing to an \<hardware\> tag in the config file
    * @throw ticpp::exception if \<channel_settings\> is defined multiple times.
    */
    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

    /**
    * @brief Initialize device
    */
    int initCard();

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

    void setDeviceFilterSettings(ticpp::Iterator<ticpp::Element>const &elem);
    void checkFilterAttributes(ticpp::Iterator<ticpp::Element>const &elem);
    void getFilterParams(ticpp::Iterator<ticpp::Element>const &elem,\
      unsigned int &type, bool &notch, float &f_low, float &f_high, float &sense);

//-----------------------------------------------

  private:
    bool acquiring_;   ///< to check, if data acquisition has started (needed if used as master)
    boost::uint16_t current_block_;     ///< counter variable -- only used if blocks >1
    boost::uint32_t expected_values_;

    boost::mutex sync_mut_;  ///< mutex needed for synchronisation
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

    boost::int32_t error;
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

    static const std::string hardware;
    static const std::string hardware_name;
    static const std::string hardware_version;
    static const std::string hardware_serial;
    static const std::string hw_mode;
    static const std::string hw_ds;   ///< xml-tag hardware: device_settings
    static const std::string hw_fs;   ///< xml-tag hardware: sampling_rate
    static const std::string hw_channels;   ///< xml-tag hardware: measurement_channels
    static const std::string hw_ch_nr;   ///< xml-tag hardware: nr
    static const std::string hw_ch_names;   ///< xml-tag hardware: names
    static const std::string hw_ch_type;   ///< xml-tag hardware: channel type
    static const std::string hw_buffer;   ///< xml-tag hardware: blocksize
    static const std::string hw_fil;   ///< xml-tag hardware: filter
    static const std::string hw_fil_type;   ///< xml-tag hardware: filter type
    static const std::string hw_fil_order;   ///< xml-tag hardware: filter order
    static const std::string hw_fil_low;   ///< xml-tag hardware: filter lower cutoff freq.
    static const std::string hw_fil_high;   ///< xml-tag hardware: filter upper cutoff freq.
    static const std::string hw_fil_sense;   ///< xml-tag hardware: mV for gBSamp
    static const std::string hw_notch;   ///< xml-tag hardware: notch_filter
    static const std::string hw_notch_center;    ///< xml-tag hardware: notch center freq.
    static const std::string hw_cs;   ///< xml-tag hardware -- channel_settings
    static const std::string hw_sel;   ///< xml-tag hardware: selection
    static const std::string hw_cs_ch;   ///< xml-tag hardware: ch
    static const std::string hw_cs_nr;   ///< xml-tag hardware: nr
    static const std::string hw_cs_name;   ///< xml-tag hardware: name

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<gBSamp> factory_registrator_;

};

} // Namespace tobiss

#endif // GBSAMP_UNIX_H

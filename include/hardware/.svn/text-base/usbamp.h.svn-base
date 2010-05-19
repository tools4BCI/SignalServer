
/**
* @file usbamp.h
*
* @brief usbamp.h is designed to gather access to the g.USBamp for the TOBI siganlserver.
* @todo Check, if asynchronous access to the g.USBamp is possible without reading and writing the same memory at the same time.
*
* This file only compiles under Microsoft Windows with the Visual Studio Compiler.
**/

#ifdef WIN32

#ifndef USBAMP_H
#define USBAMP_H

#include <boost/cstdint.hpp>

#include <vector>
#include <set>

#include <boost/thread/condition.hpp>  // for mutex and cond. variables
#include <boost/thread/shared_mutex.hpp>
#include <windows.h>

#include "extern/include/g.usbamp/gUSBamp.h"
//#pragma comment(lib,"gUSBamp.lib")

#include "hw_thread.h"


using namespace std;

static const unsigned int USBAMP_MAX_NR_OF_DEVICES    = 16;
// static const unsigned int USBAMP_TRIGGER_LINE_CHANNEL   = 17;
static const unsigned int USBAMP_MAX_NR_OF_CHANNELS   = 17;
static const unsigned int USBAMP_MAX_NR_OF_ANALOG_CHANNELS   = 16;
static const unsigned int USBAMP_NR_OF_CHANNEL_GROUPS = 4;
static const unsigned int USBAMP_NOTCH_HALF_WIDTH = 2;   // to one side  ...  e.g.  f_center = 50 Hz -->  48/52 Hz
static const unsigned int USBMAP_ERROR_MSG_SIZE = 256;
static const unsigned int DIGITS_TO_ROUND = 1;

//-----------------------------------------------------------------------------

/**
* @class USBamp
*
* @brief A class to wrap the API provided by g.tec for the g.USBamp and enable synchronous and asynchronous data access.
*
* This class is built to gather access to the g.tec g.USBamp (usbamp). It wraps the C API
* provided by g.tec to be used in the TOBI signalserver.
* It is able to acquire data from different usbamps in a synchronous and asynchronous manner
* and letting them appear as one single device.
* Configuration has to be done by an tinyXML object, satisfying the configuration file specifications
* for the signalserver project.
*/
class USBamp : public HWThread
{
  public:
//   USBamp(boost::asio::io_service& io, const HWConfig& config)
//   : HWThread(config.sampling_rate, config.nr_channels, config.blocks)
//   {
//     ch = new UCHAR [USBAMP_MAX_NR_OF_CHANNELS * sizeof(UCHAR)];
//     initUSBamp();
//   }

  USBamp(XMLParser& parser, ticpp::Iterator<ticpp::Element> hw);

  virtual ~USBamp();

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

//-----------------------------------------------

  private:

    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

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

    void setDefaultSettings();
    void setDeviceFilterSettings(ticpp::Iterator<ticpp::Element>const &elem);
    void setChannelFilterSettings(ticpp::Iterator<ticpp::Element>const &father);

    void setDeviceNotchSettings(ticpp::Iterator<ticpp::Element>const &elem);
    void setChannelNotchSettings(ticpp::Iterator<ticpp::Element>const &father);

    void setOperationMode(ticpp::Iterator<ticpp::Element>const &elem);
    void setShortCut(ticpp::Iterator<ticpp::Element>const &elem);
    void setTriggerLine(ticpp::Iterator<ticpp::Element>const &elem);
    void setUSBampMasterOrSlave(ticpp::Iterator<ticpp::Element>const &elem);
    void setCommonGround(ticpp::Iterator<ticpp::Element>const &father);
    void setCommonReference(ticpp::Iterator<ticpp::Element>const &father);
    void setBipolar(ticpp::Iterator<ticpp::Element>const &father);
    void setDrivenRightLeg(ticpp::Iterator<ticpp::Element>const &father);
    void setIndividualDrivenRightLeg(ticpp::Iterator<ticpp::Element> &elem);

    void checkFilterAttributes(ticpp::Iterator<ticpp::Element>const &elem);
    void getFilterParams(ticpp::Iterator<ticpp::Element>const &elem,\
      unsigned int &type, unsigned int &order, float &f_low, float &f_high);
    void checkNotchAttributes(ticpp::Iterator<ticpp::Element>const &elem);
    void getNotchParams(ticpp::Iterator<ticpp::Element>const &elem, float &f_center);

    void checkNrOfChannels();
    void checkTriggerLineChannel();

    void getHandles();
    void check4USBampError();
    void initFilterPtrs();
    void setUSBampChannels();
    int search4FilterID(unsigned int type, unsigned int order, float f_low, float f_high);
    int search4NotchID(float f_center);
    void setUSBampFilter();
    void setUSBampNotch();

    void initUSBamp();

    inline double roundD(double number, int digits = DIGITS_TO_ROUND)
    {
      return floor(number * pow(10., digits) + .5) / pow(10., digits);
    }

  //-----------------------------------------------

  private:
    static set<static string> serials_;

    bool enable_sc_;
    bool external_sync_;
    bool trigger_line_;
    boost::uint64_t sample_count_;
    boost::uint64_t error_count_;

    BYTE* driver_buffer_;
    DWORD driver_buffer_size_;
    DWORD bytes_received_;
    DWORD timeout_;

    WORD error_code_;
    CHAR* error_msg_;

    HANDLE  h_;
    HANDLE  data_Ev_;
    OVERLAPPED ov_;

    vector<double> samples_;
    vector<boost::uint8_t> channels_;


    int nr_of_bp_filters_;
    FILT* bp_filters_;

    int nr_of_notch_filters_;
    FILT* notch_filters_;

    vector<boost::uint8_t> filter_id_;
    vector<boost::uint8_t> notch_id_;
    GND ground_;
    REF reference_;
    CHANNEL bipolar_channels_;
    CHANNEL drl_channels_;
    string mode_;

    Constants cst_;
};

#endif // USBAMP_H

#endif // WIN32
//-----------------------------------------------------------------------------

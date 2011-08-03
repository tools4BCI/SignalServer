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
* @file usbamp.h
*
* @brief usbamp.h is designed to gather access to the g.USBamp for the TOBI siganlserver.
* @todo Check, if asynchronous access to the g.USBamp is possible without reading and writing the same memory at the same time.
*
* This file only compiles under Microsoft Windows with the Visual Studio Compiler.
**/

#ifndef USBAMP_H
#define USBAMP_H

#include <boost/cstdint.hpp>

#include <vector>
#include <set>

#include <boost/thread/condition.hpp>  // for mutex and cond. variables
#include <boost/thread/shared_mutex.hpp>
#include <windows.h>

#include "gtec_usbamp_wrapper.h"
#include "hw_thread.h"
#include "hw_thread_builder.h"

namespace tobiss
{

static const unsigned int DIGITS_TO_ROUND = 2;

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
*
* @attention Only g.USBamp verison B is supported and tested propperly
*
* @todo Implement Linux API
* @todo Support g.USBamp version A
* @todo Document also private methods
* @todo Implement also "auto-blocksize" mode
*
* @bug  Timeouts occur every second time the USBamp ist started.
*/
class USBamp : public HWThread
{
  public:
  /**
  * @brief Constructor
  */
  USBamp(ticpp::Iterator<ticpp::Element> hw);

  /**
  * @brief Destructor
  */
  virtual ~USBamp();

  /**
  * @brief Get data asynchronously.
  * @todo Implement this mehtod properly.
  */
  virtual SampleBlock<double> getSyncData();

  /**
  * @brief Get data asynchronously.
  * @todo Implement this mehtod properly.
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
      unsigned int &type, unsigned int &order, double &f_low, double &f_high);
    void checkNotchAttributes(ticpp::Iterator<ticpp::Element>const &elem);
    void getNotchParams(ticpp::Iterator<ticpp::Element>const &elem, float &f_center);

    void checkNrOfChannels();
    void adjustSettingsToChannelSelection();
    void checkTriggerLineChannel();

    void getHandles();
    void check4USBampError();
    void initFilterPtrs();
    void setUSBampChannels();
    int search4FilterID(unsigned int type, unsigned int order, double f_low, double f_high);
    int search4NotchID(float f_center);
    void printPossibleBandPassFilters();
    void printPossibleNotchFilters();
  void printAvailableAmps();
    void setUSBampFilter();
    void setUSBampNotch();

    void initUSBamp();

    void callGT_GetData();
    void callGT_ResetTransfer();
    void callGT_Start();

    void fillSyncBuffer();
    void fillSampleBlock();

    int getUSBampFilterType(const std::string& s);
    std::string getUSBampFilterName(double n);
    std::string getUSBampOpMode(const std::string& s);
    int getUSBampBlockNr(const std::string& s);

    inline double roundD(double number, int digits = DIGITS_TO_ROUND)
    {
      return floor(number * pow(10., digits) + .5) / pow(10., digits);
    }

  //-----------------------------------------------

  private:
    static std::set<static std::string> serials_;
    static bool is_usbamp_master_;
    std::string serial_;

    static USBamp*         master_device_;
    static std::vector<USBamp*>   slave_devices_;

    bool enable_sc_;
    bool external_sync_;
    bool trigger_line_;
    boost::uint32_t  trigger_line_sig_type_;

    boost::uint64_t sample_count_;

    boost::uint64_t error_count_;
    WORD error_code_;
    CHAR* error_msg_;

    std::vector<BYTE*> driver_buffer_;
    DWORD driver_buffer_size_;
    std::vector<DWORD> bytes_received_;
    DWORD timeout_;

    boost::uint32_t expected_values_;

    boost::uint32_t first_run_;
    boost::uint32_t current_overlapped_;

    HANDLE  h_;
    std::vector<HANDLE>  data_Ev_;
    std::vector<OVERLAPPED> ov_;

    std::vector<double> samples_;
    std::vector<boost::uint16_t> channels_;


    int nr_of_bp_filters_;
    FILT* bp_filters_;

    int nr_of_notch_filters_;
    FILT* notch_filters_;

    std::vector<boost::int16_t> filter_id_;
    std::vector<boost::int16_t> notch_id_;
    GND ground_;
    REF reference_;
    CHANNEL bipolar_channels_;
    CHANNEL drl_channels_;
    std::string mode_;

    GTECUSBampWrapper usb_amp_;

    std::map<std::string, std::string> m_;	/// Attributes mab -- to be renamed

    //-----------------------------------------------
    // Constants
    static const HWThreadBuilderTemplateRegistratorWithoutIOService<USBamp> FACTORY_REGISTRATOR_;

    static const std::string hw_filter_;   ///< xml-tag hardware: filter
    static const std::string hw_filter_type_;   ///< xml-tag hardware: filter type
    static const std::string hw_filter_order_;   ///< xml-tag hardware: filter order
    static const std::string hw_filter_low_;   ///< xml-tag hardware: filter lower cutoff freq.
    static const std::string hw_filter_high_;   ///< xml-tag hardware: filter upper cutoff freq.

    static const std::string hw_notch_;   ///< xml-tag hardware: notch_filter
    static const std::string hw_notch_center_;    ///< xml-tag hardware: notch center freq.


    static const std::string hw_opmode_;       ///< USBamp specific
    static const std::string hw_sc_;            ///< USBamp specific
    static const std::string hw_trigger_line_;            ///< USBamp specific
    static const std::string hw_usbampmaster_;   ///< USBamp specific
    static const std::string hw_comgnd_;        ///< USBamp specific
    static const std::string hw_gnd_;          ///< USBamp specific
    static const std::string hw_gnd_block_;   ///< USBamp specific
    static const std::string hw_gnd_value_;   ///< USBamp specific

    static const std::string hw_comref_;   ///< USBamp specific
    static const std::string hw_cr_;       ///< USBamp specific
    static const std::string hw_cr_block_;   ///< USBamp specific
    static const std::string hw_cr_value_;   ///< USBamp specific

    static const std::string hw_bipolar_;        ///< USBamp specific
    static const std::string hw_bipolar_with_;   ///< USBamp specific
    static const std::string hw_drl_;        ///< USBamp specific
    static const std::string hw_drl_value_;   ///< USBamp specific

    /**
    * @brief Mapping std::strings, representing g.USBamp filter types, and identifiers together.
    */
    std::map<std::string, unsigned int> usbamp_filter_types_;

    /**
    * @brief Mapping std::strings, representing g.USBamp operation modes, and identifiers together.
    */
    std::map<std::string, std::string> usbamp_opModes_;

    /**
    * @brief Mapping std::strings, representing g.USBamp block namings, and identifiers together.
    */
    std::map<std::string, unsigned int> usbamp_block_names_;

};

} // Namespace tobiss

#endif // USBAMP_H

//-----------------------------------------------------------------------------

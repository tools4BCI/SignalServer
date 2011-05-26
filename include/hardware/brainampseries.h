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

#ifndef BRAINAMPSERIES_H
#define BRAINAMPSERIES_H

/**
* @file brainampseries.h
* @brief This file includes a class to gain access to the devices from brainproducts brainamp series.
**/

#include <string>

#include <boost/cstdint.hpp>
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <conio.h>
#include <WinIoCtl.h>

#include "hardware/hw_thread.h"
#include "hardware/hw_thread_builder.h"

#include "extern/include/brainproducts/BrainAmpIoCtl.h"
#include "extern/include/FilterTools/iirbutterlpf.h"

namespace tobiss
{

//-----------------------------------------------------------------------------

class BrainAmpSeries : public HWThread
{
  public:
    BrainAmpSeries(ticpp::Iterator<ticpp::Element> hw);
    ~BrainAmpSeries();

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

    enum AmpType
    {
      None = 0, Standard = 1, MR = 2, DCMRplus = 3, ExGMR = 4
    };
    enum Resolution
    {
      r_100nV = 0, r_500nV = 1, r_10muV = 2, r_152_6muV = 3
    };
    enum AcquisitionType
    {
      Impedance = 0, DataAcqu = 1, TestSignal = 2
    };

    /**
    * @brief Set configuration defined in the device settings xml section.
    */
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);
    /**
    * @brief Set configuration defined in the channel settings xml section.
    */
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);
    /**
    * @brief Set the Brainamp config structure.
    */
    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

    void setBrainAmpSamplingRate(ticpp::Iterator<ticpp::Element>const &elem);
    void setDeviceLowImp(ticpp::Iterator<ticpp::Element>const &elem);
    void setDeviceTriggerValue(ticpp::Iterator<ticpp::Element>const &elem);
    void setDeviceLowpass250(ticpp::Iterator<ticpp::Element>const &elem);
    void setDeviceDCCoupling(ticpp::Iterator<ticpp::Element>const &elem);
    void setDeviceResolution(ticpp::Iterator<ticpp::Element>const &elem);
    void setDeviceCalibrationMode(ticpp::Iterator<ticpp::Element>const &elem);

    void setChannelLowpass250(ticpp::Iterator<ticpp::Element>const &father);
    void setChannelDCDecoupling(ticpp::Iterator<ticpp::Element>const &father);
    void setChannelResolution(ticpp::Iterator<ticpp::Element>const &father);

    void setTriggerLine(ticpp::Iterator<ticpp::Element>const &elem);
    void checkTriggerLineChannel();

    std::string getErrorMsg(int error_code);
    void setBrainAmpChannelList();
    bool OpenUSBDevice();
    std::vector<AmpType> GetConnectedAmps();
    void printConnectedAmplifiers( std::vector<AmpType> amps);
    void checkHighestChannelNr( std::vector<AmpType> amps);
    void initDownsamplingFilters();

    bool readData();
    void fillSampleVector();
    void doDownSamplingAndFillSampleVector();
    unsigned int findChannelPostionInChannelList(boost::uint32_t ch);

  private:

    std::vector<short>     raw_buffer_;
    std::vector<double>    downsampling_buffer_;
    std::vector<unsigned short>    trigger_buffer_;
    std::vector<double>    samples_;

    BA_SETUP                          brainamp_settings_;
    BA_CALIBRATION_SETTINGS           brainamp_calibration_settings_;

    long                              acqu_type_;
    HANDLE                            dev_handle_;
    int                               driver_version_;

    vector< IIRButterLpf<double> >    lp_filters_;
    unsigned int                      ds_factor_;   /// downsampling factor

    bool                              trigger_line_enabled_;
    boost::uint32_t                   trigger_line_sig_type_;


    //-----------------------------------------------
    // Constants

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<BrainAmpSeries> factory_registrator_;
    static const std::string hw_low_imp_;        ///< xml-tag hardware: use low impedance
    static const std::string hw_trigger_value_;   ///< xml-tag hardware: trigger hold value
    static const std::string hw_trigger_line_;    ///< xml-tag hardware: trigger line enabled or not
    static const std::string hw_250lp_;          ///< xml-tag hardware: lowpass 250Hz
    static const std::string hw_dc_coupl_;     ///< xml-tag hardware: DC decoupling
    static const std::string hw_resolution_;     ///< xml-tag hardware: resolution
    static const std::string hw_calibration_;    ///< xml-tag hardware: use calibration mode
      static const std::string hw_calib_on_;       ///< xml-tag hardware: calibration -- on/off
      static const std::string hw_calib_sig_;      ///< xml-tag hardware: calibration -- signal type
      static const std::string hw_calib_freq_;     ///< xml-tag hardware: calibration -- frequency

    static const std::string hw_value_;          ///< xml-tag hardware: attribute for on and off

    static const float scaling_values_[];
    std::map<std::string, unsigned int> resolution_names_;

    /**
    * @brief Mapping std::strings, representing g.USBamp block namings, and identifiers together.
    */
    std::map<std::string, unsigned int> calibration_signals_;

    /**
    * @brief Mapping AmpType and AmpName + nr of channels per specific amp together
    */
    std::map<AmpType, std::pair<std::string, unsigned int> > channels_per_amp_;  //

};

}  //tobiss

#endif // BRAINAMPSERIES_H

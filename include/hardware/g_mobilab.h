
#ifndef G_MOBILAB_H
#define G_MOBILAB_H

/**
* @file g_mobilab.h
*
**/

#include <vector>
#include <string>

#include "serial_port_base.h"


namespace tobiss
{

//-----------------------------------------------------------------------------

/**
* @class GMobilab
*
* @brief
*/
class GMobilab : public SerialPortBase
{
  public:
    GMobilab(boost::asio::io_service& io, XMLParser& parser,
             ticpp::Iterator<ticpp::Element> hw);
    virtual ~GMobilab()
    {
      close();
    }


    virtual SampleBlock<double> getSyncData();

    /**
    * @brief Method to start data acquisition.
    */
    virtual void run();
    /**
    * @brief Method to stop data acquisition.
    */
    virtual void stop();

  private:

    virtual SampleBlock<double> getAsyncData();

    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);
    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

    void setScalingValues();
    unsigned char getChannelCode();
    void checkNrOfChannels();

    enum device_types_ { EEG, MULTI };

  private:
    device_types_ type_;
    std::map<unsigned int, unsigned char> channel_coding_;
    std::vector<double> scaling_factors_;
    std::vector<unsigned char>  raw_data_;
    std::vector<double>  samples_;


};

//-----------------------------------------------------------------------------

}  // Namespace tobiss


#endif // G_MOBILAB_H

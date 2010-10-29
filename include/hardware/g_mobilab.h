
#ifndef G_MOBILAB_H
#define G_MOBILAB_H

/**
* @file g_mobilab.h
*
**/

#include <vector>
#include <string>

#include <boost/thread.hpp>

#include "serial_port_base.h"
#include "hardware/hw_thread.h"
#include "hardware/hw_thread_builder.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class GMobilab
*
* @brief
*/
class GMobilab : private SerialPortBase, public HWThread
{
  public:
    GMobilab(boost::asio::io_service& io, XMLParser& parser,
             ticpp::Iterator<ticpp::Element> hw);
    virtual ~GMobilab()
    {
      async_acqu_thread_->join();
      close();
      if(async_acqu_thread_)
        delete async_acqu_thread_;
    }


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
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);
    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

    void setScalingValues();
    unsigned char getChannelCode();
    void checkNrOfChannels();

    void acquireData();

    enum device_types_ { EEG, MULTI };

  private:
    device_types_ type_;
    std::map<unsigned int, unsigned char> channel_coding_;
    std::vector<double> scaling_factors_;
    std::vector<boost::int16_t>  raw_data_;
    std::vector<double>  samples_;

    boost::thread*  async_acqu_thread_;

    static const HWThreadBuilderTemplateRegistrator<GMobilab> factory_registrator_;

};

//-----------------------------------------------------------------------------

}  // Namespace tobiss


#endif // G_MOBILAB_H

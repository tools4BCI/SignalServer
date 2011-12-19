
#ifndef PLUX_H
#define PLUX_H

#include "hw_thread.h"
#include "hw_thread_builder.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class Plux
* @brief A class to access BioPlux Devices.
* @todo Everything.
*/
class Plux : public HWThread
{
public:

	 /**
    * @brief Constructor
    */
    Plux (ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Destructor
    */
    virtual ~Plux();
	
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

private:

    /**
    * @brief Set configuration listed in the \<device_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    * @throw ticpp::exception
    */
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);

    /**
    * @brief Set configuration listed in the \<channel_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    */
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);

	static const HWThreadBuilderTemplateRegistratorWithoutIOService<Plux> FACTORY_REGISTRATOR_;
};

} // Namespace tobiss

#endif

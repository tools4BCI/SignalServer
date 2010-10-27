
/**
* @file jstick.h
*
* @brief
*
**/

#ifndef JSTICK_H
#define JSTICK_H

#ifdef WIN32
    #pragma comment(lib, "SDL.lib")
    #pragma comment(lib, "SDLmain.lib")
#endif

#include <boost/cstdint.hpp>

#include <set>

#include "hw_thread.h"
#include "hw_thread_builder.h"

using namespace std;

namespace tobiss
{
//-----------------------------------------------------------------------------

class JStick : public HWThread
{
  public:
    JStick (XMLParser& parser, ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Destructor
    */
    virtual ~JStick();

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

    //-----------------------------------------------
  private:

//     void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);

    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);

    /**
    * @brief Method to achieve synchronous data acquisition not planned to be used for joysticks.
    */
    virtual SampleBlock<double> getSyncData()   {return data_; }

    void initJoystick();

    //-----------------------------------------------

  private:
  static set<boost::uint16_t> used_ids_;

    void* joy_;   //FIXME: should be of type SDL_Joystick  ... problems with includes
    boost::uint16_t id_;

    boost::uint16_t buttons_;
    vector<bool> buttons_values_;

    boost::uint16_t axes_;
    vector<boost::int16_t> axes_values_;

    boost::uint16_t balls_;
    vector< pair<int,int> > balls_values_;

    string name_;

    SampleBlock<double> empty_block_;

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<JStick> FACTORY_REGISTRATOR_;

};

} // Namespace tobiss

#endif // JSTICK_H

//-----------------------------------------------------------------------------

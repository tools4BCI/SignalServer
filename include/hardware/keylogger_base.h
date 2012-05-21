
/**
* @file keylogger.h
*
* @brief
*
**/

#ifndef KEYLOGGER_BASE_H
#define KEYLOGGER_BASE_H

#include <set>

#include "keylogger_base.h"
#include "hw_thread.h"

#include <boost/circular_buffer.hpp>


namespace boost
{
  class thread;
}

namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class KeyLoggerBase
*
* @brief Base class to connect keyboards.
*/
class KeyLoggerBase: public HWThread
{
  public:
    KeyLoggerBase(ticpp::Iterator<ticpp::Element> hw);
    virtual ~KeyLoggerBase();

    virtual void run();
    virtual void stop();

  protected:
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);

    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);

    virtual SampleBlock<double> getAsyncData();


  private:

    virtual SampleBlock<double> getSyncData()   {return empty_block_; }

    // void init();

    void setDetachFromOS(ticpp::Iterator<ticpp::Element>const &elem);
    virtual void acquireData() = 0;

  protected:
    bool                      dirty_;
    bool                      detach_from_os_;
    boost::thread*            async_acqu_thread_;
    SampleBlock<double>       empty_block_;

    bool                                 released_;
    boost::circular_buffer<char>         pressed_keycodes_;

  private:

    static std::set<boost::uint16_t> used_ids_;
    boost::uint16_t id_;

    static const std::string str_detach_from_os_;
};

} //tobiss

#endif // KEYLOGGER_BASE_H

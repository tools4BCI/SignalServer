#include "hardware/keylogger_linux.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <X11/Xlib.h>

namespace tobiss
{

const HWThreadBuilderTemplateRegistratorWithoutIOService<KeyLogger> KeyLogger::FACTORY_REGISTRATOR_ ("keylogger");

//-----------------------------------------------------------------------------

KeyLogger::KeyLogger(ticpp::Iterator<ticpp::Element> hw)
  : KeyLoggerBase(hw)
{
  pressed_keycodes_.resize(32);

  if(!detach_from_os_)
  {
    dsp_ = XOpenDisplay( NULL );
  }
}

//-----------------------------------------------------------------------------

KeyLogger::~KeyLogger()
{
  running_ = false;
  async_acqu_thread_->join();
  if(async_acqu_thread_)
    delete async_acqu_thread_;

  if(!detach_from_os_)
     dsp_ = XOpenDisplay(NULL);
}

//-----------------------------------------------------------------------------

void KeyLogger::acquireData()
{
  while(running_)
  {
    if(!detach_from_os_)
    {
      boost::unique_lock<boost::shared_mutex> lock(rw_);

      char keys_return[32] = {0};
      XQueryKeymap(dsp_,keys_return);

      unsigned int keys_hit = 0;

      for (int i=0; i<32; i++)
      {
        if (keys_return[i] != 0)
        {
          int pos = 0;
          int num = keys_return[i];

          while ( pos < 8 )
          {
            if ((num & 0x01) == 1)
            {
              keys_hit++;

              if(released_)
              {
                dirty_ = true;
                pressed_keycodes_.push_back(XKeycodeToKeysym(dsp_, i*8+pos, 0));
                released_ = false;
              }
              // XKeysymToString(sym) --> to get a char*
            }

            pos++;
            num /= 2;
          }
        }
      }

      if(!keys_hit)
        released_ = true;

      lock.unlock();
      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }
  }
}

//-----------------------------------------------------------------------------


}


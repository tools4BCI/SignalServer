
/**
* @file keylogger.h
*
* @brief
*
**/

#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#include "keylogger_base.h"
#include "hw_thread_builder.h"

struct _XDisplay;
union _XEvent;

namespace tobiss
{

//-----------------------------------------------------------------------------

class KeyLogger : public KeyLoggerBase
{
  public:
    KeyLogger(ticpp::Iterator<ticpp::Element> hw);
    virtual ~KeyLogger();


  private:
    virtual void acquireData();


  private:

    _XDisplay*              dsp_;


    static const HWThreadBuilderTemplateRegistratorWithoutIOService<KeyLogger> FACTORY_REGISTRATOR_;

};

} //tobiss

#endif // KEYLOGGER_H

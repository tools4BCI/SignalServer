
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

#include <Windows.h>

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
    static LRESULT CALLBACK keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
    HHOOK  hook_;

  private:
    static const HWThreadBuilderTemplateRegistratorWithoutIOService<KeyLogger> FACTORY_REGISTRATOR_;

};

} //tobiss

#endif // KEYLOGGER_H


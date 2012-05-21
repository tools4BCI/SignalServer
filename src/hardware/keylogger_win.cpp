#include "hardware/keylogger_win.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

using std::cout;
using std::endl;

namespace tobiss
{

const HWThreadBuilderTemplateRegistratorWithoutIOService<KeyLogger> KeyLogger::FACTORY_REGISTRATOR_ ("keylogger");

//-----------------------------------------------------------------------------

KeyLogger::KeyLogger(ticpp::Iterator<ticpp::Element> hw)
  : KeyLoggerBase(hw)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif
}

//-----------------------------------------------------------------------------

KeyLogger::~KeyLogger()
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  UnhookWindowsHookEx(hook_);
  running_ = false;
  if(async_acqu_thread_)
    delete async_acqu_thread_;

}

//-----------------------------------------------------------------------------

void KeyLogger::acquireData()
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  if(!detach_from_os_)
  {
    hook_ = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyLogger::keyboardHookProc, 0, 0);
    MSG messages;
    GetMessage (&messages, NULL, 0, 0);
  }
}

//-----------------------------------------------------------------------------

LRESULT CALLBACK KeyLogger::keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) (lParam);
  // If key is being pressed
  if (wParam == WM_KEYDOWN)
  {
    #ifdef DEBUG
      switch (p->vkCode)
      {
        // Invisible keys
        case VK_CAPITAL: cout << "<CAPLOCK>"; break;
        case VK_SHIFT: cout << "<SHIFT>"; break;
        case VK_LCONTROL: cout << "<LCTRL>"; break;
        case VK_RCONTROL: cout << "<RCTRL>"; break;
        case VK_INSERT: cout << "<INSERT>"; break;
        case VK_END: cout << "<END>"; break;
        case VK_PRINT: cout << "<PRINT>"; break;
        case VK_DELETE: cout << "<DEL>"; break;
        case VK_BACK: cout << "<BK>"; break;
        case VK_LEFT: cout << "<LEFT>"; break;
        case VK_RIGHT: cout << "<RIGHT>"; break;
        case VK_UP: cout << "<UP>"; break;
        case VK_DOWN: cout << "<DOWN>"; break;
        case VK_RETURN: cout << "<RETURN>" << endl; break;
          // Visible keys
        default:
          std::cout << char(tolower(p->vkCode)) << std::endl;
      }
    #endif
    BYTE keyState[256];
    WCHAR buffer[16];
    ToUnicode(p->vkCode, p->scanCode, (PBYTE)&keyState, (LPWSTR)&buffer, sizeof(buffer) / 2, 0);

    boost::unique_lock<boost::shared_mutex> lock(static_rw_);
    pressed_keycodes_.push_back(buffer[0]);
    dirty_ = true;
    lock.unlock();
  }
  
  return CallNextHookEx(NULL, nCode, wParam, lParam);
}

//-----------------------------------------------------------------------------

}


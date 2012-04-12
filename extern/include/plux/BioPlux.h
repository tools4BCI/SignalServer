#include <vector>
#include <string>

#ifndef _WINDEF_
typedef unsigned char    BYTE;
typedef unsigned short	 WORD;
typedef unsigned __int64 UINT64;
#endif

#ifndef _BIOPLUXHEADER_
#define _BIOPLUXHEADER_

namespace BP
{
   class Device
   {
   public:
      struct Frame
      {
         BYTE  seq;
         bool  dig_in;
         WORD  an_in[8];
      };

      static void    FindDevices(std::vector<std::string> &devs);
      static Device* Create(const std::string &port);

      virtual void   GetDescription(std::string &str)=0;
      virtual void   BeginAcq(void)=0;
      virtual void   BeginAcq(int freq, BYTE chmask, BYTE nbits)=0;
      virtual void   GetFrames(int nframes, Frame *frames)=0;
      virtual void   SetDOut(bool dout)=0;
      virtual void   EndAcq(void)=0;
      virtual        ~Device() {}
   };

   class Err
   {
   public:
      enum Code {
         // Notifications
		   //BT_AUTHENTICATION,
		   BT_ADDRESS = 1,
		   BT_ADAPTER_NOT_FOUND,
		   BT_DEVICE_NOT_FOUND,
		   CONTACTING_DEVICE,
		   PORT_COULD_NOT_BE_OPENED,
         // Errors
		   PORT_INITIALIZATION,
		   //FIRMWARE_NOT_SUPPORTED,
		   DEVICE_NOT_IDLE,
		   DEVICE_NOT_IN_ACQUISITION_MODE,
		   PORT_COULD_NOT_BE_CLOSED,
		   BT_DEVICE_NOT_PAIRED,
         INVALID_PARAMETER,
         FUNCTION_NOT_SUPPORTED
      } code;

      enum Type {
         TYP_ERROR,
         TYP_NOTIFICATION
      };

      Err(Code c) : code(c) {}
      Type        GetType(void);
      const char* GetDescription(void);
   };
}

#endif

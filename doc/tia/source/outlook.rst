Outlook
=======

Future versions of TiA should guarantee downward compatibility. That means all TiA 1.0 commands should be supported by at least
all 1.x versions.


Error Messages
--------------
Error messages may contain error codes to automatically interpret the meaning of an error.


Datapackets
-----------
Important remark: Due to downward compatibility no datapacket can become version 10! (as the first byte
in datapacket version 2 is "10").
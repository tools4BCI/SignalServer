Changelog
=========

SignalServer v1.0
-----------------

Introduced xml message version 1.0
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 
This message version upgrade makes Signal Server 0.1 **incompatible** with version 1.0.
 
* 0.1 server / 1.0 client
 
  * The connection with the Signal Server is aborted. The client throws an exception and/or displays an error message.
 
* 1.0 server / 0.1 client
 
  * The client does not recognize the new message format and is waiting for a valid massage. It has to be terminated manually!
    The server keeps running normaly.


* Nearly added g.BSamp support

* Added mouse support

* Finished EEG simulator (Proper remote config tool still missing -- simple client available)

* Fixed g.USBamp sorting error

* Added .gdf file writing 

* Added support for the TiDServer

* Decpouling of old and new TiA Server

* Introduced DataPacket interface and new DataPacket3

* Various bugfixes

* Performance increase


SignalServer v0.2
-----------------

* Added brainamp series support

* Partly added EEG simulator support

* Fixed g.USBamp timeout bug after restart

* Various bugfixes

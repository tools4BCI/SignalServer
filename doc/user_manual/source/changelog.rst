Changelog
=========

Signal Server v0.2
^^^^^^^^^^^^^^^^^^

* Introduced xml message version 0.2

  This message version upgrade makes Signal Server 0.1 **incompatible** with version 0.2.

  * 0.1 server / 0.2 client

    The connection with the Signal Server is aborted. The client throws an exception and/or displays an error message.

  * 0.2 server / 0.1 client

    The connection with the Signal Server is aborted. The server displays an error message but keeps running.


* Added brainamp series support

* Partly added EEG simulator support

* Added g.BSamp support

* Fixed g.USBamp timeout bug after restart

* Various bugfixes

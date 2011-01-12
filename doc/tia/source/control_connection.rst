Control Connection
==================

A TiA server has to provide a TCP port on which clients can create a control connection.
Each client gets its own control connection to the server.

The control connection can be used by the clients to send requests to the server such as
"start/stop data transmission".

Server Commands
---------------

A TiA server implementation has to support the following commands:


Get Config
^^^^^^^^^^

This command is used to get the informations about signals from the server.

XML Representation
******************
::

  <?xml version="1.0" encoding="UTF-8"?>
  <message version="1.0">
    <getConfig/>
  </message>

Server Response
***************
::

  <?xml version="1.0" encoding="UTF-8"?>
  <message version="1.0">
    <config>...</config>
  </message>


Get Data Transmission
^^^^^^^^^^^^^^^^^^^^^

Two types of data transmissions exist: "tcp" and "udp".

XML Representation
******************
::

  <?xml version="1.0" encoding="UTF-8"?>
  <message version="1.0">
    <getDataTransmission type="tcp"/>
  </message>

Optionally the ``type`` attribute can be set to ``udp``.

Server Response
***************
::

  <?xml version="1.0" encoding="UTF-8"?>
  <message version="1.0">
    <dataConnection port="XXXX" />
  </message>

Start Data Transmission
^^^^^^^^^^^^^^^^^^^^^^^
XML Representation
******************
::

  <?xml version="1.0" encoding="UTF-8"?>
  <message version="1.0">
    <start/>
  </message>


Server Response
***************
Server will not respond to that message.


Stop Data Transmission
^^^^^^^^^^^^^^^^^^^^^^
XML Representation
******************
::

  <?xml version="1.0" encoding="UTF-8"?>
  <message version="1.0">
    <stop/>
  </message>

Server Response
***************
Server will not respont to that message.
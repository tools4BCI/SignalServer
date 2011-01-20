Control Connection
==================

A TiA server has to provide a TCP port on which clients can create a control connection.
Each client gets its own control connection to the server.

The control connection can be used by the clients to send requests to the server such as
"start/stop data transmission".

Each message which is send from the client to the server or vice versa is structured as followed:

* Version line
* Command line
* Optional content description line
* Empty lines
* Optional xml content


Remarks:

* The protocol is in the style of HTTP
* The message is split into lines which are terminated by the 0x0A character (also known as ``\n``, line feed or <LF>)
* Some messages contain additional XML-structured content which is UTF-8 encoded
* All characters are case sensitive!


Server Commands
---------------

A TiA 1.0 server implementation has to support the following commands:

* Check protocol version
* Get config
* Get data connection
* Start data transmission
* Stop data transmission

Check Protocol Version
^^^^^^^^^^^^^^^^^^^^^^
This command may be used by the client to check if the server understands the commands the client wants to send.
The server has to respond with an OK message if it understands commands of the given protocol version.

Representation
**************
::

  TiA 1.0 \n
  CheckProtocolVersion \n
  \n

Server Response
***************
::

  TiA 1.0 \n
  OK \n
  \n

or 

::

  TiA 1.0 \n
  Error \n
  \n


Get Config
^^^^^^^^^^

This command is used to get the informations about signals from the server.

Representation
**************
::

  TiA 1.0 \n
  GetConfig \n
  \n


Server Response
***************
::

  TiA 1.0 \n
  Config \n
  Content-Length: [Length of XML Content in Bytes] \n
  \n
  <config>....</config>

or

::

  TiA 1.0 \n
  Error \n
  \n


Get Data Transmission
^^^^^^^^^^^^^^^^^^^^^

Two types of data transmissions exist: "TCP" and "UDP".

XML Representation
******************
::

  TiA 1.0 \n
  GetDataConnection: TCP \n
  \n

or

::

  TiA 1.0 \n
  GetDataConnection: UDP \n
  \n


Server Response
***************
::

  TiA 1.0 \n
  DataConnectionPort: [Port-Number] \n
  \n
  
or

::

  TiA 1.0 \n
  Error \n
  \n


Start Data Transmission
^^^^^^^^^^^^^^^^^^^^^^^
XML Representation
******************
::

  TiA 1.0 \n
  StartDataTransmission \n
  \n


Server Response
***************
::

  TiA 1.0 \n
  OK \n
  \n

or

::

  TiA 1.0 \n
  Error \n
  \n
  


Stop Data Transmission
^^^^^^^^^^^^^^^^^^^^^^
XML Representation
******************
::

  TiA 1.0 \n
  StopDataTransmission \n
  \n

Server Response
***************
::

  TiA 1.0 \n
  OK \n
  \n

or

::

  TiA 1.0 \n
  Error \n
  \n

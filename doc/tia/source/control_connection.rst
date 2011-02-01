Control Connection
==================

A TiA server has to provide a TCP port on which clients can create a control connection.
Each client gets its own control connection to the server.

The control connection can be used by the clients to send requests to the server such as
"start/stop data transmission".

Some important remarks:

1. The protocol is in the style of HTTP (line structured text messages)
2. The messages are encoded in UTF-8
3. The message is split into lines which are terminated by the 0x0A character (also known as ``\n``, "line feed" or <LF>)
4. Some messages contain additional XML-structured content which is UTF-8 encoded
5. All characters are case sensitive!


Control Message Structure
-------------------------

Each message which is send from the client to the server or vice versa is structured as followed:

1. Version line
2. Command line
3. Optional content description line
4. An empty line
5. Optional xml-structured content

Example:
::
  
  TiA 1.0\n
  CheckProtocolVersion\n
  \n

Example with additional xml-structured content
::

  TiA 1.0\n
  MetaInfo\n
  Content-Length: 79\n
  \n
  <?xml version="1.0" encoding="UTF-8"?><tiaMetaInfo version="1.0"></tiaMetaInfo>


Reply Messages
--------------
Each command message is answered with an reply message which contains either an OK or an Error.
Error messages optionally contain an error description.

OK message:
::

  TiA 1.0\n
  OK\n
  \n

Error message without an error description:
::

  TiA 1.0\n
  Error\n
  \n


Error message including an error description:
::

  TiA 1.0\n
  Error\n
  Content-Length: 73\n
  <tiaError version="1.0" description="Human readable error description."/>



Server Commands
---------------

A TiA 1.0 server implementation has to support the following commands:

* Check protocol version
* Get metainfo
* Get data connection
* Start data transmission
* Stop data transmission
* Get server state connection

Check Protocol Version
^^^^^^^^^^^^^^^^^^^^^^
This command may be used by the client to check if the server understands the commands the client wants to send.
The server has to respond with an OK message if it understands commands of the given protocol version.

Representation:
::

  TiA 1.0\n
  CheckProtocolVersion\n
  \n

Server Response
::


or 



Get MetaInfo
^^^^^^^^^^^^

This command is used to get the informations about the signals from the server.

Representation:
::

  TiA 1.0\n
  GetMetaInfo\n
  \n


Server Response:
::

  TiA 1.0\n
  MetaInfo\n
  Content-Length: [Length of XML Content in Bytes]\n
  \n
  <?xml version="1.0" encoding="UTF-8"?><tiaMetaInfo version="1.0">....</tiaMetaInfo>

or

::

  TiA 1.0\n
  Error\n
  \n


Get Data Transmission
^^^^^^^^^^^^^^^^^^^^^

Two types of data transmissions exist: "TCP" and "UDP".

Representation:
::

  TiA 1.0 \n
  GetDataConnection: TCP \n
  \n

or

::

  TiA 1.0 \n
  GetDataConnection: UDP \n
  \n


Server Response:
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
Representation:
::

  TiA 1.0 \n
  StartDataTransmission \n
  \n


Server Response:
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
Representation:
::

  TiA 1.0 \n
  StopDataTransmission \n
  \n

Server Response:
::

  TiA 1.0 \n
  OK \n
  \n

or

::

  TiA 1.0 \n
  Error \n
  \n



TiA Meta Info
-------------
The TiA meta info is structured in XML and contains information about the signals and the subject.


TiA Error Description
---------------------
Error message in TiA version 1.0 optionally supports error descriptions in a human readable format. 
Therefore no error codes with special meaning are supported in this version.


.. TiA Server Config
.. -----------------
.. Die Server Config hat nichts mit den Meta Infos zu tun!!! Die Server config wird an die Hardware weitergereicht... TiA legt NICHT fest, wie diese config auszusehen hat!!!!!!

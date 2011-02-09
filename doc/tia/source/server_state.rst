Server State Connection
=======================

Additionally to the control- and data-connection a client may request a so called "server state connection".

This connection is used by the server to transmit messages about its state to the clients.

State Messages
--------------

* Server running
* Server shut down

Server Running
^^^^^^^^^^^^^^
Just to indicate that the server is running.

::
  
  TiA 1.0\n
  ServerStateRunning\n
  \n

Clients must no reply to this message.

Server Shut Down
^^^^^^^^^^^^^^^^
Indicates that the server will shut down soon. Therefore the control and the data connection will be closed by the server soon.

::
  
  TiA 1.0\n
  ServerStateShutdown\n
  \n

Clients must not reply to this message.
Server State Connection
=======================

Additionally to the control- and data-connection a client may request a so called "server state connection".

This connection is used by the server to transmit messages about its state to the clients.


State Messages
--------------

* Server shut down

Server Shut Down
^^^^^^^^^^^^^^^^

::
  
  TiA 1.0\n
  ServerShutdown\n
  \n
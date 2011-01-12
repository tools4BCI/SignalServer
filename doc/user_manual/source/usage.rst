Using the Signal Server
=======================

Generic Usage
^^^^^^^^^^^^^

The Signal Server is shipped with two demo configuration files ("server_config_comments.xml" and
"server_config.xml"). Those files are showing exemplary
configurations of the Signal Server. The "_commets.xml" file is equipped with additional
comments to facilitate understanding the configuration of the Signal Server.

The Signal Server can be started by a quickstart, where the file "server_config.xml" is automatically
read (operating system dependent) or by passing an individual config file to the Signal Server.


**Start Commands for individual config files:**
::
  signalserver your_config_file.xml
o
r
::
  signalserver -f your_config_file.xml

**Commands while the Signal Server is running:**

q ... stop

r ... restart (problems with various DAQ devices -- bugfix in progress)

Debian/Ubuntu
^^^^^^^^^^^^^

With the first start of the Signal Server, a folder named "tobi_sigserver_cfg" is automatically
created within the users home folder. Two files named "server_config_comments.xml" and
"server_config.xml" are located inside this folder.

Starting the Signal Server without any parameters automatically reads and starts the "server_config.xml"
inside the "tobi_sigserver_cfg" folder (quickstart).

Windows
^^^^^^^

Starting the Signal Server without any parameters (quickstart) the program automatically reads and
starts the "server_config.xml" inside the folder where the Signal Server is installed
(e.g. C:\Program Files\TOBI SignalServer\server_config.xml).

To use a different configuration, an other .xml file can be handed to the Signal Server
either using the commandline or just dragging the respective file on the Signal Server executable
or a link to it.


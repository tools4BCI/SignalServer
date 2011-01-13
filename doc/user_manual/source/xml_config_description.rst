The XML Configuration File -- Basics
====================================

This is just a general description of the XML configuration file. Detailed description for
particular hardware devices is described elsewhere.

Subject Information
^^^^^^^^^^^^^^^^^^^

This tag has to be used to store meta information from the respective subject or patient
participating in the measurement.
(Self-explanatory tags are not described here, e.g. birthday)

*Exactly one <subject> tag has to be inside the XML configuration file.*

* ID

  The subjects identification code (e.g. ch17b)

* Optional

  The optional tag can be extended at will (e.g. technician = "t1")

Server Settings
^^^^^^^^^^^^^^^

Inside this tag the Signal Server except it's attached hardware is configured.

*Exactly one <server_settings> tag has to be inside the XML configuration file.*

* ctl_port

  A TCP port every client is connected with. This port has to be specified at the client,
  (together with the IP address the server is running on; if the same machine is used, the IP
  address is 127.0.0.1) when connecting to the Signal Server.

The Signal Server supports data tranmission via UDP (reduced packet overhead, no guarantee that
all data is tranmsitted to the client). If a client requests UDP transmission UDP packets are
broadcasted into the whole subnet specified inside this tag.

Sending UDP packets to an address in an other subnet or two different ports is not supported yet.

* udp_port

  The target port.

* udp_bc_addr

  The broadcast address to transmit UDP packets to (e.g. 192.168.1. **255** -- packets are sent
  to every computer with an IP 192.168.1.XXX ).


The Hardware sections
^^^^^^^^^^^^^^^^^^^^^

The Signal Server supports data acquision from multiple devices at the same time. For this reason every
device has it's own hardware tag. Thus more than one hardware section is allowed inside the XML
configuration file.

Mode
----

Possible values are:

* Master


* Slave


* Aperiodic

  e.g. buttons; data is only delivered if a former value is altered


Inside the XML configuration file **exactly one device** has to be defined as master, all others
have to be slave or aperiodic devices.


Not all devices support master, slave and aperiodic mode.


Device Settings
---------------

In this tag settings affecting the whole data acquisition device and not only particular channels
are specified.

The simplest data acquisition device has at least a sampling rate, a blocksize and a certain
number of channels.

* Sampling Rate

  The sampling rate data is acquired with.

* Blocksize

  The number of samples grouped together before transmission.

* Measurement Channels

  *(This setting can be used for quick configuration; all channels get the same name and the same
  signal type. For individual channel names and signal types use the channel setting section.)*

  * nr

    The number of channels to acquire, starting at channel 1.

  * names

    The name for **all** channels.

  * type

    The signal type for **all** channels.


Samples are grouped into blocks from the same channel if a blocksize >1 is used (e.g. block size =
2: ch1s1 ch1s2; ch2s1 ch2s2; ...) and transmitted inside the same data packet. Altering the blocksize
does not affect the sampling rate itself, but the rate data packets are transmitted over the network.

Using a sampling rate of 1000 Hz and a blocksize of 10, samples are still acquired with 1000 Hz,
but data packets are sent with only 100 Hz, whereby every packets stores 10 samples for all acquired channels.

As incomming packets are used for timing control at the client, a bigger blocksize introduces a certain
jitter, as the client has to wait for a new packet storing more than one sample and processes all samples
immediately afterwards.

*Some hardware devices have to be used with a blocksize >1 to avoid data acquisition errors!*


Channel Settings
----------------

The channel settings tag can be used to customize individual channels and override settings done
in the device settings section.

* Selection

  Select only specific channels for recording. The sum of all channels here can be different from
  the settings done in measurement_channels in device_settings. These settings here override the prior
  channel selection!

  ``<ch nr="01" name="Cz" type="eeg" />``
  ``<ch nr="02" name="C3" type="eeg" />``

  * nr

    Number of a channel to acquire data from.

  * name

    The name for the respective channel (e.g. Cz).

  * type

    The signal type for the respective channel (e.g. eeg).



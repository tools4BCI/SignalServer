The XML Configuration File -- Basics
====================================

This is just a general description of the XML configuration file.

Subject Information
^^^^^^^^^^^^^^^^^^^

This tag has to be used to store meta information from the respective subject or patient
participating in the measurement.
(Self-explanatory tags are not described here, e.g. birthday)

*Exactly one <subject> tag has to be inside the XML configuration file.*

  ..  code-block:: xml
      :linenos:

      <subject>
        <id> asd52 </id>
        <first_name> Nobody </first_name>
        <surname> Nowhereman </surname>
        <sex> m </sex>
        <birthday> 31.12.1900 </birthday>
        <handedness> r </handedness>
        <medication> none </medication>

        <optional glasses="yes" smoker="no" />
      </subject>


* ID

  The subjects identification code (e.g. ch17b)

* Optional

  The optional tag can be extended at will (e.g. technician = "tec1")

Server Settings
^^^^^^^^^^^^^^^

Inside this tag the Signal Server except its attached hardware is configured.

*Exactly one <server_settings> tag has to be inside the XML configuration file.*

  ..  code-block:: xml
      :linenos:

      <server_settings>
        <ctl_port> 9000 </ctl_port>
        <udp_port> 9998 </udp_port>
        <udp_bc_addr> 127.0.0.255 </udp_bc_addr>

        <tid_port> 9100 </tid_port>

        <store-data>
          <filepath> way_to_files </filepath>
          <filename> hugo.gdf </filename>
          <filetype> gdf </filetype>
          <overwrite> yes </overwrite>
        </store-data>
      </server_settings>

* ctl_port

  A TCP port every client is connected to. This port has to be specified at the client
  (together with the IP address the server is running on; if the same machine is used, the IP
  address is 127.0.0.1) when connecting to the Signal Server.

UDP Data tranmission
--------------------

The Signal Server supports data tranmission via UDP (reduced packet overhead, no guarantee that
all data is tranmsitted to the client). If a client requests UDP transmission UDP packets are
broadcasted into the whole subnet specified inside this tag.

Sending UDP packets to an address in an other subnet or two different ports is not supported yet.

* udp_port

  The target port.

* udp_bc_addr

  The broadcast address to transmit UDP packets to (e.g. 192.168.1. **255** -- packets are sent
  to every computer with an IP 192.168.1.XXX ).


TiD Server
----------

* tid_port

The SignalServer support event dispatching using TiD messages. Therfore a TCP port,
TiD clients can connect to has to be given.

Saving recorded signals
-----------------------

The SignalServer supports saving data. Currently only .gdf files can be produced. Furthermore
data saving will be started together with the SignalServer start and ends when the SignalServer
is stopped.
Additional control possibilities will be added soon.

  
The Hardware sections
^^^^^^^^^^^^^^^^^^^^^

The Signal Server supports data acquision from multiple devices at the same time. For this reason every
device has its own hardware tag. Thus more than one hardware section is allowed inside the XML
configuration file.

  ..  code-block:: xml
      :linenos:

      <hardware name="sinegenerator" version="1.0" serial="">
        <mode> master </mode>
        <device_settings>
          <sampling_rate> 512 </sampling_rate>
          <measurement_channels nr="1" names="eeg" type="eeg" />
          <blocksize> 8 </blocksize>
        </device_settings>

      <channel_settings>
          <selection>
            <ch nr="01" name="C3" type="eeg" />
            <ch nr="02" name="Cz" type="eeg" />
            <ch nr="03" name="Hand" type="emg" />
          </selection>
        </channel_settings>
      </hardware>

Hardware
--------

  ``<hardware name="sinegenerator" version="1.0" serial="">``

* Name

  This attribute defines the respective hardware device to acquire data from.

* Version

  Not used yet. (May be removed in the future.)

* Serial

  Serial number of the device if available. Processed at particular devices (e.g. g.USBamp).

Mode
----

Possible values are:

* Master

* Slave


* Aperiodic

  e.g. buttons; data is only delivered if a value is altered


Inside the XML configuration file **exactly one device** has to be defined as master, all others
have to be slave or aperiodic devices. The master device **must** have the highest "virtual" sampling
rate compared to possible slave devices.

Virtual sampling rate = sampling rate / blocksize (e.g. fs = 512 Hz, bs = 8  ...  v_fs = 64 Hz)

Data is acquired from the master in blocking mode. Every time data is available at the master,
the latest data is acquired from all slaves and aperiodic devices. There is **no software synchronization**
of the acquired data!


Not all devices support master, slave, and aperiodic mode.


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
but data packets are sent with only 100 Hz, whereby every packet stores 10 samples for all acquired channels.

As incoming packets are used for timing control at the client, a bigger blocksize introduces a certain
jitter, as the client has to wait for a new packet storing more than one sample and processes all samples
immediately afterwards.

*Some hardware devices have to be used with a blocksize >1 to avoid data acquisition errors!*


Channel Settings
----------------

The channel settings tag can be used to customize individual channels and override settings done
in the device settings section.

* Selection

  Select only specific channels for recording. The sum of all channels here can be different from
  the settings done in measurement_channels in device_settings. Settings here override the prior
  channel selection!

  ..  code-block:: xml

      <ch nr="01" name="Cz" type="eeg" />

      <ch nr="02" name="C3" type="eeg" />

  * nr

    Number of a channel to acquire data from.

  * name

    The name for the respective channel (e.g. Cz).

  * type

    The signal type for the respective channel (e.g. eeg).



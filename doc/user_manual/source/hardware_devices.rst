Specific Hardware Section Configuration with the XML Config File
================================================================

**Before using some hardware with the Signal Server, please read the manufacturers user manual!**


Sine Generator
^^^^^^^^^^^^^^

The Sine Generator is a hardware emulator creating a 1 Hz sine with an amplitude of 1.
The phase is increased every 4 channels.

Only the number of channels, signal types, signal names, the sampling rate and the blocksize
are customizeable.
::
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


EEG Simulator
^^^^^^^^^^^^^

The EEG Simulator is a hardware emulator generating normal distributed random noise with
variable amplitude and offset. It is furthermore possible to add a sine to the simulated EEG
signal (e.g. to test a classifier).

An external control program to modify the EEG simulators parameters is currently in development
(a network protocol will be used for this purpose and the "port" tag defines the used port; it is not
evaluated yet).
::
  <hardware name="eegsim" version="1.0" serial="">
    <mode> master </mode>
    <device_settings>
      <sampling_rate> 500 </sampling_rate>
      <measurement_channels nr="48" names="eeg_name" type="eeg" />
      <blocksize> 1 </blocksize>
      <port> 9123 </port>

      <eeg_config  scaling="1" offset="1" />
      <sine_config frequ="2" amplitude="1"  phase="0" />
    </device_settings>

     <channel_settings>
      <selection>
        <ch nr="01" name="eeg" type="eeg" />
        <ch nr="02" name="eeg" type="eeg" />
        <ch nr="03" name="eeg" type="eeg" />
      </selection>

      <eeg_config>
        <ch nr="01" scaling="1" offset="-500" />
        <ch nr="02" scaling="1" offset="0" />
        <ch nr="03" scaling="1" offset="0" />
      </eeg_config>

      <sine_config>-->
        <!--WARNING: device settings are not overwritten here!
            sines are added to the signal; multiple entries per channel possible-->
        <ch nr="01" frequ="2" amplitude="100"  phase="0" />
        <ch nr="01" frequ="4" amplitude="1000"  phase="0" />
        <ch nr="02" frequ="3" amplitude="100"  phase="0" />
        <ch nr="03" frequ="6" amplitude="100"  phase="0" />
      </sine_config>

    </channel_settings>
  </hardware>


g.USBamp
^^^^^^^^

The g.USBamp is a mulipurpose biosignal DAQ device produced by g.tec (Guger Technologies, Graz, Austria).
Up to now only the Windows API is included into the Signal Server. The Linux API is planned to be
implemented soon.

**Important**

Using a too small blocksize for data acquisition might result in a loss of data.
The following values are recomendations from g.tec:

..
  +----------------------+----+----+-----+-----+-----+-----+------+------+------+------+-------+-------+
  | Sampling Rate \[Hz\] | 32 | 64 | 128 | 256 | 512 | 600 | 1200 | 2400 | 4800 | 9600 | 19200 | 38400 |
  +======================+====+====+=====+=====+=====+=====+======+======+======+======+=======+=======+
  | Block Size           |  1 |  2 |  4  |   8 |  16 |  32 |   64 |  128 |  256 |  512 |   512 |   512 |
  +----------------------+----+----+-----+-----+-----+-----+------+------+------+------+-------+-------+

====================  ==========
Sampling Rate \[Hz\]  Block Size
====================  ==========
      32                1
      64                2
      128               4
      256               8
      512               16
      600               32
      1200              64
      2400              128
      4800              256
      9600              512
      19200             512
      38400             512
====================  ==========

According to g.tec, the minimum buffersize can also be determined by following equation:

block_size >= sampling_rate * 0.06

Data acquisition has already been succesfully performed with a sampling rate of 512 Hz and a blocksize
of 4.

**Notice: You can use  blocksize smaller than the recommeded values on your own risk!**

 (If doing so, it is recommeded to perform extensive tests before.)

**Notice: g.USBamp running as slave without external sync is not supported yet! (4.2.2011)**

Possible messages, warnings and known issues:
---------------------------------------------

* Received not enough data:

  Starting the signal server with a connected g.UABamp, sometimes this message occurs for the first sample(s).
  This could happen, because the amplifier returns a not completely filled buffer, especially at startup.
  If this message occurs during runtime, often the signal servers process priority is too low or the used
  blocksize is too small.
  Missing values are filled up with "0".

* Timeout:

  If timeouts occur, please restart the g.USBamp and check the sync-cables, if multiple amps are used.
  (Due to a already fixed bug, this happened frequently if the signal server was stopped and started again.
  Starting and stopping the signal server a second time solved the problem.)

* Unable to set filter settings:

  Setting wrong filter settings or also a wrong sampling rate (e.g. 500 Hz) produces errors setting the
  hardware filter. Please re-check your settings if the desired filter is really supported.


Description of g.USBamp specific configuration tags:
----------------------------------------------------

The g.USBamp provides adjustable online filtering and other features. The following sections
explains which configuration settings can be done.
::
  <hardware name="g.usbamp" version="" serial="UA-2008.06.42">
    <mode> master </mode>
    <device_settings>
      <sampling_rate> 512 </sampling_rate>
      <measurement_channels nr="1" names="eeg" type="eeg" />
      <blocksize> 8 </blocksize>

      <filter type="chebyshev" order="8" f_low="0.5" f_high="100"/>
      <notch f_center="50"/>

      <shortcut> off </shortcut>

      <usbamp_master> yes </usbamp_master>

      <common_ground>
        <gnd block="a" value="1" />
        <gnd block="b" value="1" />
        <gnd block="c" value="1" />
        <gnd block="d" value="1" />
      </common_ground>

      <common_reference>
        <cr block="a" value="1" />
        <cr block="b" value="1" />
        <cr block="c" value="1" />
        <cr block="d" value="1" />
      </common_reference>
    </device_settings>

    <channel_settings>
      <selection>
        <ch nr="01" name="C3" type="eeg" />
        <ch nr="05" name="Cz" type="eeg" />
        <ch nr="06" name="ecg" type="ecg" />
        <ch nr="16" name="eyes" type="eog" />
      </selection>

      <filter>
        <ch nr="06" type="chebyshev" order="8" f_low="0.5" f_high="30"/>
        <ch nr="16" type="chebyshev" order="8" f_low="0.5" f_high="60"/>
      </filter>

      <notch>
        <ch nr="05" f_center="50"/>
        <ch nr="16" f_center="60"/>
      </notch>

      <bipolar>
        <ch nr="1" with="05" />
      </bipolar>

    </channel_settings>
  </hardware>


Every g.USBamp is equipped with a unique serial number. The respective device used for
acquisition is specified via its serial in the "serial" tag (here: UA-2008.06.42).

  ``<hardware name="g.usbamp" version="" serial="UA-2008.06.42">``

====
**Device Settings**
====

The g.USBamp has the possibilty to use different built in filters for pre-signal processing.
Possible filter settings are listed in a supplementary file called "g.USBamp_filter_settings.txt"
or can also be listed with the program "list_usbamp_filter_settings.exe".
(g.USBamp driver version 3.10.0 -- only chebyshev filters are suported yet by the amplifier)

  ``<filter type="chebyshev" order="8" f_low="0.5" f_high="100"/>``

====

The g.USBamp has the possibilty to use a hardware notch filter at 50 or 60 Hz.

  ``<notch f_center="50"/>``

====

A TTL high impulse on the SC input socket can be used to disconnect all electrode input sockets from
the input amplifiers and to connect the inputs to ground potential. (copied from the g.USBamp manual)

Turning this setting on or off enables or disables the SC socket to react on incomming TTL signals.

  ``<shortcut> off </shortcut>``

====

It is possible to use multiple g.USBamps at the same time. Those ampliefiers have to be connected
via an external sync cable (see g.USBamp manual).
One of all linked amplifiers has to provide the sync signal and act as the master device for
the other USBamps (this master has nothing to do with the master defined in every hardware tag).
For the amplifiers getting their sync signal via the "SYNC IN" socket, the <usbamp_master> tag
has to be set to "no".

  ``<usbamp_master> yes </usbamp_master>``

====

The g.USBamp has 4 channel groups and every group has its own ground connector.
It is possible to connect those grounds to a common ground via this tag by setting "value" to 1 or 0.
::
  <common_ground>
    <gnd block="a" value="1" />
    <gnd block="b" value="1" />
    <gnd block="c" value="1" />
    <gnd block="d" value="1" />
  </common_ground>

====

The g.USBamp has 4 channel groups and every group has its own reference connector.
It is possible to connect those references to a common reference via this tag by setting "value" to 1 or 0.
::
  <common_reference>
    <cr block="a" value="1" />
    <cr block="b" value="1" />
    <cr block="c" value="1" />
    <cr block="d" value="1" />
  </common_reference>

====
**Channel Settings**
====

The g.USBamp provides the possibility to set a filter for every channel individually. This setting only
overrides the global setting for the respective channel. All others remain with the global configuration.
::
  <filter>
    <ch nr="06" type="chebyshev" order="8" f_low="0.5" f_high="30"/>
  </filter>

====

The g.USBamp provides the possibility to set a notch filter for every channel individually. This setting only
overrides the global setting for the respective channel. All others remain with the global configuration.
::
  <notch>
    <ch nr="05" f_center="50"/>
  </notch>

====

The g.USBamp provides the possibility to acquire a bipolar channel combination. The channel given with the
attribute "with" is subtracted from the channel given by "nr". The Signal Server still delivers both channels.
An option to supress the channel defined by "with" is planned.
::
  <bipolar>
    <ch nr="1" with="05" />
  </bipolar>

g.Mobilab
^^^^^^^^^

The g.Mobilab and the g.Mobilab+ are portable mulipurpose biosignal DAQ systems produced by g.tec (Guger Technologies, Graz, Austria).
Both are available in two configurations, either able to acquire just EEG or also other biosgnals as EOG and ECG.
The g.Mobilab+ is can be connected via Bluetooth emulating a serial port.

Up to now only the g.Mobilab has been tested, but the g.Mobilab+ should work as well as the API is the same.
::
 <hardware name="g.mobilab" version="1.0" serial="">
    <mode> master </mode>
    <device_settings>
      <serial_port> /dev/ttyS0 </serial_port>
      <mobilab_type> eeg </mobilab_type>
      <measurement_channels nr="1" names="eeg" type="eeg" />
      <blocksize> 1 </blocksize>
    </device_settings>

    <channel_settings>
      <selection>
        <ch nr="01" name="eeg" type="eeg" />
        <ch nr="02" name="eeg" type="eeg" />
      </selection>
    </channel_settings>
  </hardware>

g.BSBamp
^^^^^^^^

The g.BSBamp is a mulipurpose biosignal DAQ device produced by g.tec (Guger Technologies, Graz, Austria).
Different variants are available. Device suitable for EEG, ECG, and EOG data acquisition or
devices able to acquire just one of those signal types.
The g.BSamp is acquired using National Instruments (Austin, TX, USA) DAQ cards.
::
 <hardware name="g.bsamp" version="2004" serial="BS-2004.08.02">
  <mode> master </mode>
  <device_settings>
    <sampling_rate> 512 </sampling_rate>
    <measurement_channels nr="1" names="eeg" type="eeg" />
    <blocksize> 1 </blocksize>
    <filter type="eeg" f_high="100" notch="on" f_low="2" sense="0.1" />
    <filter type="eog" f_high="100" notch="on" f_low="2" sense="1" />
    <filter type="emg" f_high="100" notch="on" f_low="2" sense="5" />
    <filter type="ecg" f_high="100" notch="on" f_low="2" sense="5" />
    <notch f_center="50"/>
  </device_settings>

  <channel_settings>
    <selection>
      <ch nr="01" name="eeg" type="eeg" />
      <ch nr="02" name="eeg" type="eeg" />
    </selection>

    <filter>
      <ch nr="2"  type="eeg" f_high="30"  notch="off" f_low="0.01" sense="0.05"/>
      <ch nr="15" type="ecg" f_high="100" notch="off" f_low="0.01" sense="5"/>
    </filter>
  </channel_settings>
 </hardware>


BrainAmp Series
^^^^^^^^^^^^^^^

BrainAmps are EEG acquisiton system produced by Brain Products (Gilching, Germany). All amplifiers
from the BrainAmp Series are supported.
::
  <hardware name="brainamp" version="1.0" serial="">
    <mode> master </mode>
    <device_settings>
      <sampling_rate> 500 </sampling_rate>
      <measurement_channels nr="2" names="eeg" type="eeg" />
      <blocksize> 5 </blocksize>

      <use_low_impedance>  no </use_low_impedance>
      <trigger_hold_value> 0 </trigger_hold_value>

      <lowpass_250Hz> off </lowpass_250Hz>
      <dc_coupling> off </dc_coupling>
      <resolution>  100nV </resolution>

      <calibration_mode on="no" signal="sine" freq="10" />
    </device_settings>

    <channel_settings>
      <selection>
        <ch nr="01" name="eeg" type="eeg" />
        <ch nr="02" name="eog" type="eog" />
        <ch nr="03" name="emg" type="emg" />
        <ch nr="04" name="emg"  type="emg" />
      </selection>

      <lowpass_250Hz>
        <ch nr="1" value="on"/>
        <ch nr="16" value="off"/>
      </lowpass_250Hz>

      <dc_coupling>
        <ch nr="1" value="on"/>
        <ch nr="16" value="off"/>
      </dc_coupling>

      <resolution>
        <ch nr="1" value="100nV"/>
        <ch nr="16" value="152muV"/>
      </resolution>

    </channel_settings>
  </hardware>



Generic Joysticks
^^^^^^^^^^^^^^^^^

It is possible to acquire data from attached joysticks with the Signal Server using the SDL library
(simple direct media layer) in Windows and Linux. Up to now only aperiodic mode is supported.
It is not possible to do any configuration for the joystick. The number of bottons, axes, and balls
is automatically determined by the Signal Server.
::
  <hardware name="joystick" version="1.0" serial="">
    <mode> aperiodic </mode>
    <device_settings>

    </device_settings>
  </hardware>



IntegraMouse + Generic Mouses
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

It is possible to acquire data from attached mouses with the Signal Server under Windows and Linux
using libusb.
The mouse is detached from the operating system, thus the respective mouse does not control
the mouse cursor. The data is directly fed into the Signal Server.

The IntegraMouse (LifeTool, Linz, Austria) is also supported via this configuration.

Only aperiodic mode is supported yet.

Linux:
To configure the mouse device, the VendorID and the ProductID of the device which define it
uniquly are needed. On Linux one can find them by the command ``lsusb -v``.
Both IDs are written in the first line of the device-block. Further the right
usb-port needs to be stated. One can find it also there, it is named
``bEndpointAddress`` and is listed in the ``Interface Descriptor``
of the device-block. All numbers must be added decimally.

Windows:
When using a mouse device on a Windows system, there is an additional tool needed.
Therefore the ``devcon``-tool included in the Windows Driver Kit
(`WinDDK`_) has to be installed first. Further ``libusb-win32``
(`libusb-win32`_)  must be installed on the system. It is used to
decouple the used mouse device from the opterating system. The full
path to this tool must be stated.
When configuring the TOBI SignalServer for a mouse device, one need
to generate a ``mouse.inf`` file in the directory ``bin/libusb``.
Therefore the  ``INF Wizard`` (included in libusb-win32) can be used. The
right configuration for VendorID, ProductID and Usb-Port
(``bEndpointAddress`` in the ``Interface Descriptor`` of the device-block)
can be found using the ``Test (Win) Program`` (also included in libusb-win32).
All numbers must be added decimally.

The following example of a configuration includes the data for a Mouse.  The usb-port might vary, depending on the used port.

**Important**

Entered values have to be in **decimal** format! Therefore a hexadecimal representation
given by ``lsusb -v`` has to be converted to its decimal equivalent.
::
  <hardware name="mouse" version="1.0" serial="">
    <mode> aperiodic </mode>
    <device_settings>
      <vendorid> 1351 </vendorid>
      <productid> 4136 </productid>
      <usb_port> 130 </usb_port>

      <devcon_path> C:\WinDDK\7600.16385.1\tools\devcon\i386\devcon.exe </devcon_path>

    </device_settings>
  </hardware>


.. _WinDDK: http://msdn.microsoft.com/en-us/windows/hardware/gg487428
.. _libusb-win32: http://sourceforge.net/apps/trac/libusb-win32

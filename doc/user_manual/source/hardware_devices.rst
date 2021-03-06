Specific Hardware Section Configuration with the XML Config File
================================================================

**Before using some hardware with the Signal Server, please read the manufacturers user manual!**


Sine Generator
^^^^^^^^^^^^^^

The Sine Generator is a hardware emulator creating a 1 Hz sine with an amplitude of 1.
The phase is increased every 4 channels.

Only the number of channels, signal types, signal names, the sampling rate and the blocksize
are customizeable.

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


EEG Simulator
^^^^^^^^^^^^^

The EEG Simulator is a hardware emulator generating normal distributed random noise with
variable amplitude and offset. It is furthermore possible to add a sine to the simulated EEG
signal (e.g. to test a classifier).

An external control program to modify the EEG simulators parameters is currently in development
(a network protocol will be used for this purpose and the "port" tag defines the used port; it is not
evaluated yet).

  ..  code-block:: xml
      :linenos:

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

  ..  code-block:: xml
      :linenos:

      <hardware name="g.usbamp" version="" serial="UA-2008.06.42">
        <mode> master </mode>
        <device_settings>
          <sampling_rate> 512 </sampling_rate>
          <measurement_channels nr="1" names="eeg" type="eeg" />
          <blocksize> 8 </blocksize>

          <filter type="chebyshev" order="8" f_low="0.5" f_high="100"/>
          <notch f_center="50"/>

          <shortcut> off </shortcut>

          <trigger_line type="user1"> on </trigger_line>

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

  ..  code-block:: xml

      <hardware name="g.usbamp" version="" serial="UA-2008.06.42">

====
**Device Settings**
====

The g.USBamp has the possibilty to use different built in filters for pre-signal processing.
Possible filter settings are listed in a supplementary file called "g.USBamp_filter_settings.txt"
or can also be listed with the program "list_usbamp_filter_settings.exe".
(g.USBamp driver version 3.10.0 -- only chebyshev filters are suported yet by the amplifier)

  ..  code-block:: xml

      <filter type="chebyshev" order="8" f_low="0.5" f_high="100"/>

====

The g.USBamp has the possibilty to use a hardware notch filter at 50 or 60 Hz.

  ..  code-block:: xml

      <notch f_center="50"/>

====

A TTL high impulse on the SC input socket can be used to disconnect all electrode input sockets from
the input amplifiers and to connect the inputs to ground potential. (copied from the g.USBamp manual)

Turning this setting on or off enables or disables the SC socket to react on incomming TTL signals.

  ..  code-block:: xml

      <shortcut> off </shortcut>

====

Digital input > 250 mV (e.g. a trigger signal) can be recorded together with the acquired
data with the same sampling rate as the recorded biosignal. This feature can be enabled via
the trigger line, using the respective connectors at the back of the USBamp.
A channel with the given signal type of the trigger line is automatically added to the recorded signals.

The new channel, representing the trigger inputs, is binary coded (2^n):
  
  0 ... all trigger channels low
  
  1 ... channel 1 high
  
  2 ... channel 2 high
  
  3 ... channels 1 and 2 high
  
  4 ... channel 3 high
  
  etc.
  
  ..  code-block:: xml

      <trigger_line type="user1"> on </trigger_line>

====

It is possible to use multiple g.USBamps at the same time. Those ampliefiers have to be connected
via an external sync cable (see g.USBamp manual).
One of all linked amplifiers has to provide the sync signal and act as the master device for
the other USBamps (this master has nothing to do with the master defined in every hardware tag).
For the amplifiers getting their sync signal via the "SYNC IN" socket, the <usbamp_master> tag
has to be set to "no".

  ..  code-block:: xml

      <usbamp_master> yes </usbamp_master>

====

The g.USBamp has 4 channel groups and every group has its own ground connector.
It is possible to connect those grounds to a common ground via this tag by setting "value" to 1 or 0.

  ..  code-block:: xml

      <common_ground>
        <gnd block="a" value="1" />
        <gnd block="b" value="1" />
        <gnd block="c" value="1" />
        <gnd block="d" value="1" />
      </common_ground>

====

The g.USBamp has 4 channel groups and every group has its own reference connector.
It is possible to connect those references to a common reference via this tag by setting "value" to 1 or 0.

  ..  code-block:: xml

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

  ..  code-block:: xml

      <filter>
        <ch nr="06" type="chebyshev" order="8" f_low="0.5" f_high="30"/>
      </filter>

====

The g.USBamp provides the possibility to set a notch filter for every channel individually. This setting only
overrides the global setting for the respective channel. All others remain with the global configuration.

  ..  code-block:: xml

      <notch>
        <ch nr="05" f_center="50"/>
      </notch>

====

The g.USBamp provides the possibility to acquire a bipolar channel combination. The channel given with the
attribute "with" is subtracted from the channel given by "nr". The Signal Server still delivers both channels.
An option to supress the channel defined by "with" is planned.

  ..  code-block:: xml
      :linenos:

      <bipolar>
        <ch nr="1" with="05" />
      </bipolar>


bioPlux
^^^^^^^

The bioPlux is a biosignal (EMG, ECG) and sensor DAQ device produced by PLUX, Engenharia De Biosensores LDA. Up to now only a 32 bit Windows API is available.

  ..  code-block:: xml
      :linenos:

      <hardware name="bioplux" mac="00:07:80:4B:2C:A2" statoutput="0" version="1.0" serial="">
          <mode> master </mode>
          <device_settings>
            <sampling_rate> 1000 </sampling_rate>
            <blocksize> 10 </blocksize>
          </device_settings>

          <channel_settings>
            <selection>
              <ch nr="01" name="arm" type="emg" />
              <ch nr="02" name="leg" type="emg" />
              <ch nr="05" name="preasure" type="sensor" />
            </selection>
          </channel_settings>
      </hardware>


Every bioPlux device is identified with a unique mac address. The API also provides a software emulated test device that is identified with "Test" instead of the mac address.
If statoutput is provided and not "0", the module prints statistical information (samples lost, dropped, delayed, etc.) to the standard output.
When operating in Slave mode, data is read into a FIFO buffer to allow asynchroneous acquisition. In case of a buffer underrun, the last sample is repeated. In case of a buffer overrun, the oldest sample is dropped from the buffer. Lost samples are detected and linearly interpolated.

A buffersize tag in the device_settings can be used to set the size of the buffer in slave mode.
  .. code-block:: xml
     :linenos:

     <device_settings>
       ...
       <buffersize> 40 </buffersize>
       ...
     <device_settings>

While the digital output is not supported, the digital input is mapped to channel 9.
Channels 10, 11 and 12 provide debugging information (time, frame delay, buffer count).



g.Mobilab
^^^^^^^^^

The g.Mobilab and the g.Mobilab+ are portable mulipurpose biosignal DAQ systems produced by g.tec (Guger Technologies, Graz, Austria).
Both are available in two configurations, either able to acquire just EEG or also other biosgnals as EOG and ECG.
The g.Mobilab+ is can be connected via Bluetooth emulating a serial port.

Up to now only the g.Mobilab has been tested, but the g.Mobilab+ should work as well as the API is the same.

  ..  code-block:: xml
      :linenos:

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

  ..  code-block:: xml
      :linenos:

      <hardware name="g.bsamp" version="2004" serial="BS-2004.08.02">
        <mode> master </mode>
        <device_settings>
          <sampling_rate> 500 </sampling_rate>
          <device_label> Dev1 </device_label>
          <measurement_channels nr="16" names="eeg" type="eeg" />
          <blocksize> 1 </blocksize>

          <jumper type="eeg" lowpass="100" notch="on" highpass="2" sense="0.1" />
          <jumper type="emg" lowpass="100" notch="on" highpass="2" sense="5" />
          <jumper type="ecg" lowpass="100" notch="on" highpass="2" sense="5" />
          <jumper type="eog" lowpass="100" notch="on" highpass="2" sense="1" />

          <daq_mode> rse </daq_mode>
        </device_settings>

        <channel_settings>
        <selection>
          <ch nr="01" name="eeg" type="eeg" />
          <ch nr="02" name="eeg" type="eeg" />

            <ch nr="15" name="eeg" type="ecg" />
        </selection>

        <jumper>
          <ch nr="2"  type="eeg" lowpass="30"  notch="off" highpass="0.5" sense="0.05"/>
          <ch nr="15" type="ecg" lowpass="100" notch="off" highpass="0.01" sense="5"/>
        </jumper>
      </channel_settings>
      </hardware>


BrainAmp Series
^^^^^^^^^^^^^^^

BrainAmps are EEG acquisiton system produced by Brain Products (Gilching, Germany). All amplifiers
from the BrainAmp Series are supported.

  ..  code-block:: xml
      :linenos:

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
            <ch nr="4" value="off"/>
          </lowpass_250Hz>

          <dc_coupling>
            <ch nr="1" value="on"/>
            <ch nr="3" value="off"/>
          </dc_coupling>

          <resolution>
            <ch nr="1" value="100nV"/>
            <ch nr="4" value="152muV"/>
          </resolution>

        </channel_settings>
      </hardware>


Generic Joysticks
^^^^^^^^^^^^^^^^^

It is possible to acquire data from attached joysticks with the Signal Server using the SDL library
(simple direct media layer) in Windows and Linux. Up to now only aperiodic mode is supported.
It is not possible to do any configuration for the joystick. The number of bottons, axes, and balls
is automatically determined by the Signal Server.

  ..  code-block:: xml
      :linenos:

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
uniquly are needed. On Linux one can find them by the command ``lsusb -v`` (see also the example below).
Both IDs are written in the first line of the device-block. Further the right
usb-port needs to be stated. One can find it also there, it is named
``bEndpointAddress`` and is listed in the ``Interface Descriptor``
of the device-block.

Windows:
When using a mouse device on a Windows system, there is an additional tool needed.
Therefore the ``devcon``-tool included in the Windows Driver Kit
(`WinDDK`_) has to be installed first. Further the libusb-win32-"Filter Driver"-package (``libusb-win32-devel-filter-x.x.x.x.zip``)
(`libusb-win32`_)  must be installed on the system. It is used to
decouple the used mouse device from the opterating system. The full
path to this tool must be stated.
When configuring the TOBI SignalServer for a mouse device, one need
to generate a ``mouse.inf`` file in the directory ``bin/libusb``.
Therefore the  ``INF Wizard`` (included in libusb-win32-"Filter Driver"-package) can be used. The
right configuration for VendorID, ProductID and Usb-Port
(``bEndpointAddress`` in the ``Interface Descriptor`` of the device-block)
can be found using the ``Test (Win) Program`` (also included in libusb-win32-"Filter Driver"-package).

The following example of a configuration includes the data for a Mouse (the numbers refere to the IntegraMouse).  The usb-port might vary, depending on the used port.

**Important**

Entered values can be in decimal or hexadecimal format! If a hex-value is used, "0x" has to be
preceding to the actual value.

  ..  code-block:: xml
      :linenos:

      <hardware name="mouse" version="1.0" serial="">
        <mode> aperiodic </mode>
        <device_settings>
          <vendorid> 1351 </vendorid>
          <productid> 4136 </productid>
          <usb_port> 0x82 </usb_port>

          <devcon_path> C:\WinDDK\7600.16385.1\tools\devcon\i386\devcon.exe </devcon_path>
          <inf_file_path> C:\cool_path\fancy_mouse.inf </inf_file_path>

        </device_settings>
      </hardware>


Output from ``lsusb -v`` in Linux and from the ``libusb Test-Tool``,
the needed values are highlighted with "*":
::
  Bus 004 Device 002: ID 046d:c046 Logitech, Inc. RX1000 Laser Mouse
  Device Descriptor:
    bLength                18
    bDescriptorType         1
    bcdUSB               2.00
    bDeviceClass            0 (Defined at Interface level)
    bDeviceSubClass         0
    bDeviceProtocol         0
    bMaxPacketSize0         8

    ***** idVendor           0x046d Logitech, Inc. *****
    ***** idProduct          0xc046 RX1000 Laser Mouse *****

    bcdDevice           27.20
    iManufacturer           1 Logitech
    iProduct                2 USB Optical Mouse
    iSerial                 0
    bNumConfigurations      1
    Configuration Descriptor:
      bLength                 9
      bDescriptorType         2
      wTotalLength           34
      bNumInterfaces          1
      bConfigurationValue     1
      iConfiguration          0
      bmAttributes         0xa0
        (Bus Powered)
        Remote Wakeup
      MaxPower               98mA
      Interface Descriptor:
        bLength                 9
        bDescriptorType         4
        bInterfaceNumber        0
        bAlternateSetting       0
        bNumEndpoints           1
        bInterfaceClass         3 Human Interface Device
        bInterfaceSubClass      1 Boot Interface Subclass
        bInterfaceProtocol      2 Mouse
        iInterface              0
          HID Device Descriptor:
            bLength                 9
            bDescriptorType        33
            bcdHID               1.10
            bCountryCode            0 Not supported
            bNumDescriptors         1
            bDescriptorType        34 Report
            wDescriptorLength      59
          Report Descriptors:
            * UNAVAILABLE *
        Endpoint Descriptor:
          bLength                 7
          bDescriptorType         5
          
          ***** bEndpointAddress     0x81  EP 1 IN *****
          
          bmAttributes            3
            Transfer Type            Interrupt
            Synch Type               None
            Usage Type               Data
          wMaxPacketSize     0x0005  1x 5 bytes
          bInterval              10
    Device Status:     0x0000
      (Bus Powered)

.. _WinDDK: http://msdn.microsoft.com/en-us/windows/hardware/gg487428
.. _libusb-win32: http://sourceforge.net/apps/trac/libusb-win32


DataQ Instruments Devices
^^^^^^^^^^^^^^^^^^^^^^^^^

The SignalServer supports data acquisition from DataQ Instruments devices. Currently
only the DI-720-USB is supported, but can be extended to other devices with minimal effort.

**NOTICE**

The minimum burst rate (sampling rate * nr. of channels) is 500 Hz, and the maximum rate is 200000 Hz.
This values have to be met by setting the nr. of channels and the sampling rate properly.


  ..  code-block:: xml
      :linenos:

      <hardware name="di-720" version="1.0" serial="">
      <mode> master </mode>
      
      <device_settings>
        <sampling_rate> 1000 </sampling_rate>
        <measurement_channels nr="2" names="eeg" type="eeg" />
        <blocksize> 1 </blocksize>

        <range> 10 </range>
        <bipolar> on </bipolar>
      </device_settings>

      <channel_settings>
        <selection>
          <ch nr="01" name="ch1" type="eeg" />
          <ch nr="02" name="ch2" type="eeg" />
        </selection>
        
        <range>
          <ch nr="01" value="5"/>
          <ch nr="02" value="10"/>
        </range>

        <bipolar>
          <ch nr="01" value="on" />
          <ch nr="02" value="on" />
        </bipolar>
      </channel_settings>

      </hardware>


**Voltage Range**


DataQ devices support data acquisition of different voltage ranges. This range can be set either
for the whole device or also individually for single channels.

  Setting the voltage range for the whole device:
  
  ..  code-block:: xml

      <range> 10 </range>

    Possible values are 10, 5, 2.5 and 1.25 volts.


  Setting the voltage range for single channels (those channels have to be set for recording too!):
  
  ..  code-block:: xml

      <channel_settings>
        <range>
          <ch nr="01" value="5"/>
          <ch nr="02" value="10"/>
        </range>
      </channel_settings>


**Bipolar recording**


DataQ devices further allow bipolar recording. Without a bipolar recording, every input channel
is referenced against ground whereas in a bipolar case channel 1 is referenced to channel 9,
ch. 2 to ch. 10 ... and ch. 8 to ch. 16. Ongoing, using both input plugs of a DataQ device, 
ch. 17 is referenced to ch. 25 ... (based on the DataQ `DI-720 hardware manual`_, pages 23ff, 28.2.2012).


  Setting bipolar recording for the whole device:
  
  ..  code-block:: xml

      <bipolar> on </bipolar>

  Setting bipolar recording for single channels:
  
  (those channels have to be set for recording too,
  bipolar reference channels, e.g. ch. 9, must not be set for recording!)
  
  ..  code-block:: xml

      <channel_settings>
        <bipolar>
          <ch nr="01" value="on" />
          <ch nr="02" value="on" />
        </bipolar>
      </channel_settings>


.. _DI-720 hardware manual: http://www.dataq.com/support/documentation/pdf/manual_pdfs/720_730.pdf

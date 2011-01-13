Specific Hardware Section Configuration with the XML Config File
========================================================

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

An external control program to modify the EEG simulators parameters is currently in development.
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
::
  <hardware name="g.usbamp" version="1.4" serial="UA-2008.06.42">
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
        <ch nr="01" name="eeg" type="eeg" />
        <ch nr="02" name="eeg" type="eeg" />
        <ch nr="03" name="eeg" type="eeg" />
        <ch nr="04" name="eeg" type="eeg" />
        <ch nr="05" name="eeg" type="eeg" />
        <ch nr="06" name="eeg" type="eeg" />
        <ch nr="07" name="eeg" type="eeg" />
        <ch nr="08" name="eeg" type="eeg" />
        <ch nr="09" name="eeg" type="eeg" />
        <ch nr="10" name="eeg" type="eeg" />
        <ch nr="11" name="eeg" type="eeg" />
        <ch nr="12" name="eeg" type="eeg" />
        <ch nr="13" name="eeg" type="eeg" />
        <ch nr="14" name="eeg" type="eeg" />
        <ch nr="15" name="eeg" type="eeg" />
        <ch nr="16" name="eeg" type="eeg" />
      </selection>

      <filter>
        <ch nr="15" type="chebyshev" order="8" f_low="0.5" f_high="30"/>
        <ch nr="16" type="chebyshev" order="8" f_low="0.5" f_high="60"/>
      </filter>

      <notch>
        <ch nr="15" f_center="50"/>
        <ch nr="16" f_center="50"/>
      </notch>

      <bipolar>
        <ch nr="1" with="16" />
        <ch nr="2" with="3" />
      </bipolar>

    </channel_settings>
  </hardware>

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
from the BrainAmp Series are supported here.
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
        <ch nr="01" name="eog" type="eog" />
        <ch nr="01" name="emg" type="emg" />
        <ch nr="01" name="bp"  type="bp" />
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

**Insert some text.**
::
  <hardware name="joystick" version="1.0" serial="">
    <mode> aperiodic </mode>
    <device_settings>

    </device_settings>
  </hardware>



IntegraMouse + Generic Mouses
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Insert some text.**
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
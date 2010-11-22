/*
    This file is part of the TOBI signal server.

    The TOBI signal server is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI signal server is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI signal server.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christoph Eibel
    Contact: christoph.eibel@tugraz.at
*/

#ifndef GTEC_USBAMP_WRAPPER_H
#define GTEC_USBAMP_WRAPPER_H

#ifdef WIN32

#include <exception>
#include <iostream>

namespace tobiss
{

// FILTER
#define F_CHEBYSHEV		0
#define F_BUTTERWORTH	1
#define F_BESSEL		2

// MODES
#define M_NORMAL		0
#define	M_IMPEDANCE		1
#define	M_CALIBRATE		2
#define M_COUNTER		3

//WAVESHAPES
#define WS_SQUARE		0x01
#define WS_SAWTOOTH		0x02
#define WS_SINE			0x03
#define WS_DRL			0x04
#define WS_NOISE		0x05


#define HEADER_SIZE		38

	//_____________________________________________________________________________
//
//							DATA STRUCTURES
//_____________________________________________________________________________


typedef struct _DigitalIO
{
   BOOL	 DIN1; // True if set (high voltage level)
   BOOL  DIN2;
   BOOL  DOUT1;
   BOOL  DOUT2;
} DigitalIO, *PDigitalIO;

typedef struct _DigitalOUT
{
	BOOL SET_0; // True if digital OUT0 should be set to VALUE_0 ...
	BOOL VALUE_0;
	BOOL SET_1;
	BOOL VALUE_1;
	BOOL SET_2;
	BOOL VALUE_2;
	BOOL SET_3;
	BOOL VALUE_3;
}DigitalOUT, *PDigitalOUT;

typedef struct _GND
{
	BOOL GND1;
	BOOL GND2;
	BOOL GND3;
	BOOL GND4;
} GND, *PGND;

typedef struct _REF
{
	BOOL ref1;
	BOOL ref2;
	BOOL ref3;
	BOOL ref4;
} REF, *PREF;

typedef struct _DAC
{
	BYTE WaveShape;
	WORD Amplitude;
	WORD Frequency;
	WORD Offset;
} DAC, *PDAC;

typedef struct _CHANNEL
{
	UCHAR Channel1;
	UCHAR Channel2;
	UCHAR Channel3;
	UCHAR Channel4;
	UCHAR Channel5;
	UCHAR Channel6;
	UCHAR Channel7;
	UCHAR Channel8;
	UCHAR Channel9;
	UCHAR Channel10;
	UCHAR Channel11;
	UCHAR Channel12;
	UCHAR Channel13;
	UCHAR Channel14;
	UCHAR Channel15;
	UCHAR Channel16;
} CHANNEL, *PCHANNEL;

typedef struct _SCALE
{
	float factor[16];
	float offset[16];
} SCALE, *PSCALE;


typedef struct _GT_DEVICEINFO
{
	char deviceInfo[256];
} GT_DEVICEINFO, *PGT_DEVICEINFO;

typedef struct _FILT
{
	float fu;
	float fo;
	float fs;
	float type;
	float order;

} FILT, *PFILT;

typedef FLOAT (CALLBACK* GT_GetDriverVersionType)(void);
typedef BOOL (CALLBACK* GT_GetDataType)(HANDLE, BYTE*, DWORD, OVERLAPPED*);
typedef FLOAT (CALLBACK* GT_GetHWVersionType)(HANDLE);
typedef HANDLE (CALLBACK* GT_OpenDeviceType) (int);
typedef HANDLE (CALLBACK* GT_OpenDeviceExType) (LPSTR);
typedef BOOL (CALLBACK* GT_CloseDeviceType) (HANDLE*);
typedef BOOL (CALLBACK* GT_SetBufferSizeType) (HANDLE, WORD);
typedef BOOL (CALLBACK* GT_SetSampleRateType) (HANDLE, WORD);
typedef BOOL (CALLBACK* GT_StartType) (HANDLE);
typedef BOOL (CALLBACK* GT_StopType) (HANDLE);
typedef BOOL (CALLBACK* GT_SetChannelsType) (HANDLE, UCHAR*, UCHAR);
typedef BOOL (CALLBACK* GT_SetDigitalOutType) (HANDLE, UCHAR, UCHAR);
typedef BOOL (CALLBACK* GT_SetDigitalOutExType) (HANDLE, DigitalOUT);
typedef BOOL (CALLBACK* GT_GetDigitalIOType) (HANDLE , PDigitalIO );
typedef BOOL (CALLBACK* GT_GetDigitalOutType) (HANDLE , PDigitalOUT );
typedef BOOL (CALLBACK* GT_GetLastErrorType) (WORD * , char *);
typedef BOOL (CALLBACK* GT_ResetTransferType) (HANDLE );
typedef BOOL (CALLBACK* GT_GetSerialType) (HANDLE , LPSTR ,UINT );
typedef BOOL (CALLBACK* GT_VRType) (int , void **,int , void **);
typedef BOOL (CALLBACK* GT_EnableTriggerLineType) (HANDLE , BOOL );
typedef BOOL (CALLBACK* GT_GetImpedanceType) (HANDLE , UCHAR , double* );
typedef BOOL (CALLBACK* GT_CalibrateType) (HANDLE ,PSCALE );
typedef BOOL (CALLBACK* GT_SetScaleType) (HANDLE , PSCALE );
typedef BOOL (CALLBACK* GT_GetScaleType) (HANDLE , PSCALE );
typedef BOOL (CALLBACK* GT_GetFilterSpecType) (FILT *);
typedef BOOL (CALLBACK* GT_GetNumberOfFilterType) (int* );
typedef BOOL (CALLBACK* GT_SetBandPassType) (HANDLE , UCHAR , int );
typedef BOOL (CALLBACK* GT_GetNotchSpecType) (FILT *);
typedef BOOL (CALLBACK* GT_GetNumberOfNotchType) (int* );
typedef BOOL (CALLBACK* GT_SetNotchType) (HANDLE , UCHAR , int );
typedef BOOL (CALLBACK* GT_SetModeType) (HANDLE , UCHAR );
typedef BOOL (CALLBACK* GT_GetModeType) (HANDLE , UCHAR* );
typedef BOOL (CALLBACK* GT_SetGroundType) (HANDLE , GND );
typedef BOOL (CALLBACK* GT_GetGroundType) (HANDLE , GND* );
typedef BOOL (CALLBACK* GT_SetReferenceType) (HANDLE , REF );
typedef BOOL (CALLBACK* GT_GetReferenceType) (HANDLE , REF* );
typedef BOOL (CALLBACK* GT_SetBipolarType) (HANDLE , CHANNEL );
typedef BOOL (CALLBACK* GT_SetDRLChannelType) (HANDLE , CHANNEL );
typedef BOOL (CALLBACK* GT_EnableSCType) (HANDLE , BOOL );
typedef BOOL (CALLBACK* GT_SetSlaveType) (HANDLE , BOOL );

//---------------------------------
/// wrapper class
class GTECUSBampWrapper
{
public:
	GTECUSBampWrapper () : dll_handle_ (NULL)
	{
		dll_handle_ = LoadLibrary("gUSBamp.dll");
		if (dll_handle_ == NULL)
			throw std::runtime_error ("Can't load gUSBamp.dll");

		setupDLLFunction (get_driver_verstion_ptr_, "GT_GetDriverVersion");
		setupDLLFunction (get_data_ptr_, "GT_GetData");
		setupDLLFunction (get_hw_version_ptr_, "GT_GetHWVersion");
		setupDLLFunction (open_device_ptr_, "GT_OpenDevice");
		setupDLLFunction (open_device_ex_ptr_, "GT_OpenDeviceEx");
		setupDLLFunction (close_device_ptr_, "GT_CloseDevice");

		setupDLLFunction (set_sample_rate_ptr_, "GT_SetSampleRate");
		setupDLLFunction (start_ptr_, "GT_Start");
		setupDLLFunction (stop_ptr_, "GT_Stop");
		setupDLLFunction (set_channels_ptr_, "GT_SetChannels");
		setupDLLFunction (set_digital_out_ptr_, "GT_SetDigitalOut");
		setupDLLFunction (set_digital_out_ex_ptr_, "GT_SetDigitalOutEx");
		setupDLLFunction (get_digital_io_ptr_, "GT_GetDigitalIO");
		setupDLLFunction (get_digital_out_ptr_, "GT_GetDigitalOut");
		setupDLLFunction (get_last_error_ptr_, "GT_GetLastError");
		setupDLLFunction (reset_transfer_ptr_, "GT_ResetTransfer");
		setupDLLFunction (get_serial_ptr_, "GT_GetSerial");

		setupDLLFunction (vr_ptr_, "GT_VR");
		setupDLLFunction (enable_trigger_line_ptr_, "GT_EnableTriggerLine");
		setupDLLFunction (get_impedance_ptr_, "GT_GetImpedance");
		setupDLLFunction (calibrate_ptr_, "GT_Calibrate");
		setupDLLFunction (set_scale_ptr_, "GT_SetScale");
		setupDLLFunction (get_scale_ptr_, "GT_GetScale");
		setupDLLFunction (get_filter_spec_ptr_, "GT_GetFilterSpec");
		setupDLLFunction (get_number_of_filter_ptr_, "GT_GetNumberOfFilter");
		setupDLLFunction (set_band_pass_ptr_, "GT_SetBandPass");
		setupDLLFunction (get_notch_spec_ptr_, "GT_GetNotchSpec");

		setupDLLFunction (get_number_of_notch_ptr_, "GT_GetNumberOfNotch");
		setupDLLFunction (set_notch_ptr_, "GT_SetNotch");
		setupDLLFunction (set_mode_ptr_, "GT_SetMode");
		setupDLLFunction (get_mode_ptr_, "GT_GetMode");
		setupDLLFunction (set_ground_ptr_, "GT_SetGround");
		setupDLLFunction (get_ground_ptr_, "GT_GetGround");
		setupDLLFunction (set_reference_ptr_, "GT_SetReference");
		setupDLLFunction (get_reference_ptr_, "GT_GetReference");
		setupDLLFunction (set_bipolar_ptr_, "GT_SetBipolar");
		setupDLLFunction (set_drl_channel_ptr_, "GT_SetDRLChannel");
		setupDLLFunction (enable_sc_ptr_, "GT_EnableSC");
		setupDLLFunction (set_slave_ptr_, "GT_SetSlave");
	}

	~GTECUSBampWrapper ()
	{
		FreeLibrary (dll_handle_);
	}

	FLOAT getDriverVersion ()
	{
		return get_driver_verstion_ptr_ ();
	}

	BOOL getData (HANDLE hDevice, BYTE *pData, DWORD dwSzBuffer, OVERLAPPED *ov)
	{
		return get_data_ptr_ (hDevice, pData, dwSzBuffer, ov);
	}

	BOOL start (HANDLE hDevice)
	{
		return start_ptr_ (hDevice);
	}

	BOOL stop (HANDLE hDevice)
	{
		return stop_ptr_ (hDevice);
	}

	BOOL closeDevice (HANDLE* hDevice)
	{
		return close_device_ptr_ (hDevice);
	}

	BOOL setChannels(HANDLE hDevice, UCHAR *ucChannels, UCHAR ucSizeChannels)
	{
		return set_channels_ptr_ (hDevice, ucChannels, ucSizeChannels);
	}

	BOOL resetTransfer (HANDLE hDevice)
	{
		return reset_transfer_ptr_ (hDevice);
	}

	BOOL getLastError (WORD * wErrorCode, char *pLastError)
	{
		return get_last_error_ptr_ (wErrorCode, pLastError);
	}

	HANDLE openDeviceEx (LPSTR lpSerial)
	{
		return open_device_ex_ptr_ (lpSerial);
	}

	FLOAT getHWVersion (HANDLE hDevise)
	{
		return get_hw_version_ptr_ (hDevise);
	}

	BOOL setBufferSize(HANDLE hDevice, WORD wBufferSize)
	{
		return set_buffer_size_ptr_ (hDevice, wBufferSize);
	}
	BOOL setSampleRate(HANDLE hDevice, WORD wSampleRate)
	{
		return set_sample_rate_ptr_ (hDevice, wSampleRate);
	}
	BOOL setDigitalOut(HANDLE hDevice, UCHAR ucNumber, UCHAR ucValue)
	{
		return set_digital_out_ptr_ (hDevice, ucNumber, ucValue);
	}
	BOOL setDigitalOutEx(HANDLE hDevice, DigitalOUT dout)
	{
		return set_digital_out_ex_ptr_ (hDevice, dout);
	}
	BOOL getDigitalIO(HANDLE hDevice, PDigitalIO pDIO)
	{
		return get_digital_io_ptr_ (hDevice, pDIO);
	}
	BOOL getDigitalOut(HANDLE hDevice, PDigitalOUT pDOUT)
	{
		return get_digital_out_ptr_ (hDevice, pDOUT);
	}
	BOOL getSerial(HANDLE hDevice, LPSTR lpstrSerial,UINT uiSize)
	{
		return get_serial_ptr_ (hDevice, lpstrSerial, uiSize);
	}
	BOOL VR(int nargin, void *varargin[],int nargout, void *varargout[])
	{
		return vr_ptr_ (nargin, varargin, nargout, varargout);
	}
	BOOL enableTriggerLine(HANDLE hDevice, BOOL bEnable)
	{
		return enable_trigger_line_ptr_ (hDevice, bEnable);
	}
	BOOL getImpedance(HANDLE hDevice, UCHAR Channel, double* Impedance)
	{
		return get_impedance_ptr_ (hDevice, Channel, Impedance);
	}
	BOOL calibrate(HANDLE hDevice,PSCALE Scaling)
	{
		return calibrate_ptr_ (hDevice, Scaling);
	}

	//------------
	// filters
	BOOL setScale (HANDLE hDevice, PSCALE Scaling)
	{
		return set_scale_ptr_ (hDevice, Scaling);
	}
	BOOL getScale (HANDLE hDevice, PSCALE Scaling)
	{
		return get_scale_ptr_ (hDevice, Scaling);
	}
	BOOL getFilterSpec (FILT *FilterSpec)
	{
		return get_filter_spec_ptr_ (FilterSpec);
	}
	BOOL getNumberOfFilter (int* nof)
	{
		return get_number_of_filter_ptr_ (nof);
	}
	BOOL setBandPass (HANDLE hDevice, UCHAR ucChannel, int index)
	{
		return set_band_pass_ptr_ (hDevice, ucChannel, index);
	}
	BOOL getNotchSpec (FILT *FilterSpec)
	{
		return get_notch_spec_ptr_ (FilterSpec);
	}
	BOOL getNumberOfNotch (int *nof)
	{
		return get_number_of_notch_ptr_ (nof);
	}
	BOOL setNotch (HANDLE hDevice, UCHAR ucChannel, int index)
	{
		return set_notch_ptr_ (hDevice, ucChannel, index);
	}

	//--------------
	// mode
	BOOL setMode(HANDLE hDevice, UCHAR ucMode)
	{
		return set_mode_ptr_ (hDevice, ucMode);
	}

	BOOL getMode(HANDLE hDevice, UCHAR* ucMode)
	{
		return get_mode_ptr_ (hDevice, ucMode);
	}
	BOOL setGround(HANDLE hDevice, GND CommonGround)
	{
		return set_ground_ptr_ (hDevice, CommonGround);
	}
	BOOL getGround(HANDLE hDevice, GND* CommonGround)
	{
		return get_ground_ptr_ (hDevice, CommonGround);
	}
	BOOL setReference(HANDLE hDevice, REF CommonReference)
	{
		return set_reference_ptr_ (hDevice, CommonReference);
	}
	BOOL getReference(HANDLE hDevice, REF* CommonReference)
	{
		return get_reference_ptr_ (hDevice, CommonReference);
	}
	BOOL setBipolar(HANDLE hDevice, CHANNEL bipoChannel)
	{
		return set_bipolar_ptr_ (hDevice, bipoChannel);
	}
	BOOL setDRLChannel(HANDLE hDevice, CHANNEL drlChannel)
	{
		return set_drl_channel_ptr_ (hDevice, drlChannel);
	}
	BOOL enableSC(HANDLE hDevice, BOOL bEnable)
	{
		return enable_sc_ptr_ (hDevice, bEnable);
	}
	BOOL setSlave(HANDLE hDevice, BOOL bSlave)
	{
		return set_slave_ptr_ (hDevice, bSlave);
	}

private:
	template<typename T>
	void setupDLLFunction (T& pointer, std::string const& name)
	{
		pointer = (T)GetProcAddress (dll_handle_, name.c_str());
		if (pointer == NULL)
			throw std::runtime_error (std::string ("gUSBamp.dll: missing function ").append (name));
	}

	HINSTANCE dll_handle_;
	GT_GetDriverVersionType get_driver_verstion_ptr_;
	GT_GetDataType get_data_ptr_;
	GT_GetHWVersionType get_hw_version_ptr_;
	GT_OpenDeviceType open_device_ptr_;
	GT_OpenDeviceExType open_device_ex_ptr_;
	GT_CloseDeviceType close_device_ptr_;
	GT_SetBufferSizeType set_buffer_size_ptr_;
	GT_SetSampleRateType set_sample_rate_ptr_;
	GT_StartType start_ptr_;
	GT_StopType stop_ptr_;
	GT_SetChannelsType set_channels_ptr_;
	GT_SetDigitalOutType set_digital_out_ptr_;
	GT_SetDigitalOutExType set_digital_out_ex_ptr_;
	GT_GetDigitalIOType get_digital_io_ptr_;
	GT_GetDigitalOutType get_digital_out_ptr_;
	GT_GetLastErrorType get_last_error_ptr_;
	GT_ResetTransferType reset_transfer_ptr_;
	GT_GetSerialType get_serial_ptr_;
	GT_VRType vr_ptr_;
	GT_EnableTriggerLineType enable_trigger_line_ptr_;
	GT_GetImpedanceType get_impedance_ptr_;
	GT_CalibrateType calibrate_ptr_;
	GT_SetScaleType set_scale_ptr_;
	GT_GetScaleType get_scale_ptr_;
	GT_GetFilterSpecType get_filter_spec_ptr_;
	GT_GetNumberOfFilterType get_number_of_filter_ptr_;
	GT_SetBandPassType set_band_pass_ptr_;
	GT_GetNotchSpecType get_notch_spec_ptr_;
	GT_GetNumberOfNotchType get_number_of_notch_ptr_;
	GT_SetNotchType set_notch_ptr_;
	GT_SetModeType set_mode_ptr_;
	GT_GetModeType get_mode_ptr_;
	GT_SetGroundType set_ground_ptr_;
	GT_GetGroundType get_ground_ptr_;
	GT_SetReferenceType set_reference_ptr_;
	GT_GetReferenceType get_reference_ptr_;
	GT_SetBipolarType set_bipolar_ptr_;
	GT_SetDRLChannelType set_drl_channel_ptr_;
	GT_EnableSCType enable_sc_ptr_;
	GT_SetSlaveType set_slave_ptr_;
};

} // namespace tobiss

#endif // WIN32

#endif // GTEC_USBAMP_WRAPPER_H

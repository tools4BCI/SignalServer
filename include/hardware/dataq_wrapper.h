/*
    This file is part of the TOBI SignalServer.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU General Public License Usage
    Alternatively, this file may be used under the terms of the GNU
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file gpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/gpl.html.

    In case of GNU General Public License Usage ,the TOBI SignalServer
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI SignalServer. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: SignalServer@tobi-project.org
*/

/**
* @file gtec_usbamp_wrapper.h
**/

#ifndef DATAQ_WRAPPER_H
#define DATAQ_WRAPPER_H

#include <windows.h>
#include <exception>
#include <string>
#include <iostream>

#include "extern/include/dataq/hdrdef32.h"
namespace tobiss
{

#ifndef ACHN
#define ACHN 29
#endif

typedef struct tagCODASHDR
{
   unsigned short header[2];
   struct hdrcmn hdrc;
   struct chinfo cary[ACHN];
   unsigned short vbflags;
} CODASHDR;

typedef struct di_anin_struct {
   unsigned short chan; /* input channel. 0 to 15 */
   unsigned short diff; /* input config. 0 = single end, 1 = diff. */
   unsigned short gain; /* gain code. 0 to 3 */
   unsigned short unipolar; /* unipolar or bipolar. 0 = BI, 1 = UN */
} DI_ANIN_STRUCT;

typedef struct di_info_struct {
   unsigned short port; /* device port address */
   unsigned short buf_in_chn; /* device input channel */
   unsigned short buf_out_chn; /* device output channel */
   unsigned short sft_lvl; /* software interrupt level */
   unsigned short hrd_lvl; /* hardware interrupt level */
   short *buf_in_ptr; /* input buffer pointer */
   unsigned short buf_in_size; /* input buffer size in words */
   short *buf_out_ptr; /* output buffer pointer */
   unsigned short buf_out_size; /* output buffer size in words */
   char tsr_version[20]; /* TSR version & hardware model */
   char dsp_version[20]; /* DSP program version */
   char sdk_version[20]; /* SDK library version */
   unsigned long serial_no; /* PCB serial number */
   unsigned long last_cal; /* last calibration time in sec since 1-1-1970 */
   char board_id[10]; /* PCB model name */
   char pgh_pgl;  /* type of PGA 0 = pgh, 1 = pgl */
   char hrdwr_rev; /* ASCII char REV letter */
   char pal0_rev; /* ASCII char REV of PAL0 */
   char pal1_rev; /* ASCII char REV of PAL1 */
} DI_INFO_STRUCT;

typedef struct di_info_struct32 {
   unsigned short port; /* device port address */
   unsigned short buf_in_chn; /* device input channel */
   unsigned short buf_out_chn; /* device output channel */
   unsigned short sft_lvl; /* software interrupt level */
   unsigned short hrd_lvl; /* hardware interrupt level */
   unsigned short padding; /* for 4-byte alignment of following */
   short *buf_in_ptr; /* input buffer pointer */
   unsigned buf_in_size; /* input buffer size in words */
   short *buf_out_ptr; /* output buffer pointer */
   unsigned buf_out_size; /* output buffer size in words */
   char tsr_version[20]; /* TSR version & hardware model */
   char dsp_version[20]; /* DSP program version */
   char sdk_version[20]; /* SDK library version */
   unsigned long serial_no; /* PCB serial number */
   unsigned long last_cal; /* last calibration time in sec since 1-1-1970 */
   char board_id[10]; /* PCB model name */
   char pgh_pgl;  /* type of PGA 0 = pgh, 1 = pgl */
   char hrdwr_rev; /* ASCII char REV letter */
   char pal0_rev; /* ASCII char REV of PAL0 */
   char pal1_rev; /* ASCII char REV of PAL1 */
} DI_INFO_STRUCT32;

typedef struct di_mode_struct {
   unsigned short mode:4; /* mode 0 = off, 1 = triggered analog, 2 = digital */
   unsigned short hystx:4; /* hysteresis index for DI-500 */
   unsigned short scnx:8; /* scanlist index of DI-500 trig chan, 0 for first chan */
   unsigned short trig_level; /* trigger level */
   unsigned short trig_slope; /* trigger slope 0 = +, 1 = - */
   unsigned short trig_pre; /* pre trigger number fo samples before trigger */
   unsigned short trig_post; /* post trigger length of scan before trigger occurs */
} DI_MODE_STRUCT;

typedef struct di_inlist_struct {
   unsigned short chan; /* input channel. 0 to 15 */
   unsigned short diff; /* input config. 0 = single ended, 1 = diff */
   unsigned short gain; /* gain. 0 to 3 */
   unsigned short unipolar; /* unipolar or bipolar, 0 = BI, 1= UN */
   unsigned short dig_out_enable; /* 1 = digital output enable */
   unsigned short dig_out; /* D0 to D4 = data */
   unsigned short ave; /* sample averaging, 0 = off, 1 = on */
   unsigned short counter; /* scan position counter */
} DI_INLIST_STRUCT;

typedef struct di_outlist_struct {
   unsigned short unipolar; /* unipolar or bipolar. 0 = BI, 1 = UN */
   unsigned short digital; /* digital output from buffer = 1, analog output = 0 */
   unsigned short dig_out_enable; /* 1 = digital out enable with analog */
   unsigned short dig_data; /* D0 to D7 = data for output */
   unsigned short counter; /* scan position counter */
} DI_OUTLIST_STRUCT;


typedef int (PASCAL *FPDIINLIST)(struct di_inlist_struct *);
typedef int (PASCAL *FPDIBURSTRATE)(unsigned);
typedef unsigned (PASCAL *FPDIMODE)(struct di_mode_struct *);
typedef int (PASCAL *FPDICLOSE)(void);
typedef int (PASCAL *FPDIINFO)(struct di_info_struct *);
typedef int (PASCAL *FPDIBUFFERFREE)(unsigned);
typedef short* (PASCAL *FPDIBUFFERALLOC)(unsigned,unsigned);
typedef int (PASCAL *FPDISTARTSCAN)(void);
typedef int (PASCAL *FPDIOUTLIST)(struct di_outlist_struct *);
typedef int (PASCAL *FPDILISTLENGTH)(unsigned,unsigned);
typedef int (PASCAL *FPDISTOPSCAN)(void);
typedef int (PASCAL *FPDICOPYMUX)(short *);
typedef unsigned (PASCAL *FPDISETDATAMODE)(unsigned);
typedef int (PASCAL *FPDIOPEN)(unsigned);
typedef unsigned (PASCAL *FPDIBUFFERSTATUS)(unsigned);
typedef unsigned (PASCAL *FPDIDIGIN)(void);
typedef unsigned (PASCAL *FPDIDIGOUT)(unsigned);
typedef int (PASCAL *FPDIANIN)(struct di_anin_struct *);
typedef int (PASCAL *FPDIANOUT)(unsigned,unsigned);
typedef int (PASCAL *FPDISTRERR)(unsigned,char *);
typedef int (PASCAL *FPDICOM)(unsigned,unsigned);
typedef double (PASCAL *FPDIMAXIMUMRATE)(double);
typedef double (PASCAL *FPDISAMPLERATE)(double, int *, int *);

typedef unsigned (PASCAL *FPDIBUFFERSIZE)(void);
typedef int (PASCAL *FPDICOPYARRAY)(unsigned, short*, unsigned);
typedef int (PASCAL *FPDICOPYBUFFER)(unsigned, short*, unsigned);
typedef unsigned (PASCAL *FPDICOPYHEADER)(unsigned, CODASHDR*, unsigned, unsigned);
typedef CODASHDR* (PASCAL *FPDIGETACQHEADER)(void);
//typedef int (PASCAL *FPDISETPORTMODE)(unsigned);
typedef unsigned (PASCAL *FPDISTATUSREAD)(short*, unsigned);

//---------------------------------
/// wrapper class
class DataQWrapper
{
	public:
		DataQWrapper (std::string dll_filename)
			: dll_handle_ (NULL), dll_filename_(dll_filename)
		{

      size_t origsize = strlen(dll_filename.c_str() ) + 1;
      const size_t newsize = 128;
      size_t convertedChars = 0;
      wchar_t wcstring[newsize];
      mbstowcs_s(&convertedChars, wcstring, origsize, dll_filename.c_str(), _TRUNCATE);

      //dll_handle_ = LoadLibrary( (LPCWSTR)wcstring );
      dll_handle_ = LoadLibrary( dll_filename_.c_str( ) );
      if (dll_handle_ == NULL)
        throw std::runtime_error("Can't load " + dll_filename_ + "!");

      setupDLLFunction (fndi_inlist_ptr_,"di_inlist");
      setupDLLFunction (fndi_burst_rate_ptr_,"di_burst_rate");
      setupDLLFunction (fndi_mode_ptr_,"di_mode@4");
      setupDLLFunction (fndi_close_ptr_,"di_close");
      setupDLLFunction (fndi_info_ptr_,"di_info");
      setupDLLFunction (fndi_buffer_free_ptr_,"di_buffer_free");
      setupDLLFunction (fndi_buffer_alloc_ptr_,"di_buffer_alloc");
      setupDLLFunction (fndi_start_scan_ptr_,"di_start_scan");
      setupDLLFunction (fndi_outlist_ptr_,"di_outlist");
      setupDLLFunction (fndi_list_length_ptr_,"di_list_length");
      setupDLLFunction (fndi_stop_scan_ptr_,"di_stop_scan");
      setupDLLFunction (fndi_copy_mux_ptr_,"di_copy_mux");
      setupDLLFunction (fndi_set_data_mode_ptr_,"di_set_data_mode");
      setupDLLFunction (fndi_open_ptr_,"di_open");
      setupDLLFunction (fndi_buffer_status_ptr_,"di_buffer_status");
      setupDLLFunction (fndi_digin_ptr_,"di_digin");
      setupDLLFunction (fndi_digout_ptr_,"di_digout");
      setupDLLFunction (fndi_anin_ptr_,"di_anin");
      setupDLLFunction (fndi_anout_ptr_,"di_anout");
      setupDLLFunction (fndi_strerr_ptr_,"di_strerr");
      setupDLLFunction (fndi_com_ptr_,"di_com");
      setupDLLFunction (fndi_maximum_rate_ptr_,"di_maximum_rate");
      setupDLLFunction (fndi_sample_rate_ptr_,"di_sample_rate");

      setupDLLFunction (fndi_buffer_size_ptr_, "di_buffer_size");
      setupDLLFunction (fndi_copy_array_ptr_, "di_copy_array");
      setupDLLFunction (fndi_copy_buffer_ptr_, "di_copy_buffer");
      setupDLLFunction (fndi_copy_header_ptr_, "di_copy_header");
      setupDLLFunction (fndi_get_acq_header_ptr_, "di_get_acq_header");
      //setupDLLFunction (fndi_set_port_mode_ptr_, "di_set_port_mode");
      setupDLLFunction (fndi_status_read_ptr_, "di_status_read");

    }

    ~DataQWrapper ()
      { FreeLibrary (dll_handle_); }

    int di_inlist(struct di_inlist_struct *input_list)
      { return fndi_inlist_ptr_(input_list); }

    int di_burst_rate(unsigned rate)
      { return fndi_burst_rate_ptr_(rate); }

    unsigned di_mode(struct di_mode_struct *mode)
      { return fndi_mode_ptr_(mode); }

    int di_close(void)
      { return fndi_close_ptr_(); }

    int di_info(struct di_info_struct *info)
      { return fndi_info_ptr_(info); }

    int di_buffer_free(unsigned chn)
      { return fndi_buffer_free_ptr_(chn); }

    short* di_buffer_alloc(unsigned chn,unsigned size)
      { return fndi_buffer_alloc_ptr_(chn, size); }

    int di_start_scan(void)
      { return fndi_start_scan_ptr_(); }

    int di_outlist(struct di_outlist_struct *output_list)
      { return fndi_outlist_ptr_(output_list); }

    int di_list_length(unsigned in_length, unsigned out_length)
      { return fndi_list_length_ptr_(in_length, out_length); }

    int di_stop_scan(void)
      { return fndi_stop_scan_ptr_(); }

    int di_copy_mux(short *dest)
      { return fndi_copy_mux_ptr_(dest); }

    unsigned di_set_data_mode(unsigned datamode)
      { return fndi_set_data_mode_ptr_(datamode); }

    int di_open(unsigned devno)
      { return fndi_open_ptr_(devno); }

    unsigned di_buffer_status(unsigned chn)
      { return fndi_buffer_status_ptr_(chn); }

    unsigned di_digin(void)
      { return fndi_digin_ptr_(); }

    unsigned di_digout(unsigned i)
      { return fndi_digout_ptr_(i); }

    int di_anin(struct di_anin_struct *analog_input)
      { return fndi_anin_ptr_(analog_input); }

    int di_anout(unsigned dac_data, unsigned range)
      { return fndi_anout_ptr_(dac_data, range); }

    int di_strerr(unsigned err_code,char *err_str)
      { return fndi_strerr_ptr_(err_code, err_str); }

    int di_com(unsigned di_command, unsigned di_data)
      { return fndi_com_ptr_(di_command, di_data); }

    double di_maximum_rate(double max_burst_rate)
      { return fndi_maximum_rate_ptr_(max_burst_rate); }

    double di_sample_rate(double desired_rate, int *oversample, int *burst_div)
      { return fndi_sample_rate_ptr_(desired_rate, oversample, burst_div); }

    unsigned di_buffer_size(void)
      { return fndi_buffer_size_ptr_(); }

    int di_copy_array(unsigned buf_index, short *source, unsigned word_count)
      { return fndi_copy_array_ptr_(buf_index, source, word_count); }

    int di_copy_buffer(unsigned buf_index, short *dest,  unsigned word_count)
      { return fndi_copy_buffer_ptr_(buf_index, dest, word_count); }

    unsigned di_copy_header(unsigned hdr_index,  CODASHDR *dest,
                            unsigned byte_count, unsigned clear_bits)
      { return fndi_copy_header_ptr_(hdr_index, dest, byte_count, clear_bits); }

    CODASHDR* di_get_acq_header(void)
      { return fndi_get_acq_header_ptr_(); }

    //int di_set_port_mode(unsigned mode)
    //  { return fndi_set_port_mode_ptr_(mode); }

    unsigned di_status_read(short *dest, unsigned num_scans)
      { return fndi_status_read_ptr_(dest, num_scans); }


	private:
		DataQWrapper () { }
		DataQWrapper (const DataQWrapper &) { }

		template<typename T> void setupDLLFunction (T& pointer, std::string const& name)
		{
			pointer = (T)GetProcAddress (dll_handle_, name.c_str());
			if (pointer == NULL)
				throw std::runtime_error (std::string ( dll_filename_ + " -- missing function: ").append (name));
		}

	private:
		HINSTANCE         dll_handle_;
		std::string   		dll_filename_;

		FPDIINLIST			  fndi_inlist_ptr_;
		FPDIBURSTRATE		  fndi_burst_rate_ptr_;
		FPDIMODE			    fndi_mode_ptr_;
		FPDICLOSE			    fndi_close_ptr_;
		FPDIINFO			    fndi_info_ptr_;
		FPDIBUFFERFREE		fndi_buffer_free_ptr_;
		FPDIBUFFERALLOC		fndi_buffer_alloc_ptr_;
		FPDISTARTSCAN		  fndi_start_scan_ptr_;
		FPDIOUTLIST			  fndi_outlist_ptr_;
		FPDILISTLENGTH		fndi_list_length_ptr_;
		FPDISTOPSCAN		  fndi_stop_scan_ptr_;
		FPDICOPYMUX			  fndi_copy_mux_ptr_;
		FPDISETDATAMODE		fndi_set_data_mode_ptr_;
		FPDIOPEN			    fndi_open_ptr_;
		FPDIBUFFERSTATUS	fndi_buffer_status_ptr_;
		FPDIDIGIN	    		fndi_digin_ptr_;
		FPDIDIGOUT		  	fndi_digout_ptr_;
		FPDIANIN          fndi_anin_ptr_;
		FPDIANOUT			    fndi_anout_ptr_;
		FPDISTRERR			  fndi_strerr_ptr_;
		FPDICOM				    fndi_com_ptr_;
		FPDIMAXIMUMRATE		fndi_maximum_rate_ptr_;
		FPDISAMPLERATE		fndi_sample_rate_ptr_;

    // functions taken from http://www.dataq.com/sdk/
    FPDIBUFFERSIZE    fndi_buffer_size_ptr_;
    FPDICOPYARRAY     fndi_copy_array_ptr_;
    FPDICOPYBUFFER    fndi_copy_buffer_ptr_;
    FPDICOPYHEADER    fndi_copy_header_ptr_;
    FPDIGETACQHEADER  fndi_get_acq_header_ptr_;
    //FPDISETPORTMODE   fndi_set_port_mode_ptr_;
    FPDISTATUSREAD    fndi_status_read_ptr_;

};

} // namespace tobiss

#endif // DATAQ

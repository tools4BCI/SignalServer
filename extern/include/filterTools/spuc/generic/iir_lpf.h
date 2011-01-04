// 
// Copyright(c) 1993-1996 Tony Kirke
// author="Tony Kirke" *
/*
 * SPUC - Signal processing using C++ - A DSP library
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef IIR_LPF
#define IIR_LPF
namespace SPUC {
//! \brief  Template Class for 1st Order low pass IIR filter from S-domain transformation
//! \ingroup iir
//
//!  Template for 1st Order IIR filter.<p>
//!   The filter is assumed the first order low pass digital filter 
//!   generated by the bilinear transform of a first order analog 
//!   filter.<p>
//!   \f$ H(s) = \frac{A}{(s+A)} \f$
//!   <p>\f$ H(z) = \frac{A*(1+z^{-1})}{((A-1)+(A-1)*z^{-1})} \f$ where A is real
template <class Numeric> class iir_lpf
{
    protected:   
    	double gain;
		double a;
    	Numeric out;
        Numeric previous_out;
		Numeric previous_in;
        
    public:
        iir_lpf(double A=0) {
			gain = (1-A)/(A+1);
			a    = A/(A+1);
        	previous_in = previous_out = out = 0 ; }
		void set_coeff(double A) { gain=A;}
		//! Constructor reading coefficient from a file.
		iir_lpf(const char* file)
		  {
			FILE *iirf = fopen(file,"r"); 
			fscanf(iirf,"%lf",&gain);
			fclose(iirf);
			previous_in = previous_out = out = 0;
		  }             
		//! Print out coefficients
		void print() {
			cout << "IIR B0 = " << a << ", ";
			cout << "A0 = " << gain << "\n";
		}
		//! Input new sample and calculate output
		Numeric clock(Numeric input) {
		  // Shift previous outputs and calculate new output */
		  out = gain*previous_out + a*(input+previous_in);
		  previous_out = out;
		  previous_in = input;
		  return(out);
		}
		//! Reset
		void reset() {
		  previous_in = previous_out = out = 0;
		}
};                                               
} // namespace SPUC 
#endif

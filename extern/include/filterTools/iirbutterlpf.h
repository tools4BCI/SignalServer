#ifndef IIRBUTTERLPF_H
#define IIRBUTTERLPF_H

#include "BasicFilter.h"
#include <fstream>
#include <algorithm>
#include <stdexcept>

#include "spuc/generic/butterworth.h"

//----------------------------------------------------------------------------------

template<typename T> class IIRButterLpf : public BasicFilter<T>
{
  public:
    IIRButterLpf(double sampling_rate, double cutoff_freq,
                 double attenuation = 0.5, unsigned int order = 2, bool useZeroPhase = 0);
    //IIRButterLpf() {lpf_<T>(0,0,0)};

    virtual T clock(T sample );
    virtual void filter(std::vector<T>& in, std::vector<T>& out);

  private:
    double                   freq_;
    double                   fs_;
    double                   attenuation_;
    unsigned int             order_;
    bool                     zero_phase_;

    SPUC::butterworth<T>*       lpf_;

};

//----------------------------------------------------------------------------------

template<typename T> IIRButterLpf<T>::IIRButterLpf(double sampling_rate, double cutoff_freq,
  double attenuation, unsigned int order, bool useZeroPhase)
    : freq_(cutoff_freq), fs_(sampling_rate), attenuation_(attenuation),
      order_(order), zero_phase_(useZeroPhase)
{
    if(freq_ >= fs_)
      throw(std::invalid_argument("Filter freq must be lower than fs!"));

    if(!freq_)
      throw(std::invalid_argument("Filter freq must not be 0!"));

    if(!fs_)
      throw(std::invalid_argument("Fs must not be 0!"));

    if(attenuation_ < 0)
      throw(std::invalid_argument("Attenuation must be a value (in dB) >0!"));

    if(!order_)
      throw(std::invalid_argument("Order must not be 0!"));

    if(zero_phase_)
    {
      if(order%2 != 0)
        throw(std::invalid_argument("Using ZeroPhase filtering, order has to be an even number!"));

      lpf_ = new SPUC::butterworth<T>(cutoff_freq/sampling_rate, order/2, attenuation);
    }
    else
      lpf_ = new SPUC::butterworth<T>(cutoff_freq/sampling_rate, order, attenuation);
}

//----------------------------------------------------------------------------------

template<typename T> T IIRButterLpf<T>::clock(T sample )
{
  if(zero_phase_)
    throw(std::runtime_error("IIRButterLpf::clock -- Zero phase not possible using clock!"));

  return(lpf_->clock(sample));

}

//----------------------------------------------------------------------------------

template<typename T> void IIRButterLpf<T>::filter(std::vector<T>& in, std::vector<T>& out)
{
  if(in.size() != out.size() )
    throw(std::invalid_argument("IIRButterLpf::filter -- In- and out vector sizes different!"));

  for(unsigned int n =0; n < in.size(); n++)
    out[n] = lpf_->clock(in[n]);

  if(!zero_phase_)
    return;

  std::reverse(out.begin(), out.end());
  for(unsigned int n =0; n < in.size(); n++)
    out[n] = lpf_->clock(out[n]);

  std::reverse(out.begin(), out.end());
}

//----------------------------------------------------------------------------------

#endif // IIRBUTTERLPF_H

#ifndef IIRLPF_H
#define IIRLPF_H

#include "BasicFilter.h"
#include <fstream>
#include <algorithm>
#include <stdexcept>

#include "spuc/generic/chebyshev.h"

//----------------------------------------------------------------------------------

template<typename T> class IIRChebyLpf : public BasicFilter<T>
{
  public:
    IIRChebyLpf(bool cheby2, double sampling_rate, double cutoff_freq,
           double ripple = 0.5, unsigned int order = 2, bool useZeroPhase = 0);


    virtual T clock(T sample );
    virtual void filter(std::vector<T>& in, std::vector<T>& out);

  private:
    bool                     cheby2_;
    double                   freq_;
    double                   fs_;
    double                   ripple_;
    unsigned int             order_;
    bool                     zero_phase_;

    SPUC::chebyshev<T>*       lpf_;
};

//----------------------------------------------------------------------------------

template<typename T> IIRChebyLpf<T>::IIRChebyLpf(bool cheby2, double sampling_rate, double cutoff_freq,
  double ripple, unsigned int order, bool useZeroPhase)
    : cheby2_(cheby2), freq_(cutoff_freq), fs_(sampling_rate), ripple_(ripple),
      order_(order), zero_phase_(useZeroPhase)
{
  if(cheby2_)
    throw(std::invalid_argument("Only Chebyshev Type 1 filters supported yet!"));

  if(freq_ >= fs_)
    throw(std::invalid_argument("Filter freq must be lower than fs!"));

  if(!freq_)
    throw(std::invalid_argument("Filter freq must not be 0!"));

  if(!fs_)
    throw(std::invalid_argument("Fs must not be 0!"));

  if(ripple_ < 0)
    throw(std::invalid_argument("Ripple must be a value (in dB) >0!"));

  if(!order_)
    throw(std::invalid_argument("Order must not be 0!"));

  if(zero_phase_)
  {
    if(order%2 != 0)
      throw(std::invalid_argument("Using ZeroPhase filtering, order has to be an even number!"));

    lpf_ = new SPUC::chebyshev(cutoff_freq/sampling_rate, order/2, ripple);
  }
  else
    lpf_ = new SPUC::chebyshev(cutoff_freq/sampling_rate, order, ripple);
}

//----------------------------------------------------------------------------------

template<typename T> T IIRChebyLpf<T>::clock(T sample )
{
  if(zero_phase_)
    throw(std::runtime_error("IIRChebyLpf::clock -- Zero phase not possible using clock!"));

  return(lpf_->clock(sample));

}

//----------------------------------------------------------------------------------

template<typename T> void IIRChebyLpf<T>::filter(std::vector<T>& in, std::vector<T>& out)
{
  if(in.size() != out.size() )
    throw(std::invalid_argument("IIRChebyLpf::filter -- In- and out vector sizes different!"));

  for(unsigned int n =0; n < in.size(); n++)
    out[n] = lpf_.clock(in[n]);

  if(!zero_phase_)
    return;

  std::reverse(out.begin(), out.end());
  for(unsigned int n =0; n < in.size(); n++)
    out[n] = lpf_->clock(out[n]);

  std::reverse(out.begin(), out.end());
}

//----------------------------------------------------------------------------------


#endif // IIRLPF_H

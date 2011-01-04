#ifndef IIRNOTCH_H
#define IIRNOTCH_H

#include "BasicFilter.h"
#include "iirnotch2.h"

#include <vector>
#include <stdexcept>
#include <algorithm>

//----------------------------------------------------------------------------------

template<typename T> class IIRNotch : public BasicFilter<T>
{
  public:
    IIRNotch(double sampling_rate, double notch_freq,
             unsigned int order = 2, bool useZeroPhase = 0);

    virtual T clock(T sample );
    virtual void filter(std::vector<T>& in, std::vector<T>& out);

  private:
    unsigned int             order_;
    bool                     zero_phase_;

    IIRNotch2<T>             notch_;

};

//----------------------------------------------------------------------------------

template<typename T> IIRNotch<T>::IIRNotch(double sampling_rate, double notch_freq,
  unsigned int order, bool useZeroPhase)
    : order_(order), zero_phase_(useZeroPhase), notch_(sampling_rate, notch_freq)
{
  if(!order)
    throw(std::invalid_argument("Order must not be 0!"));

  if(order%2 != 0)
    throw(std::invalid_argument("Order has to be an even number!"));

}

//----------------------------------------------------------------------------------

template<typename T> T IIRNotch<T>::clock(T sample )
{
  if(zero_phase_)
    throw(std::runtime_error("IIRNotch::clock -- Zero phase not possible using clock!"));

  return(notch_.clock(sample));

}

//----------------------------------------------------------------------------------

template<typename T> void IIRNotch<T>::filter(std::vector<T>& in, std::vector<T>& out)
{
  if(in.size() != out.size() )
    throw(std::invalid_argument("IIRNotch::filter -- In- and out vector sizes different!"));

  notch_.filter(in, out);

  for(unsigned int n = 1; n < order_/2; n++)
    notch_.filter(out, out);

  if(!zero_phase_)
    return;

  std::reverse(out.begin(), out.end());
  for(unsigned int n = 0; n < order_/2; n++)
    notch_.filter(out, out);

  std::reverse(out.begin(), out.end());
}

//----------------------------------------------------------------------------------


#endif // IIRNOTCH_H

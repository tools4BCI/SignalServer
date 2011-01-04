#ifndef IIRNOTCH2_H
#define IIRNOTCH2_H

#include "BasicFilter.h"

#include <vector>
#include <complex>
#include <stdexcept>

static const double POLE_DISTANCE = 0.99;

//----------------------------------------------------------------------------------

template<typename T> class IIRNotch2 : public BasicFilter<T>
{
  public:
    IIRNotch2(double sampling_rate, double notch_freq);

    virtual T clock(T sample );
    virtual void filter(std::vector<T>& in, std::vector<T>& out);

  private:
    void calculateCoefficients();

  private:
    double          freq_;
    double          fs_;
    double          d_;

    std::vector<double>   a_;
    std::vector<double>   b_;

    std::vector<T> clock_buffer_;
};

//----------------------------------------------------------------------------------

template<typename T> IIRNotch2<T>::IIRNotch2(double sampling_rate, double notch_freq)
    : freq_(notch_freq), fs_(sampling_rate), d_(POLE_DISTANCE),
      a_(3,0), b_(3,0), clock_buffer_(2,0)
{
  if(freq_ >= fs_)
    throw(std::invalid_argument("Filter freq must be lower than fs!"));

  if(!freq_)
    throw(std::invalid_argument("Filter freq must not be 0!"));

  if(!fs_)
    throw(std::invalid_argument("Fs must not be 0!"));

  calculateCoefficients();
}

//----------------------------------------------------------------------------------

template<typename T> T IIRNotch2<T>::clock(T sample )
{
    T sum = 0;
    T temp = 0;

    for(unsigned int m = 0; m < 2; m++)
      sum += clock_buffer_[m] * -a_[m +1];

    temp = sample + sum;

    sum = temp * b_[0];
    for(unsigned int m = 0; m < 2; m++)
      sum += clock_buffer_[m] * b_[m +1];

    clock_buffer_[1] = clock_buffer_[0];
    clock_buffer_[0] = temp;
    return(sum);
}

//----------------------------------------------------------------------------------

template<typename T> void IIRNotch2<T>::filter(std::vector<T>& in, std::vector<T>& out)
{
  T temp = 0;
  T sum = 0;

  std::vector<T> buffer(2,0);
  for(unsigned int n = 0; n < in.size(); n++)
  {
    sum = 0;
    for(unsigned int m = 0; m < 2; m++)
      sum += buffer[m] * -a_[m +1];

    temp = in[n] + sum;

    sum = temp * b_[0];
    for(unsigned int m = 0; m < 2; m++)
      sum += buffer[m] * b_[m +1];

    out[n] = sum;

    buffer[1] = buffer[0];
    buffer[0] = temp;
  }

}

//----------------------------------------------------------------------------------

template<typename T> void IIRNotch2<T>::calculateCoefficients()
{
  double w = (2 * M_PI * freq_)/fs_;
  std::complex<double> e1(0,w);
  std::complex<double> e2(0,-w);
  std::complex<double> v_comp = std::exp(e1) + std::exp(e2);

  if(v_comp.imag())
    throw(std::runtime_error("Coefficient factor has imaginary part -- something went wrong!"));

  double v( v_comp.real() );

  b_[0] = 1;
  b_[1] = -v;
  b_[2] = 1;

  a_[0] = b_[0];
  a_[1] = b_[1] * d_;
  a_[2] = b_[2] * pow(d_, 2);
}
#endif // IIRNOTCH2_H

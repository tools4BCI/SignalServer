#ifndef BasicFilter_H
#define BasicFilter_H

#include "filterTools_global.h"

#include <vector>

//enum FilterType{FIR, cheby1, cheby2, butter, elliptic};

template<typename T> class  BasicFilter
{
  public:
    virtual T clock(T sample) = 0;
    virtual void filter(std::vector<T>& in, std::vector<T>& out) = 0;

  protected:
    BasicFilter();


};

//----------------------------------------------------------------------------------

template<typename T> BasicFilter<T>::BasicFilter()
{
}

//----------------------------------------------------------------------------------


#endif // BasicFilter_H

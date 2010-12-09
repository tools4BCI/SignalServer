#ifndef CLOCK_H
#define CLOCK_H

#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>

namespace tobiss
{

class Clock
{
public:
    //---------------------------------------------------------------------------------------------
    static Clock& instance ();

    //---------------------------------------------------------------------------------------------
    boost::uint64_t getMicroSeconds () const;

    //---------------------------------------------------------------------------------------------
    void reset ();


private:
    Clock ();
    Clock (Clock const& src) {}
    Clock& operator= (Clock const& src) {}

    boost::posix_time::ptime start_time_;
};

}


#endif // CLOCK_H

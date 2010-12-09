#include "tia-private/clock.h"

#include <boost/date_time/microsec_time_clock.hpp>

namespace tobiss
{

//-------------------------------------------------------------------------------------------------
Clock::Clock () :
    start_time_ (boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time ())
{
    // nothing to do here at the moment ;)
}

//-------------------------------------------------------------------------------------------------
Clock& Clock::instance ()
{
    static Clock instance;
    return instance;
}

//-------------------------------------------------------------------------------------------------
boost::posix_time::ptime Clock::startTime () const
{
    return start_time_;
}

//-------------------------------------------------------------------------------------------------
boost::uint64_t Clock::getMicroSeconds () const
{
    boost::posix_time::time_duration time = boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time () - start_time_;
    boost::uint64_t microseconds = time.ticks ();
    microseconds *= 100000;
    microseconds /= time.ticks_per_second ();
    return microseconds;
}

//-------------------------------------------------------------------------------------------------
void Clock::reset ()
{
    start_time_ = boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time ();
}

}

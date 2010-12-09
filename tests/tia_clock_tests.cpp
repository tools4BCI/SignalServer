#include "tia-private/clock.h"

#include "UnitTest++/UnitTest++.h"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/microsec_time_clock.hpp>

using namespace tobiss;

//-------------------------------------------------------------------------------------------------
TEST (clockReset)
{
    boost::posix_time::ptime start_time = boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time ();

    Clock& clock = Clock::instance ();
    clock.reset ();

    boost::uint64_t elapsed_microseconds_clock = clock.getMicroSeconds ();

    boost::posix_time::time_duration time = start_time - boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time ();
    boost::uint64_t elapsed_microseconds_test = (time.ticks() * 1000000) / time.ticks_per_second();
    CHECK (elapsed_microseconds_clock <= elapsed_microseconds_test);
}

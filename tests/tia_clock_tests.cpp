#include "tia-private/clock.h"

#include "UnitTest++/UnitTest++.h"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/microsec_time_clock.hpp>
#include <boost/thread.hpp>
#include <boost/thread/xtime.hpp>

using namespace tobiss;

//-------------------------------------------------------------------------------------------------
TEST (clockTest)
{
    boost::posix_time::ptime start_time = boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time ();

    Clock& clock = Clock::instance ();
    clock.reset ();

    boost::xtime xt;
    boost::xtime_get (&xt, boost::TIME_UTC);
    xt.nsec += 10000;
    boost::thread::sleep (xt);

    boost::uint64_t elapsed_microseconds_clock = clock.getMicroSeconds ();

    boost::posix_time::time_duration time = boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time () - start_time;
    boost::uint64_t elapsed_microseconds_test = time.total_microseconds ();

    CHECK (elapsed_microseconds_test >= 1);
    CHECK (elapsed_microseconds_clock >= 1);
    CHECK (elapsed_microseconds_clock <= elapsed_microseconds_test);
}

#include "tia-private/clock.h"

#include "UnitTest++/UnitTest++.h"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/microsec_time_clock.hpp>

using namespace tobiss;

//-------------------------------------------------------------------------------------------------
TEST (clockReset)
{
    boost::posix_time::ptime start_time = boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time ();
    boost::posix_time::time_duration time = start_time_ - boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time ();

    Clock& clock = Clock::instance ();
    clock.reset ();
    CHECK (clock.getMicroSeconds() < 100);
}

/*
    This file is part of the TOBI SignalServer test routine.

    The TOBI SignalServer test routine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI SignalServer test routine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI SignalServer test routine. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

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

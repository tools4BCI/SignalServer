/*
    This file is part of the TOBI Interface A (TiA) library.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file lgpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/lgpl.html.

    In case of GNU Lesser General Public License Usage ,the TiA library
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TiA library. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: TiA@tobi-project.org
*/

#include "tia-private/clock.h"

#include <boost/date_time/microsec_time_clock.hpp>

namespace tia
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

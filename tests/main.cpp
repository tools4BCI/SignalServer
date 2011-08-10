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

#include "UnitTest++/UnitTest++.h"

#include <cstdlib>
#include <ctime>
#include <iostream>

int main()
{
    srand (time(NULL));
    std::cout << std::endl << "Build on " << __DATE__ << " at " << __TIME__ << std::endl << std::endl;
    return UnitTest::RunAllTests();
}

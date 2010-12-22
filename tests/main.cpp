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

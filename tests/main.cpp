#include "UnitTest++/UnitTest++.h"

#include <cstdlib>
#include <ctime>

int main()
{
    srand (time(NULL));
    return UnitTest::RunAllTests();
}

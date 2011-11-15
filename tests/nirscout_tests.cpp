
#include <iostream>

#include "UnitTest++/UnitTest++.h"

#include "hardware/nirscout.h"


#include <boost/date_time.hpp>
#include <boost/asio.hpp>

using namespace std;

static const int NIRSCOUT_TIMEOUT = 1000;
static const int STR_BUFFER_SIZE = 1024;
static const int CHUNK_SIZE = 1;

//-----------------------------------------------------------------------------

TEST(NIRScout_init)
{

  tobiss::HWThread* handle = new tobiss::NIRScout(0);


  delete handle;

}

//-----------------------------------------------------------------------------

TEST(NIRScout_data_acqu)
{
  tobiss::HWThread* handle = new tobiss::NIRScout(0);

  boost::posix_time::microseconds wait_duration(1000000*2);
  boost::asio::io_service io;

  handle->run();

  boost::asio::deadline_timer timer(io, wait_duration);
  timer.wait();

  handle->stop();

  delete handle;
}

//-----------------------------------------------------------------------------

TEST(NIRScout_timing)
{

}


//-----------------------------------------------------------------------------

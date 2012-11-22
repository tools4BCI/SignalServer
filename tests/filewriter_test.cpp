
#include "UnitTest++/UnitTest++.h"

#include "filewriter/file_writer.h"
#include "filewriter/file_writer_impl_base.h"
#include "filewriter/gdf_writer_impl.h"


#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <utility>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <boost/cstdint.hpp>

using namespace std;

//-----------------------------------------------------------------------------

TEST(FileWriterBaseTest)
{
  tobiss::FileWriterImplBase* writer = 0;

  writer = new tobiss::GdfWriterImpl();

  writer->addSample(1,1.0);

  if(writer)
  {
    delete writer;
    writer = 0;
  }

}

//-----------------------------------------------------------------------------


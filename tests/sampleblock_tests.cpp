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

#include "sampleblock/sample_block.h"
#include "tia/defines.h"

#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <utility>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <boost/cstdint.hpp>

using namespace std;

TEST(emptySampleBlock)
{
  tobiss::SampleBlock<double>   sample_block;

  CHECK(sample_block.getNrOfBlocks() == 0);
  CHECK(sample_block.getNrOfChannels() == 0);
  CHECK(sample_block.getNrOfSamples() == 0);
  CHECK(sample_block.getNrOfSignalTypes() == 0);
  CHECK(sample_block.getSamples().size() == 0);
  CHECK(sample_block.getTypes().size() == 0);

  std::vector<double> empty_vec;
  CHECK_ARRAY_EQUAL(sample_block.getSamples(), empty_vec, sample_block.getSamples().size());
  CHECK_ARRAY_EQUAL(sample_block.getTypes(), empty_vec, sample_block.getTypes().size());

  CHECK_THROW (sample_block.getSignalByFlag(1), std::invalid_argument);
  CHECK_THROW (sample_block.getSignalByNr(1), std::invalid_argument);
}

TEST(sampleBlocksorting)
{
  tobiss::SampleBlock<double>   sample_block;
  srand ( time(NULL) );

  boost::uint16_t blocksize = 1;
  boost::uint16_t nr_ch = 16;
  std::vector< std::pair<boost::uint32_t, double> > samples;

  for(unsigned int n = 0; n < nr_ch; n++)
    for(unsigned int m = 0; m < blocksize; m++)
    {
      samples.push_back( make_pair( rand() % 10 + 1 , rand() )  );

      cout << n << ": ";
      cout << "type: " << (*(--(samples.end()))).first <<  " -- ";
      cout << (*(--(samples.end()))).second << endl;
    }


//  sample_block.init(blocksize, nr_ch , ch_types);
//  sample_block.appendBlock(samples, blocksize);

  std::vector<double> return_samples;

  return_samples = sample_block.getSamples();

  for(unsigned int n = 0; n < return_samples.size(); n++)
  {
    cout << return_samples[n] << endl;
  }

}

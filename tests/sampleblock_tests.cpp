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
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <boost/cstdint.hpp>

using namespace std;

//-----------------------------------------------------------------------------

bool compare_pair (std::pair<boost::uint32_t, double> i,
                      std::pair<boost::uint32_t, double> j)
{
  return ( i.first < j.first );
}

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

TEST(sampleBlocksorting)
{
  tobiss::SampleBlock<double>   sample_block;
  srand ( time(NULL) );

  boost::uint16_t blocksize = 240;
  boost::uint16_t nr_ch = 16;
  unsigned int nr_sig_types = 8;
  vector<boost::uint32_t> ch_types;
  vector<double> values;
  std::vector< std::pair<boost::uint32_t, double> > samples;

  for(unsigned int sig_type = 1; sig_type <= nr_sig_types; sig_type++)
    for(unsigned int n = 0; n < nr_ch; n++)
    {
        boost::uint32_t type = rand() % nr_sig_types +1;
        ch_types.push_back(type);

        double value = rand();
        values.push_back(value);

        for(unsigned int n = 0; n < blocksize; n++)
        {
          samples.push_back( make_pair( type, value ));

          // cout << n << ": ";
          // cout << "type: " << (*(--(samples.end()))).first <<  " -- ";
          // cout << (*(--(samples.end()))).second << endl;
        }
    }

  sample_block.init(blocksize, nr_ch *nr_sig_types, ch_types);

  for(unsigned int n = 0; n < blocksize; n++)
    sample_block.appendBlock(values, 1);


  stable_sort (samples.begin(), samples.end(), compare_pair );
  std::vector<boost::uint32_t> sig_types = sample_block.getTypes();
  std::vector< std::pair<boost::uint32_t, double> > return_samples;

  for(unsigned int n = 0; n < sig_types.size(); n++)
  {
    std::vector<double> ret( sample_block.getSignalByFlag( sig_types[n] ) );

    for(unsigned int m = 0; m < ret.size(); m++)
    {
      return_samples.push_back( make_pair( sig_types[n], ret[m] ) );
      // cout << sig_types[n] << ": " << ret[m] << endl;
    }
  }

  for(unsigned int n = 0; n < samples.size(); n++)
  {
    CHECK_EQUAL(samples[n].first, return_samples[n].first);
    CHECK_EQUAL(samples[n].second, return_samples[n].second);
  }


}

//-----------------------------------------------------------------------------

TEST(specificSampleBlockSorting)
{
  tobiss::SampleBlock<double>   sample_block;
  boost::uint16_t blocksize = 2;
  boost::uint16_t nr_ch = 11;

  std::vector<boost::uint32_t> ch_types;

  for(unsigned int n = 0; n < 4; n++)
    ch_types.push_back(SIG_EOG);
  for(unsigned int n = 0; n < 6; n++)
    ch_types.push_back(SIG_EMG);

  ch_types.push_back(SIG_ECG);

  vector<double> samples;
  vector<double> eog;
  vector<double> emg;
  vector<double> ecg;

  for(unsigned int n = 0; n < 4; n++)
    for(unsigned int n = 0; n < blocksize; n++)
      eog.push_back( rand() );

  for(unsigned int n = 0; n < 6; n++)
    for(unsigned int n = 0; n < blocksize; n++)
      emg.push_back( rand() );

  for(unsigned int n = 0; n < 1; n++)
    for(unsigned int n = 0; n < blocksize; n++)
      ecg.push_back( rand() );

  samples.insert( samples.end(), eog.begin(), eog.end() );
  samples.insert( samples.end(), emg.begin(), emg.end() );
  samples.insert( samples.end(), ecg.begin(), ecg.end() );


  sample_block.init(blocksize, nr_ch, ch_types);
  sample_block.setSamples( samples );

  vector<double> ret;
  vector<double> eog_ret = sample_block.getSignalByFlag( SIG_EOG );
  vector<double> emg_ret = sample_block.getSignalByFlag( SIG_EMG );
  vector<double> ecg_ret = sample_block.getSignalByFlag( SIG_ECG );


  ret.insert( ret.end(), eog_ret.begin(), eog_ret.end());
  ret.insert( ret.end(), emg_ret.begin(), emg_ret.end());
  ret.insert( ret.end(), ecg_ret.begin(), ecg_ret.end());


  CHECK_EQUAL(eog.size(), eog_ret.size() );
  CHECK_ARRAY_EQUAL(eog, eog_ret, eog.size());

  CHECK_EQUAL(emg.size(), emg_ret.size() );
  CHECK_ARRAY_EQUAL(emg, emg_ret, emg.size());

  CHECK_EQUAL(ecg.size(), ecg_ret.size() );
  CHECK_ARRAY_EQUAL(ecg, ecg_ret, ecg.size());

  CHECK_EQUAL(ret.size(), samples.size() );
  CHECK_ARRAY_EQUAL(ret, samples, ret.size());

}


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

/**
* @file raw_mem.cpp
**/

#include <iostream>
#include <vector>

#include <boost/numeric/conversion/cast.hpp>
#include "tia-private/datapacket/raw_mem.h"

namespace tobiss
{

using boost::numeric_cast;
using boost::numeric::bad_numeric_cast;
using boost::numeric::positive_overflow;
using boost::numeric::negative_overflow;

using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;

using std::vector;
using std::cerr;

//-----------------------------------------------------------------------------

RawMem::RawMem(uint32_t flags, uint64_t sample_nr, uint64_t packet_nr, \
                boost::posix_time::ptime timestamp, \
                vector<uint16_t>& nr_values, vector<uint16_t>& nr_blocks,
                vector<double>& data)  : size_(0)
{
  size_ = sizeof(flags) + sizeof(sample_nr) + sizeof(packet_nr) + sizeof(timestamp) \
  + nr_blocks.size() * sizeof(nr_blocks[0]) \
  + nr_values.size() * sizeof(nr_values[0]) \
  + data.size() * sizeof(float);  // FIXXXXXME  ...  hardcoded sizeof() !!!!

  mem_ = malloc(size_);

  uint32_t* ui32_ptr = reinterpret_cast<uint32_t*>(mem_);
  *ui32_ptr++ = flags;

  uint64_t* ui64_ptr = reinterpret_cast<uint64_t*>(ui32_ptr);
  *ui64_ptr++ = sample_nr;
  *ui64_ptr++ = packet_nr;

  boost::posix_time::ptime* ptime_ptr
  = reinterpret_cast<boost::posix_time::ptime*>(ui64_ptr);
  *ptime_ptr++ = timestamp;

  uint16_t* ui16_ptr = reinterpret_cast<uint16_t*>(ptime_ptr);

  for(unsigned int n = 0; n < nr_values.size(); n++)
    *ui16_ptr++ = nr_values[n];
  for(unsigned int n = 0; n < nr_blocks.size(); n++)
    *ui16_ptr++ = nr_blocks[n];

  try
  {
    float* flt_ptr = reinterpret_cast<float*>(ui16_ptr);
    for(unsigned int n = 0; n < data.size(); n++)
      *flt_ptr++ = numeric_cast<float>(data[n]);
  }
  catch(negative_overflow& e)
  {
    cerr << "RawMem -- Constructor: " << e.what();
  }
  catch(positive_overflow& e)
  {
    cerr << "RawMem -- Constructor: " <<  e.what();
  }
  catch(bad_numeric_cast& e)
  {
    cerr << "RawMem -- Constructor: " <<  e.what();
  }
}

} // Namespace tobiss

//-----------------------------------------------------------------------------

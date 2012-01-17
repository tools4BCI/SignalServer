/*
    This file is part of the TOBI SignalServer.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------


    GNU General Public License Usage
    Alternatively, this file may be used under the terms of the GNU
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file gpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/gpl.html.

    In case of GNU General Public License Usage ,the TOBI SignalServer
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI SignalServer. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: SignalServer@tobi-project.org
*/

/**
* @file statistics.cpp
**/

#include "misc/statistics.h"

#include <algorithm>


namespace tobiss
{

  using std::for_each;

//-----------------------------------------------------------------------------
 
Statistics::Statistics( )
{
  N = 0;
  mean_ = 0;
  adaptive_mean_ = 0;
  update_coefficient_ = 0.01;
  adaptive_var_ = 0;
}

//-----------------------------------------------------------------------------
 
Statistics::~Statistics( )
{
}

//-----------------------------------------------------------------------------
 
void Statistics::update( double x )
{
  sample_buffer_.push_back( x );

  N++;

  mean_ = ( (N-1)*mean_ + x ) / N;

  adaptive_mean_ = adaptive_mean_ * (1-update_coefficient_) + x * update_coefficient_;
  adaptive_var_ = adaptive_var_ * (1-update_coefficient_) + update_coefficient_ * ( x - adaptive_mean_ ) * ( x - adaptive_mean_ );
}

//-----------------------------------------------------------------------------

void Statistics::getSamples( size_t buffersize, boost::circular_buffer<double>::const_iterator &begin, boost::circular_buffer<double>::const_iterator &end  )
{
  end = sample_buffer_.end();
  begin = end - buffersize;
}

//-----------------------------------------------------------------------------

void Statistics::requestBuffer( size_t buffersize )
{
  if( buffersize > sample_buffer_.capacity() )
    sample_buffer_.set_capacity( buffersize );
}

//-----------------------------------------------------------------------------

}

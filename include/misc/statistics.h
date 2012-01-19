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
* @file statistics.h
*
* @brief Class to collect various runtime statistics.
**/

#ifndef STATISTICS_H
#define STATISTICS_H

#include <boost/circular_buffer.hpp>
#include <vector>

namespace tobiss
{

/**
* @class Statistics
* @brief Class to calculate various (univariate) runtime statistics.
*/
class Statistics
{
friend class StatisticInterface;

public:
  /**
    * @brief Constructor
    */
  Statistics( );

  /**
    * @brief Destructor
    */
  virtual ~Statistics( );
  
  /**
    * @brief Update statistics with a new sample
    * @param[in] x double New sample
    */
  void update( double x );

  void reset( );

  double get_mean( ) { return mean_; }
  double get_adaptive_mean( ) { return adaptive_mean_; }
  double get_adaptive_var( ) { return adaptive_var_; }

protected:  
  /**
    * @brief Request buffer to be at least of given size
    */
  void requestBuffer( size_t buffersize );

  /**
    * @brief Returns iterators to a range of the sample buffer buffer
    * @param[in] buffersize size_t size of the buffer
    * @param[out] begin boost::circular_buffer<double>::const_iterator start of the buffer section
    * @param[out] end size_t boost::circular_buffer<double>::const_iterator end of the buffer section
    */
  void getSamples( size_t buffersize, boost::circular_buffer<double>::const_iterator &begin, boost::circular_buffer<double>::const_iterator &end  );

private:
  template<typename T> inline static void delptr( T ptr ) { delete ptr; }

private:
  boost::circular_buffer<double> sample_buffer_;
  double N;
  double update_coefficient_;
  double mean_;
  double adaptive_mean_;
  double adaptive_var_;
};

}

#endif

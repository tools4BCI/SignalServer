/*
    Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Michele Tavella <michele.tavella@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    It is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this file.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TCTIMESTAMP_HPP
#define TCTIMESTAMP_HPP
#include "TCTime.hpp"
#include <string>

typedef struct timeval TCTimeval;

/*! \brief Timestamp class
 *
 * \ingroup tobicore
 */
class TCTimestamp {
  public:
    /*! \brief Constructor
     */
    TCTimestamp(void);

    /*! \brief Destructor
     */
    virtual ~TCTimestamp(void);

    /*! \brief Invalidates the timestamp
     */
    virtual void Unset(void);

    /*! \brief Check if the timestamp is valid
     *
     * \return True if valid, false otherwise
     */
    virtual bool IsSet(void);

    /*! \brief Tics the timestamp
     *
     * Tic is like starting a stopwatch timer
     */
    virtual void Tic(void);

    /*! \brief Tocs the timestamp
     *
     * Measures the difference between the external timestamp and
     * the instance.
     *
     * \arg timestamp External timestamp
     * \return Time difference in milliseconds
     */
    virtual double Toc(TCTimeval* timestamp);

    /*! \brief Tocs the timestamp
     *
     * Toc is like checking how long a stopwatch timer has been running
     *
     * \return Time difference in milliseconds
     */
    virtual double Toc(void);

    /*! \brief Timestamp getter (as string)
     *
     * Writes a stringified representation of the timestamp
     * \arg timestamp Stringified timestamp representation
     */
    virtual void Get(std::string* timestamp) const;

    /*! \brief Timestamp setter (as string)
     *
     * Reads a stringified representation of the timestamp
     * \arg timestamp Stringified timestamp representation
     * \return True if stringified representation is valid
     */
    virtual bool Set(const std::string& timestamp);

    /*! \brief Timestamp setter
     *
     * From external timestamp
     * \arg timestamp Reference to external timestamp
     */
    virtual void Set(const TCTimeval* timestamp);

  public:

    /*! \brief Timestamp
     */
    TCTimeval timestamp;
};

#endif

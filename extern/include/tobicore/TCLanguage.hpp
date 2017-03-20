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

#ifndef TCLANGUAGE_HPP
#define TCLANGUAGE_HPP

#include "TCBlock.hpp"
#include <string>

/*! \brief Language model
 *
 * \ingroup tobicore
 */
class TCLanguage {
  public:
    /*! \brief Creates a status message.
     *
     * \arg component Component ID (i.e.: iA, iB, iC...)
     * \arg status Return code
     * \arg fidx Frame index
     * \return Status message
     */
    std::string Status(const int component, const int status,
        const int fidx  = TCBlock::BlockIdxUnset);

    /*! \brief Checks if message is compatible
     *
     * Checks if a status message is compatible.
     *
     * \arg message A status message
     * \return True if compatible, false otherwise
     */
    bool CheckVersion(const std::string& message);

    /*! \brief
     *
     * \arg component
     * \arg status
     * \arg fidx
     * \return
     */
    bool IsStatus(const std::string& message, int* component, int* status,
        int* fidx);

    /*! \brief
     *
     * \arg component
     * \arg status
     * \arg fidx
     * \return
     */
    std::string GetComponent(int component);

    /*! \brief
     *
     * \arg component
     * \arg status
     * \arg fidx
     * \return
     */
    std::string GetStatus(int component);

  public:
    /*
    //! \brief Supported components
    enum { IA, IB, IC, ID, Otherwise };

    //! \brief Error codes
    enum { Ready = 0, Quit, ErrorGeneric, ErrorNotSupported };
     */

    //! \brief TOBI iA
    const static int IA = 1;

    //! \brief TOBI iB
    const static int IB = 2;

    //! \brief TOBI iC
    const static int IC = 3;

    //! \brief TOBI iD
    const static int ID = 4;

    //! \brief Ready return code
    const static int Ready = 1;

    //! \brief Quit return code
    const static int Quit  = 2;

    //! \brief Generic error code
    const static int ErrorGeneric = -1;

    //! \brief Not-supported error code
    const static int ErrorNotSupported = -2;
};

#endif

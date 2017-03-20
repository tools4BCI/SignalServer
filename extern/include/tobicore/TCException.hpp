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

#ifndef TCEXCEPTION_HPP
#define TCEXCEPTION_HPP

#include <string>

/*! \brief Exception class
 *
 * \ingroup tobicore
 */
class TCException {
  public:
    /*! \brief Constructor
     *
     * \param info Exception message
     * \param caller Method that raised the exception (default: "undef")
     */
    TCException(std::string info, std::string caller = "undef");

    /*! \brief Destructor
     */
    virtual ~TCException(void);

    /*! \brief Caller getter
     * \return Caller
     */
    virtual std::string GetCaller(void) const;

    /*! \brief Message getter
     * \return Message
     */
    virtual std::string GetInfo(void) const;

  public:
    /*! \brief Operator << overload
     * \return std::ostream
     */
    friend std::ostream &operator<<(std::ostream& output,
        const TCException&);

    /*! \brief Operator << overload
     * \return std::ostream
     */
    friend std::ostream &operator<<(std::ostream& output,
        const TCException*);

    /*! \brief Operator == overload
     *
     * To compare two different exceptions
     * \return True if the exception message are different
     */
    bool operator==(const TCException& right);

    /*! \brief Operator != overload
     *
     * To compare two different exceptions
     * \return True if the exception message are the same
     */
    bool operator!=(const TCException& right);

  private:
    /*! \brief Information message
     *
     * i.e. 'Out of tomato exception'
     */
    std::string _info;

    /*! \brief Caller/thrower
     *
     * i.e. 'void Pizza::Make(uint32 id)'
     */
    std::string _caller;
};

#endif

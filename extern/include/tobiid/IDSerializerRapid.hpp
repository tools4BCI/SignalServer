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

#ifndef IDSERIALIZERRAPID_HPP
#define IDSERIALIZERRAPID_HPP

#include "IDMessage.hpp"
#include "IDSerializer.hpp"

/*! \brief TOBI iD concrete XML serializer
 *
 * \ingroup tobiid
 *
 * Serializes an IDMessage to XML using RapidXML.
 *
 * \sa IDSerializer
 */
class IDSerializerRapid : public IDSerializer {
  public:
    /*! \brief Constructor
     * \arg message Pointer to an IDMessage instance
     * \arg indent If true, the XML output will be indented
     * \arg declaration If true, the XML output will contain an XML
     * declaration
     */
    IDSerializerRapid(IDMessage* const message = NULL,
        const bool indent = false, const bool declaration = false);

    /*! \brief Destructor
       */
    virtual ~IDSerializerRapid(void);

    /*! \brief Serializes to std::string
     *
     * \arg buffer Pointer to the std::string that will contain the result
     * of the serialization
     * \return Pointer to the buffer instance
     */
    virtual std::string* Serialize(std::string* buffer);

    /*! \brief Deserializes from std::string
     *
     * \arg buffer Pointer to the std::string that contains the result
     * of a serialization and that will be used for the deserialization
     * \return A pointer to the buffer instance
     */
    virtual std::string* Deserialize(std::string* const buffer);

  protected:
    //! \brief Indentation flag
    bool _indent;
    //! \brief Declaration flag
    bool _declaration;
};

#endif

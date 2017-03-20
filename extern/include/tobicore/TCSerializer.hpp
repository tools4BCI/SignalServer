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

#ifndef TCSERIALIZER_HPP
#define TCSERIALIZER_HPP

#include "TCBlock.hpp"

/*! \brief Serializer interface for TCBlock
 *
 * \ingroup tobicore
 *
 * Exactly like ICSerializer or IDSerializer
 *
 * \sa ICSerializer, IDSerializer
 */
class TCSerializer {
  public:
    /*! \brief Constructor
     */
    TCSerializer(void);
    /*! \brief Constructor
     *
     * \arg block TCBlock to serialize/deserialize
     */
    TCSerializer(TCBlock* const block);

    /*! \brief Destructor
     */
    virtual ~TCSerializer(void);

    /*! \brief Sets the associated ICMessage
     *
     * \arg block TCBlock to serialize/deserialize
     * \return Pointer to the serializer instance
     */
    TCSerializer* SetMessage(TCBlock* const block);

    /*! \brief Serializes to std::string
     *
     * \arg buffer Pointer to the std::string that will contain the result
     * of the serialization
     * \return Pointer to the buffer instance
     */
    virtual std::string* Serialize(std::string* buffer) = 0;

    /*! \brief Deserializes from std::string
     *
     * \arg buffer Pointer to the std::string that contains the result
     * of a serialization and that will be used for the deserialization
     * \return A pointer to the buffer instance
     */
    virtual std::string* Deserialize(std::string* const buffer) = 0;

    /*! \brief Serializes to std::string
     * \return std::string that contains the result of the serialization
     */
    virtual std::string SerializeCp(void);

    /*! \brief Deserializes from std::string
     *
     * \arg buffer std::string that contains the result
     * of a serialization and that will be used for the deserialization
     */
    virtual void DeserializeCp(const std::string& buffer);

    /*! \brief Serializes to a char array
     *
     * \arg buffer Pointer to a char array that will contain the result of
     * the serialization
     * \arg bsize Size of the char array
     */
    virtual char* SerializeCh(char* buffer, const unsigned int bsize);

    /*! \brief Deserializes from a char array
     *
     * \arg buffer Pointer to a char array that contains a serialized
     * ICMessage and that will be used for deserialization
     * \arg bsize Size of the char array
     */
    virtual const char* DeserializeCh(const char* buffer, const unsigned int bsize);

  protected:
    /*! \brief Pointer to the associated TCBlock
     *
     * Each TCSerializer has an associated TCBlock.
     */
    TCBlock* _message;
};

#endif

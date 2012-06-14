/*
    Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Michele Tavella <michele.tavella@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IDSERIALIZER_HPP
#define IDSERIALIZER_HPP

#include "IDMessage.hpp"

/*! \brief TOBI iD serializer interface
 *
 * \ingroup tobiid
 *
 * An IDMessage object embeds a simple data structure with very few fields
 * (specially if compared to the ICMessage recursive data structure).
 * In order to transmit an IDMessage from one process to another, it is then
 * necessary to serialize its data structure to a format that might be
 * transferred, for examples, trough a socket.
 *
 * The IDSerializer interface can then be derived to implement different
 * flavors of serialization formats. Ad today only XML is supported by the
 * concrete IDSerializerRapid class.
 */
class IDSerializer {
	public:
		/*! \brief Constructor
		 */
		IDSerializer(void);

		/*! \brief Constructor
		 *
		 * \arg message Pointer to the IDMessage to serialize/deserialize
		 */
		IDSerializer(IDMessage* const message);

		/*! \brief Destructor
		 */
		virtual ~IDSerializer(void);
		
		/*! \brief Sets the associated IDMessage
		 *
		 * \arg IDMessage to serialize/deserialize
		 * \return Pointer to the serializer instance
		 */
		IDSerializer* SetMessage(IDMessage* const message);

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
		 * IDMessage and that will be used for deserialization
		 * \arg bsize Size of the char array
		 */
		virtual const char* DeserializeCh(const char* buffer, const unsigned int bsize);
	
	public:
		/*! \brief Pointer to the associated IDMessage
		 *
		 * Each IDSerializer has an associated IDMessage. Once an IDMessage is
		 * associated, its content can be serialized.
		 * It is also possible to deserialize a string into the associated
		 * IDMessage.
		 */
		IDMessage* message;
};

#endif

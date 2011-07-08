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

class IDSerializer {
	public:
		IDSerializer(void);
		IDSerializer(IDMessage* const message);
		virtual ~IDSerializer(void);
		
		IDSerializer* SetMessage(IDMessage* const message);

		virtual std::string* Serialize(std::string* buffer) = 0;
		virtual std::string* Deserialize(std::string* const buffer) = 0;
		
		virtual std::string SerializeCp(void);
		virtual void DeserializeCp(const std::string& buffer);
		
		virtual char* SerializeCh(char* buffer, const unsigned int bsize);
		virtual const char* DeserializeCh(const char* buffer, const unsigned int bsize);
	
	public:
		IDMessage* message;
};

#endif

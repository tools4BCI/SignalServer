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

#ifndef TCSERIALIZER_HPP
#define TCSERIALIZER_HPP

#include "TCBlock.hpp"

class TCSerializer {
	public:
		TCSerializer(void);
		TCSerializer(TCBlock* const block);
		virtual ~TCSerializer(void);
		
		TCSerializer* SetMessage(TCBlock* const block);

		virtual std::string* Serialize(std::string* buffer) = 0;
		virtual std::string* Deserialize(std::string* const buffer) = 0;
		
		virtual std::string SerializeCp(void);
		virtual void DeserializeCp(const std::string& buffer);
		
		virtual char* SerializeCh(char* buffer, const unsigned int bsize);
		virtual const char* DeserializeCh(const char* buffer, const unsigned int bsize);
	
	protected:
		TCBlock* _message;
};

#endif

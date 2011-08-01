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

#ifndef IDSERIALIZERRAPID_HPP
#define IDSERIALIZERRAPID_HPP

#include "IDMessage.hpp"
#include "IDSerializer.hpp"

class IDSerializerRapid : public IDSerializer {
	public:
		IDSerializerRapid(IDMessage* const message = NULL, 
				const bool indent = false, const bool declaration = false);
		virtual ~IDSerializerRapid(void);
		
		virtual std::string* Serialize(std::string* buffer);
		virtual std::string* Deserialize(std::string* const buffer);

	protected:
		bool _indent;
		bool _declaration;
};

#endif

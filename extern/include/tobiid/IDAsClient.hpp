/*/*
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

#ifndef IDASCLIENT_HPP 
#define IDASCLIENT_HPP 

#include "IDTypes.hpp"
#include "IDMessage.hpp"
#include "IDSerializer.hpp"
#include <tobicore/TCException.hpp>
#include <tobicore/TCBlock.hpp>
#include <vector>

class IDAsClient : public TCBlock {
	public:
		IDAsClient(void);
		virtual ~IDAsClient(void);
		virtual void Add(const IDMessage message, bool updatefidx = false);
		virtual bool Get(IDMessage* message = NULL,  
				const IDFtype type = IDMessage::FamilyUndef,
				const IDevent event = IDMessage::EventNull, 
				const int direction = IDAsClient::BlockAll);
		virtual unsigned int Size(void) const;
		virtual unsigned int Clear(void);
		virtual void Dump(void) const;
	private:
	protected:

	public:
		static const int BlockThis = 0;
		static const int BlockPrev = 1;
		static const int BlockNext = 2;
		static const int BlockAll = 3;
	private:
		std::vector<IDMessage> _queue;
	protected:
};

#endif

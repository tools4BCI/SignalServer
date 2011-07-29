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

#ifndef TCBLOCK_HPP 
#define TCBLOCK_HPP 

#include "TCTimestamp.hpp"
#include <string>

typedef struct timeval TCTimeval;

class TCBlock {
	public:
		TCBlock(void);
		/*! \brief Sets frame index value
		 *
		 */
		virtual int SetBlockIdx(int fidx = 0);
		/*! \brief Gets frame index value
		 */
		virtual int GetBlockIdx(void) const;
		/*! \brief Increments frame index value
		 */
		virtual int IncBlockIdx(void);
		virtual void UnsetBlockIdx(void);
		virtual bool IsSetBlockIdx(void) const;

	public:
		static const int BlockIdxUnset = -1;
		TCTimestamp absolute;
		TCTimestamp relative;
	protected:
		int _blockidx;
};

#endif

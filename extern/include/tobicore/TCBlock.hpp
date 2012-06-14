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

/*! \brief Data block
 * 
 * \ingroup tobicore
 * 
 * Each communication blocks in the BCI should carry a frame index
 * value (generally set by the acquisition module), an absolute timestamp (i.e.
 * 2011-11-19, 22:00:00.00.00) and a relative timestamp (i.e. 200 seconds).
 */
class TCBlock {
	public:
		/*! \brief Constructor
		 */
		TCBlock(void);

		/*! \brief Frame index setter
		 */
		virtual int SetBlockIdx(int fidx = TCBlock::BlockIdxUnset);

		/*! \brief Frame index getter
		 */
		virtual int GetBlockIdx(void) const;

		/*! \brief Increments frame index
		 */
		virtual int IncBlockIdx(void);

		/*! \brief Unsets (invalidates) frame index
		 * 
		 * If the frame index is unset, it means a particular 
		 * communication blocks has not been generated within the BCI
		 * pipeline. For example, a BCI-controlled robot on the moon 
		 * will send messages without setting the frame index. 
		 * A neurofeedback will receive and generate communication blocks 
		 * that carry both a correct frame index and all the timestamp
		 * information.
		 */
		virtual void UnsetBlockIdx(void);

		/*! \brief Checks if frame index is set (valid)
		 * \return True if set, false otherwise
		 */
		virtual bool IsSetBlockIdx(void) const;

	public:
		/*! \brief Unset frame index value
		 */
		static const int BlockIdxUnset = -1;
		
		/*! \brief  Absolute timestamp
		 */
		TCTimestamp absolute;
		
		/*! \brief  Relative timestamp
		 */
		TCTimestamp relative;
	protected:
		/*! \brief Frame index 
		 */
		int _blockidx;
};

#endif

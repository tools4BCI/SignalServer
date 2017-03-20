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

#ifndef IDTYPES_HPP
#define IDTYPES_HPP
#include <string>

//#define IDTYPES_FAMILY_UNDEF		"undef"
//#define IDTYPES_FAMILY_BIOSIG		"biosig"
//#define IDTYPES_FAMILY_CUSTOM		"custom"

#define IDMESSAGE_VERSION_SUPPORTED 			"0.0.2.1"
#define IDMESSAGE_VERSION 			"0.3.0.0"

#define IDMESSAGE_ROOTNODE 			"tobiid"
#define IDMESSAGE_ROOTNODE_NEW	"tid"
#define IDMESSAGE_VERSIONNODE		"version"

#define IDMESSAGE_FRAMENODE			"frame"
#define IDMESSAGE_BLOCKNODE			"block"

#define IDMESSAGE_DESCRIPTIONNODE	"description"
#define IDMESSAGE_FAMILYNODE		"family"
#define IDMESSAGE_EVENTNODE 		"event"
#define IDMESSAGE_VALUENODE 		"value"
#define IDMESSAGE_TIMESTAMPNODE		"timestamp"
#define IDMESSAGE_REFERENCENODE		"reference"

#define IDMESSAGE_ABSOLUTE_TS		"absolute"
#define IDMESSAGE_RELATIVE_TS		"relative"

#define IDMESSAGE_SOURCENODE    		"source"

/*! \brief TOBI iD family type
 *
 * \ingroup tobiid
 */
typedef int IDFtype;

/*! \brief TOBI iD family value type
 *
 * \ingroup tobiid
 */
typedef std::string IDFvalue;

/*! \brief TOBI iD event
 *
 * \ingroup tobiid
 */
typedef int IDevent;

/*! \brief TOBI iD value
 *
 * \ingroup tobiid
 */
typedef float IDvalue;

#endif

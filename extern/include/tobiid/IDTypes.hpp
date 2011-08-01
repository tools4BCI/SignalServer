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

#ifndef IDTYPES_HPP
#define IDTYPES_HPP
#include <string>

#define IDTYPES_FAMILY_UNDEF		"undef"
#define IDTYPES_FAMILY_BIOSIG		"biosig"
#define IDTYPES_FAMILY_CUSTOM		"custom"

#define IDMESSAGE_VERSION 			"0.0.2.1"
#define IDMESSAGE_ROOTNODE 			"tobiid"
#define IDMESSAGE_VERSIONNODE		"version"
#define IDMESSAGE_FRAMENODE			"frame"
#define IDMESSAGE_DESCRIPTIONNODE	"description"
#define IDMESSAGE_FAMILYNODE		"family"
#define IDMESSAGE_EVENTNODE 		"event"
#define IDMESSAGE_TIMESTAMPNODE		"timestamp"
#define IDMESSAGE_REFERENCENODE		"reference"

/*! \mainpage libtobiid
 *
 * \section sec_intro Introduction
 * libtobiid implements the TOBI hBCI Interface C. 
 * The IDLanguage class handles the sIDMessage message structure.
 * IDLanguageRapid is responsible for serializing/deserializing messages in XML
 * format.
 *
 * As today, the library is at his very stage of development. 
 *
 * \section sec_xml XML message
 * The XML structure looks like this:
 *
 * <PRE>
 * <?xml version="1.0" encoding="utf-8"?>
 * <<b>messagec</b> version="0.0.1">
 *   <<b>classifier</b> id="1" description="MI Classifier">
 *     <<b>class</b> id="1" description="Right Hand">
 *             <<b>value</b> type="prob">0.100000</<b>value</b>>
 *             <<b>label</b> type="biosig">0x301</<b>label</b>>
 *     </<b>class</b>>
 *     <<b>class</b> id="2" description="Left Hand">
 *             <<b>value</b> type="prob">0.200000</<b>value</b>>
 *             <<b>label</b> type="biosig">0x302</<b>label</b>>
 *     </<b>class</b>>
 *     <<b>class</b> id="3" description="Feet">
 *             <<b>value</b> type="prob">0.300000</<b>value</b>>
 *             <<b>label</b> type="biosig">0x303</<b>label</b>>
 *     </<b>class</b>>
 *    </<b>classifier</b>>
 *    <<b>classifier</b> id="15" description="Language Classifier">
 *      <<b>class</b> id="12" description="Error">
 *              <<b>value</b> type="rcoe">0.100000</<b>value</b>>
 *              <<b>label</b> type="biosig">0x401</<b>label</b>>
 *      </<b>class</b>>
 *      <<b>class</b> id="1" description="Correct">
 *              <<b>value</b> type="rcoe">0.900000</<b>value</b>>
 *              <<b>label</b> type="biosig">0x402</<b>label</b>>
 *      </<b>class</b>>
 *    </<b>classifier</b>>
 * </<b>messagec</b>>
 * </PRE>
 *
 * Each message can hold at max IDTYPES_MAX_CLASSIFIERS classifiers and
 * IDTYPES_MAX_CLASSES classes per classifier (look at IDTypes.hpp).
 * Look at IDLanguage and IDLanguageRapid for a more precise description of what
 * is going on.
 *
 * \section sec_install Installation
 *
 * \subsection sec_install_step1 Dependencies
 * In order to compile the library, you need: 
 * <a href='http://rapidxml.sourceforge.net/'>RapidXML</a>.
 *
 * \subsection sec_install_step2 Compile the sources
 * Execute, as usual:
 * <PRE>
 * sh autogen.sh
 * mkdir build
 * cd build/ 
 * ../configure --prefix=/home/mtavella/Build 
 * make
 * make install
 * </PRE>
 *
 * \subsection sec_install_examples Examples
 * Please take a look at:
 * <PRE>
 * examples/exampleicrapid.cpp
 * </PRE>
 *
 * \subsection sec_install_doxy Compile the Doxygen documentation 
 * <PRE>
 * make doxygen-doc
 * </PRE>
 *
 * \subsection sec_install_windows Visual Studio 2008 
 * If you are willing to compile libtobiid with Visual Studio 2008, you might
 * want to check the bindings/windll/ folder.
 * Be aware that you need to set where the RapidXML includes are in the project
 * properties.
 *
 * \subsection sec_notes Notes, to-do list etc.
 * Feel free to add/remove.
 *  - Errors
 *  	-# Handle errors properly, both in libtobiid and mextobiic
 *	- XML
 * 		-# Do we like RapidXML?
 * 		-# Are the performances good enough? 
 * 		-# Personally, I would also be able to send the binary data directly.
 *	- Labels
 *  	-# As today the labels are handled as char arrays. Any suggestion
 *  	here?
 *  	-# Generally speaking, how are labels handled in the hBCI?
 *	- Misc
 *      -# assert() is widely used when something is not done correctly. 
 *      Much work is required on this side, but before making things better,
 *      we need to decide whether we are on the right track.
 *      By the way, should we define return codes or use exceptions?
 *
 * \subsection sec_releases Releases and changelog
 * 	- v0.0.3.0 
 * 		-# Fixed IDClassifier::LabelType when asked for LabelClass
 * 	- v0.0.2.0 First public release
 */


//! Typedef
typedef int   		IDFtype;
typedef std::string IDFvalue;

typedef int IDevent;

#endif

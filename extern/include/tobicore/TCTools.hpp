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

#ifndef TCTOOLS_HPP
#define TCTOOLS_HPP

#include <string>

/*! \brief Static tools class.
 *
 * \ingroup tobicore
 *
 * It provides some static methods to perform data conversion. 
 */
class TCTools {
	public:	
		/*! \brief Classic itoa
		 * \arg value Integer to convert
		 * \arg result Reference to char array containing the converted integer
		 * \arg base Base for conversion
		 * \return Reference to char array containing the converted integer upon
		 * success, NULL otherwise.
		 */
		static char* itoa(int value, char* result, int base = 10);

		/*! \brief Classic ftoa
		 * \arg value Float to convert
		 * \arg result Reference to char array containing the converted float
		 * \return Reference to char array containing the converted float upon
		 * success, NULL otherwise.
		 */
		static int ftoa(float value, char* result);
		
		/*! \brief Classic atof
		 * \arg value String to convert
		 * \return Converted float
		 */
		static float atof(const char* value);
		
		/*! \brief Classic itos
		 * \arg Value to convert
		 * \return Converted integer as string
		 */
		static std::string itos(int value);
};

#endif

/*
    This file is part of the TOBI Interface A (TiA) library.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file lgpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/lgpl.html.

    In case of GNU Lesser General Public License Usage ,the TiA library
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TiA library. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: TiA@tobi-project.org
*/

#include "tia-private/newtia/messages_impl/tia_control_message_parser_1_0.h"
#include "tia-private/newtia/messages/tia_control_message_tags_1_0.h"
#include "tia-private/newtia/tia_exceptions.h"

#include <sstream>
#include <boost/algorithm/string.hpp>

using std::string;
using std::pair;

namespace tia
{

unsigned const TiAControlMessageParser10::MAX_LINE_LENGTH_ = 1000;

//-----------------------------------------------------------------------------
TiAControlMessage TiAControlMessageParser10::parseMessage (InputStream& stream) const
{
    string version = readVersion (stream);
    pair<string, string> command_and_parameter = readCommandAndParameter (stream);
    string content = readContent (stream);
    return TiAControlMessage (version, command_and_parameter.first,
                              command_and_parameter.second, content);
}

//-----------------------------------------------------------------------------
string TiAControlMessageParser10::readVersion (InputStream& stream) const
{
    string id_and_version = boost::algorithm::trim_copy(stream.readUntil(TiAControlMessageTags10::NEW_LINE_CHAR) );

    if (id_and_version != TiAControlMessageTags10::ID_AND_VERSION)
        throw TiAException (string ("TiAControlMessageParser10::readVersion Wrong Message ID and Version. Required \"") + TiAControlMessageTags10::ID_AND_VERSION + "\" but was \"" + id_and_version + "\".");

    return TiAControlMessageTags10::VERSION;
}

//-----------------------------------------------------------------------------
pair<string, string> TiAControlMessageParser10::readCommandAndParameter (InputStream& stream) const
{
    string line = boost::algorithm::trim_copy(stream.readUntil(TiAControlMessageTags10::NEW_LINE_CHAR));
    return getPair (line);
}

//-----------------------------------------------------------------------------
string TiAControlMessageParser10::readContent (InputStream& stream) const
{
    string line = boost::algorithm::trim_copy(stream.readUntil(TiAControlMessageTags10::NEW_LINE_CHAR) );
    if (line.size () == 0)
        return "";

    pair<string, string> content_length_pair = getPair (line);
    if (content_length_pair.first != TiAControlMessageTags10::CONTENT_LENGTH)
        throw TiAException (string ("TiAControlMessageParser10::readContent: Invalid field. Expected \"") + TiAControlMessageTags10::CONTENT_LENGTH + "\" but was \"" + content_length_pair.first + "\"");

    std::istringstream isstr (content_length_pair.second);
    size_t length = 0;
    isstr >> length;
    if (isstr.fail ())
        throw TiAException (string ("TiAControlMessageParser10::readContent: Error while converting \"") + content_length_pair.second + "\" into a number.");

    string empty_line = stream.readUntil(TiAControlMessageTags10::NEW_LINE_CHAR);
    if (empty_line.size ())
        throw TiAException (string ("TiAControlMessageParser10::readContent: Expecting an empty line before content starts."));

    string content = stream.readString (length);
    if (content.size () != length)
        throw TiAException (string ("TiAControlMessageParser10::readContent: Missing bytes!"));

    return content;
}


//-----------------------------------------------------------------------------
pair<string, string> TiAControlMessageParser10::getPair (std::string const& str) const
{
    size_t delimiter_index = 0;
    while ((str[delimiter_index] != TiAControlMessageTags10::COMMAND_DELIMITER) &&
           delimiter_index < str.size ())
        delimiter_index++;

    string tag = str.substr (0, delimiter_index);
    string value;
    if (delimiter_index < str.size () - 1)
        value = boost::algorithm::trim_copy(str.substr (delimiter_index + 1, str.size () + 1 - delimiter_index));

    return pair<string, string> (tag, value);
}


}

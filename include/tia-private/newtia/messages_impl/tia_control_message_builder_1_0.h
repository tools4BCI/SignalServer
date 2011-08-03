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

#ifndef TIA_CONTROL_MESSAGE_BUILDER_1_0_H
#define TIA_CONTROL_MESSAGE_BUILDER_1_0_H

#include "../messages/tia_control_message_tags_1_0.h"
#include "../messages/tia_control_message_builder.h"
#include "../tia_exceptions.h"

#include <sstream>

namespace tia
{

//-----------------------------------------------------------------------------
class TiAControlMessageBuilder10 : public TiAControlMessageBuilder
{
public:
    virtual std::string buildTiAMessage (TiAControlMessage const& message)
    {
        std::string str = TiAControlMessageTags10::ID + TiAControlMessageTags10::SPACE;
        str += message.getVersion ();
        str += TiAControlMessageTags10::NEW_LINE;

        str += message.getCommand ();
        if (message.getParameters().size())
        {
            str += TiAControlMessageTags10::COMMAND_DELIMITER;
            str += message.getParameters ();
        }
        str += TiAControlMessageTags10::NEW_LINE;

        if (message.getContent().size())
        {
            str += TiAControlMessageTags10::CONTENT_LENGTH;
            str += TiAControlMessageTags10::COMMAND_DELIMITER;

            std::ostringstream osstr;
            osstr << message.getContent().size();
            if (osstr.fail())
                throw TiAException ("TiAControlMessageBuilder10::buildTiAMessage failed to convert Content length into a string.");
            str += osstr.str();
            str += TiAControlMessageTags10::NEW_LINE;
        }

        str += TiAControlMessageTags10::NEW_LINE;
        str += message.getContent ();
        return str;
    }

};

}

#endif // TIA_CONTROL_MESSAGE_BUILDER_1_0_H

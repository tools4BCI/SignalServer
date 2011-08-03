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

#ifndef GET_SERVERSTATE_CONNECTION_COMMAND_H
#define GET_SERVERSTATE_CONNECTION_COMMAND_H

#include "../tia_control_command.h"
#include "../string_utils.h"
#include "../messages/tia_control_message_tags_1_0.h"

namespace tia
{

//-----------------------------------------------------------------------------
class GetServerStateConnectionCommand : public TiAControlCommand
{
public:
    GetServerStateConnectionCommand (unsigned state_server_port)
        : state_server_port_ (state_server_port)
    {}

    virtual ~GetServerStateConnectionCommand () {}

    virtual TiAControlMessage execute (TiAControlMessage const& command)
    {
        return TiAControlMessage (command.getVersion(), TiAControlMessageTags10::SERVER_STATE_CONNECTION_PORT,
                                  toString (state_server_port_), "");
    }

private:
    unsigned state_server_port_;
};

}

#endif // GET_SERVERSTATE_CONNECTION_COMMAND_H

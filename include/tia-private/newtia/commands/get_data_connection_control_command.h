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

#ifndef GET_DATA_CONNECTION_CONTROL_COMMAND_H
#define GET_DATA_CONNECTION_CONTROL_COMMAND_H

#include "../tia_control_command.h"
#include "../tia_control_command_context.h"
#include "../server/data_server.h"

namespace tia
{

//-----------------------------------------------------------------------------
class GetDataConnectionControlCommand : public TiAControlCommand
{
public:
    GetDataConnectionControlCommand (TiAControlCommandContext& command_context, DataServer& data_server)
        : command_context_ (command_context), data_server_ (data_server)
    {}

    virtual ~GetDataConnectionControlCommand () {}

    virtual TiAControlMessage execute (TiAControlMessage const& command);

private:
    TiAControlCommandContext& command_context_;
    DataServer& data_server_;
};

}

#endif // GET_DATA_CONNECTION_SERVER_COMMAND_H

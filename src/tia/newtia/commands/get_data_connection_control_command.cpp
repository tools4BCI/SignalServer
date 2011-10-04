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

#include "tia-private/newtia/commands/get_data_connection_control_command.h"
#include "tia-private/newtia/messages/tia_control_message_tags_1_0.h"
#include "tia-private/newtia/messages/standard_control_messages.h"

#include <sstream>
#include <iostream>

using std::string;

namespace tia
{

//-------------------------------------------------------------------------------------------------
TiAControlMessage GetDataConnectionControlCommand::execute (TiAControlMessage const& command)
{
    string version = command.getVersion ();

    string command_response = TiAControlMessageTags10::DATA_CONNECTION_PORT;

    string parameter;
    string content;

    ConnectionID data_connection;
    if (command.getParameters() == TiAControlMessageTags10::UDP)
    {
      std::string remote(command.getRemoteEndpointIP () );
      size_t pos = remote.rfind ('.');
      remote.erase (pos);

      std::string target(data_server_.getTargetIP () );
      pos = target.rfind ('.');
      target.erase (pos);

      if(target != remote)
        return CustomErrorControlMessage (version,
                                          "Target and remote subnet do not match!");
      else
        data_connection = data_server_.addConnection (true);
    }
    else if (command.getParameters() == TiAControlMessageTags10::TCP)
        data_connection = data_server_.addConnection (false);
    else
        return ErrorControlMessage (version);

    command_context_.addDataConnectionID (data_connection);
    std::ostringstream oss;
    oss << data_server_.localPort (data_connection);
    parameter = oss.str ();

    return TiAControlMessage (version, command_response, parameter, content);
}

}

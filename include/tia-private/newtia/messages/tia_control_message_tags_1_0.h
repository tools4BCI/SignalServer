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

#ifndef TIA_CONTROL_MESSAGE_TAGS_1_0_H
#define TIA_CONTROL_MESSAGE_TAGS_1_0_H

#include <string>

namespace tia
{

namespace TiAControlMessageTags10
{
    char const NEW_LINE_CHAR = 0x0A;
    std::string const NEW_LINE (1, NEW_LINE_CHAR);
    char const COMMAND_DELIMITER = ':';


    std::string const VERSION = "1.0";
    std::string const ID = "TiA";
    std::string const SPACE = " ";
    std::string const ID_AND_VERSION = ID + std::string (" ") + VERSION;

    std::string const CONTENT_LENGTH = "Content-Length";

    std::string const OK = "OK";
    std::string const ERROR_STR = "Error";
    std::string const METAINFO = "MetaInfo";
    std::string const CHECK_PROTOCOL_VERSION = "CheckProtocolVersion";
    std::string const GET_METAINFO = "GetMetaInfo";
    std::string const DATA_CONNECTION_PORT = "DataConnectionPort";
    std::string const SERVER_STATE_CONNECTION_PORT = "ServerStateConnectionPort";
    std::string const GET_SERVER_STATE_CONNECTION = "GetServerStateConnection";
    std::string const GET_DATA_CONNECTION = "GetDataConnection";
    std::string const START_DATA_TRANSMISSION = "StartDataTransmission";
    std::string const STOP_DATA_TRANSMISSION = "StopDataTransmission";

    std::string const SERVER_STATE_RUNNING = "ServerStateRunning";
    std::string const SERVER_STATE_SHUTDOWN = "ServerStateShutdown";

    std::string const TCP = "TCP";
    std::string const UDP = "UDP";

}

}

#endif // TIA_CONTROL_MESSAGE_TAGS_1_0_H

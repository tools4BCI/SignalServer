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

#ifndef TIA_CONTROL_MESSAGE_H
#define TIA_CONTROL_MESSAGE_H

#include <string>

namespace tia
{

//-----------------------------------------------------------------------------
class TiAControlMessage
{
public:
    TiAControlMessage (std::string const& version, std::string const& command,
                       std::string const& parameters, std::string const& content)
                           : version_ (version), command_ (command),
                             parameters_ (parameters), content_ (content),
                             remote_endpoint_ip_("0.0.0.0")
    {}
    virtual ~TiAControlMessage () {}

    void setRemoteEndpointIP (std::string ip) {remote_endpoint_ip_ = ip;}

    std::string getVersion () const {return version_;}
    std::string getCommand () const {return command_;}
    std::string getParameters () const {return parameters_;}
    std::string getContent () const {return content_;}
    std::string getRemoteEndpointIP () const {return remote_endpoint_ip_;}

private:
    std::string version_;
    std::string command_;
    std::string parameters_;
    std::string content_;
    std::string remote_endpoint_ip_;
};

}

#endif // TIA_CONTROL_MESSAGE_H

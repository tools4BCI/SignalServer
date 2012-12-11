/*
    This file is part of the TOBI SignalServer.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU General Public License Usage
    Alternatively, this file may be used under the terms of the GNU
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file gpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/gpl.html.

    In case of GNU General Public License Usage ,the TOBI SignalServer
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI SignalServer. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2012 Graz University of Technology
    Contact: SignalServer@tobi-project.org
*/

/**
* @file cml_tags.h
*
* @brief xml_tags contains static xml strings used within the signalserver
*
*
**/

#ifndef XMLTAGS_H
#define XMLTAGS_H

#include <string>


//  -->  TIA_META_INFO_PARSE_AND_BUILD_FUNCTIONS_H

namespace tobiss
{

namespace xmltags
{
static const std::string sigserver_config("tobi-config");
static const std::string server_settings("server_settings");

static const std::string ctl_port("ctl_port");
static const std::string udp_bc_addr("udp_bc_addr");
static const std::string udp_port("udp_port");

static const std::string tid_server("tid_server");
static const std::string tid_use("tid_use");
static const std::string tid_port("tid_port");
static const std::string tid_assume_zero_network_delay("assume_zero_network_delay");

static const std::string subject = "subject";
static const std::string subject_id = "id";
static const std::string subject_firstname = "first_name";
static const std::string subject_surname = "surname";
static const std::string subject_birthday = "birthday";
static const std::string subject_sex = "sex";

static const std::string hardware = "hardware";
static const std::string hardware_name = "name";


static const std::string store_data("store-data");
static const std::string store_data_value("value");
static const std::string filename("filename");
static const std::string filetype("filetype");
static const std::string filepath("filepath");
static const std::string filepath_default("rec");

static const std::string file_exists("file_exists");
static const std::string file_exists_overwrite("overwrite");
static const std::string file_exists_new_file("new_file");

static const std::string append_to_filename("append_to_filename");
static const std::string append_to_filename_default("_run_");
static const std::string continous_saving("continous_saving");

//static const std::string file_reader("file-reader");
  //filepath, name and type from store_data
//static const std::string fr_speedup("speedup");
//static const std::string fr_stop("stop_at_end");


}

} // Namespace tobiss

//---------------------------------------------------------------------------------------
#endif // XMLTAGS_H

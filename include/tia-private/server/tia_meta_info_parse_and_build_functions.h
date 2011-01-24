#ifndef TIA_META_INFO_PARSE_AND_BUILD_FUNCTIONS_H
#define TIA_META_INFO_PARSE_AND_BUILD_FUNCTIONS_H

#include "tia/ssconfig.h"

namespace tia
{

//-----------------------------------------------------------------------------
tobiss::SSConfig parseTiAMetaInfoFromXMLString (std::string const& tia_meta_info_xml_string);

//-----------------------------------------------------------------------------
std::string buildTiAMetaInfoXMLString (tobiss::SSConfig const& tia_meta_info);


}

#endif // TIA_META_INFO_PARSE_AND_BUILD_FUNCTIONS_H

#include <string>

#include "gawrs_face/common/version.h"

#define STR(s) #s

#define VERSION_CAT(a, b, c) STR(a) "." STR(b) "." STR(c)
#define CORE_VERSION VERSION_CAT(CORE_MAJOR_VERSION, CORE_MINOR_VERSION, CORE_PATCH_VERSION)

std::string __gawrs_face_version = "core version: " CORE_VERSION "; build data: " __TIME__ ", " __DATE__;
const char* coreVersion = CORE_VERSION;
const char* buildDate = __TIME__ ", " __DATE__;
const char* copyRight = "All Rights Reserved.";

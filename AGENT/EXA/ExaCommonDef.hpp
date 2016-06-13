#ifndef __EXA_COMMON_DEF_HPP__
#define __EXA_COMMON_DEF_HPP__

#include <string>

#include "CommonCode.hpp"
#include "CommandFormat.hpp"

#include "CFileLog.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

namespace vnfm {
    enum eStatusCode: int
    {
        eOk             = 200,
        eBadRequest     = 400,
        eForbidden      = 403,
        eNotFound       = 404,
        eInternal       = 500
    };

    const std::string   Ok("Ok");
    const std::string   BadRequest("Bad Request");
    const std::string   Forbidden("Forbidden");
    const std::string   NotFound("Not Found");
    const std::string   Internal("Internal Server Error");
    const std::string   Unknown("Unknown");


    const int nDEF_BUF_SIZE = 256;
}

#endif // __EXA_COMMON_DEF_HPP__

#ifndef __CCLIAPI_HPP__
#define __CCLIAPI_HPP__

#include <string>
#include "CRabbitAlloc.hpp"

class CCliApi : public CRabbitAlloc{
    protected:
        std::string m_strErrString;

    public:
        static const int RESULT_OK  = 1;
        static const int RESULT_NOK = 2;
        static const int RESULT_PARSING_ERROR = 3;

        std::string& GetStrError() { return m_strErrString; };
};

#endif

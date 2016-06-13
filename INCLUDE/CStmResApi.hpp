#ifndef __CSTARESAPI_HPP__
#define __CSTARESAPI_HPP__

#include <string>
#include "CRabbitAlloc.hpp"

class CStmResApi : public CRabbitAlloc{
		protected:
				int m_nSessionId;
				time_t m_nCollectTime;
		public:
				std::string m_strErrString;
				enum eResult{
						RESAPI_RESULT_OK = 1,
						RESAPI_RESULT_NOK = 2,
						RESAPI_RESULT_INVALID_MESSGAE_LEN = 3,
						RESAPI_RESULT_BUFFER_TOO_SMALL = 4,
						RESAPI_RESULT_INVALID_TABLE_TYPE = 5,
						RESAPI_RESULT_PARSING_ERROR = 6,
						RESAPI_RESULT_VALUE_NOT_EXIST = 7
				};

				enum eReslutCode{
						RESAPI_RESULT_CODE_SUCCESS = 1,
						RESAPI_RESULT_CODE_DATA_NOT_EXIST = 2
				};

				std::string GetStrError() { return m_strErrString; }
};

#endif

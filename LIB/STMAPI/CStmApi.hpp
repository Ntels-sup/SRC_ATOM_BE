#ifndef __STMAPI_HPP__
#define __STMAPI_HPP__

#include <string>

#define STMAPI_ERR 1

#define STMAPI_LOG(_LVL,...)


class CStmApi{
		protected:
				std::string m_strErrString;

		public:
				enum eResult{
						RESULT_OK = 1,
						RESULT_NOK = 2,
						RESULT_INVALID_MESSGAE_LEN = 3,
						RESULT_BUFFER_TOO_SMALL = 4,
						RESULT_INVALID_TABLE_TYPE = 5,
						RESULT_PARSING_ERROR = 6,
						RESULT_VALUE_NOT_EXIST = 7
				};

				enum eTableType{
						TABLE_TYPE_STS = 1,
						TABLE_TYPE_HIST = 2
						//TABLE_TYPE_RES = 3
				};

				enum eResultCode{
						RESULT_CODE_SUCCESS = 1,
						RESULT_CODE_TABLE_NOT_EXIST = 2,
						RESULT_CODE_DATA_NOT_EXIST = 3,
						RESULT_CODE_UNKNOWN_ERROR = 4
				};

		std::string GetStrError() { return m_strErrString; }

};

#endif

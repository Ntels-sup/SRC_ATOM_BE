#ifndef __CCLIREGAPI_HPP_
#define __CCLIREGAPI_HPP_
#include <string>

class CCliRegApi{
	public:
		static const int RESULT_OK = 0;
		static const int RESULT_NOK = 1;
		static const int RESULT_PARSING_ERROR = 2;
		std::string	m_strErrString;
};

#endif

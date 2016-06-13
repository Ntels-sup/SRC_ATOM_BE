#ifndef __CREGAPI_HPP_
#define __CREGAPI_HPP_
#include <string>

class CRegApi{
		public:
				enum {
						OK = 0,
						NOK = 1,
						PARSING_ERROR = 2
				};
				std::string	m_strErrString;
};

#endif

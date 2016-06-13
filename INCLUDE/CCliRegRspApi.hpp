#ifndef __CCLIREGRSPAPI_HPP__
#define __CCLIREGRSPAPI_HPP__
#include <vector>
#include <string>
#include "CCliRegApi.hpp"

class CCliRegRspApi : public CCliRegApi {
		private:
				int m_nCode;
		public: 
				static const int RESULT_CODE_REG_SUCCESS = 0;
				static const int RESULT_CODE_REG_FAILED = 1;
				CCliRegRspApi();
				~CCliRegRspApi();
				int Init(int a_nCode);
				int GetCode() { return m_nCode; };
				int DecodeMessage(std::vector<char>& a_vecPayload);
				int DecodeMessage(std::string &a_strPayload);
				int EncodeMessage(std::vector<char>& a_vecPayload);	
				int EncodeMessage(std::string &a_strPayload);
};

#endif

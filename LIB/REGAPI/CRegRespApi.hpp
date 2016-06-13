#ifndef __CREGRESPAPI_HPP__
#define __CREGRESPAPI_HPP__
#include <vector>
#include <string>
#include "CRegApi.hpp"

class CRegRespApi : public CRegApi {
		private:
				int m_nCode;
				std::string m_strText;
		public: 
				CRegRespApi();
				~CRegRespApi();
				int Init(int a_nCode, char *szText);
				int Init(int a_nCode, std::string &szText);
				int GetCode() { return m_nCode; };
				std::string GetText() { return m_strText; };
				int DecodeMessage(std::vector<char>& a_vecPayload);
				int DecodeMessage(std::string &a_strPayload);
				int EncodeMessage(std::vector<char>& a_vecPayload);	
				int EncodeMessage(std::string &a_strPayload);
};

#endif

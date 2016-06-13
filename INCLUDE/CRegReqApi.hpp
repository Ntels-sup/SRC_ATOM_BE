#ifndef __CREGREQAPI_HPP__
#define __CREGREQAPI_HPP__
#include <vector>
#include <string>
#include "CRegApi.hpp"

class CRegReqApi : public CRegApi {
		private:
				std::string m_strPkgName;
				std::string m_strNodeType;
				std::string m_strProcName;
				unsigned int m_nProcNo;
		public:
				CRegReqApi();
				~CRegReqApi();
				int Init(std::string &a_strPkgName, std::string &a_strNodeType, std::string &a_strProcName, int a_nProcNo);
				int Init(char *a_szPkgName, char *a_szNodeType, char *a_szProcName, int a_nProcNo);
				std::string GetPkgName() { return m_strPkgName; };
				std::string GetNodeType() { return m_strNodeType; };
				std::string GetProcName() { return m_strProcName; };
				int GetProcNo() { return m_nProcNo; };
				int DecodeMessage(std::vector<char>& a_vecPayload);
				int DecodeMessage(std::string &a_strPayload);
				int EncodeMessage(std::vector<char>& a_vecPayload);	
				int EncodeMessage(std::string &a_strPayload);
};

#endif


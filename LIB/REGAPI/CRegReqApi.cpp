#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include "CRegReqApi.hpp"

CRegReqApi::CRegReqApi()
{
		m_nProcNo = 0;
}

CRegReqApi::~CRegReqApi()
{
}


int CRegReqApi::Init(char *a_szPkgName, char *a_szNodeType, char *a_szProcName, int a_nProcNo)
{
		m_strPkgName = a_szPkgName;
		m_strNodeType = a_szNodeType;
		m_strProcName = a_szProcName;
		m_nProcNo = a_nProcNo;

		return CRegApi::OK;
}

int CRegReqApi::Init(std::string &a_strPkgName, std::string &a_strNodeType, std::string &a_strProcName, int a_nProcNo)
{

		m_strPkgName = a_strPkgName;
		m_strNodeType = a_strNodeType;
		m_strProcName = a_strProcName;
		m_nProcNo = a_nProcNo;

		return CRegApi::OK;
}

int CRegReqApi::DecodeMessage(std::vector<char>& a_vecPayload)
{
		int nRet = 0;
		std::string  strPayload; 

		strPayload = &a_vecPayload[0];

		nRet = DecodeMessage(strPayload);

		return nRet;
}

int CRegReqApi::DecodeMessage(std::string &a_strPayload)
{
		try{
				rabbit::document cDoc;
				rabbit::object cBody;

				cDoc.parse(a_strPayload);

				cBody = cDoc["BODY"];

				m_strPkgName = cBody["pkgname"].as_string();
				m_strNodeType = cBody["nodetype"].as_string();

				m_strProcName = cBody["procname"].as_string();
				m_nProcNo = cBody["procno"].as_int();

		} catch(rabbit::type_mismatch   e) {
				m_strErrString = e.what();
				return PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString = e.what();
				return PARSING_ERROR;
		} catch(...) {
				m_strErrString = "Unknown Error";
				return PARSING_ERROR;
		}

		return CRegApi::OK;
}

int CRegReqApi::EncodeMessage(std::vector<char>& a_vecPayload)
{
		int nRet = CRegApi::OK;
		std::string strPayload;

		nRet = EncodeMessage(strPayload);
		if(nRet != CRegApi::OK){
				return nRet;
		}

		a_vecPayload.clear();
		a_vecPayload.insert(a_vecPayload.end(), strPayload.begin(), strPayload.end());

		return CRegApi::OK;
}

int CRegReqApi::EncodeMessage(std::string &a_strPayload)
{
		try{
				rabbit::object cRoot;
				rabbit::object cBody = cRoot["BODY"];

				cBody["pkgname"] = m_strPkgName;
				cBody["nodetype"] = m_strNodeType;


				cBody["procname"] = m_strProcName;
				cBody["procno"] = m_nProcNo;

				a_strPayload = cRoot.str();
				//strncpy(a_chData, cRoot.str().c_str(), cRoot.str().size());

				//*a_nDataLen = cRoot.str().size();
		} catch(rabbit::type_mismatch   e) {
				m_strErrString = e.what();
				return PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString = e.what();
				return PARSING_ERROR;
		} catch(...) {
				m_strErrString = "Unknown Error";
				return PARSING_ERROR;
		}

		return CRegApi::OK;
}


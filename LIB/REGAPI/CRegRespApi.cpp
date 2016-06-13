#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include "CRegRespApi.hpp"

CRegRespApi::CRegRespApi()
{
		m_nCode = 0;
}

CRegRespApi::~CRegRespApi()
{
}

int CRegRespApi::Init(int a_nCode, char *szText)
{
		m_nCode = a_nCode; 
		m_strText = szText;

		return CRegApi::OK;
}

int CRegRespApi::Init(int a_nCode, std::string &szText)
{
		m_nCode = a_nCode; 
		m_strText = szText;

		return CRegApi::OK;
}

int CRegRespApi::DecodeMessage(std::vector<char>& a_vecPayload)
{
		int nRet = 0;
		std::string  strPayload; 

		strPayload = &a_vecPayload[0];

		nRet = DecodeMessage(strPayload);

		return nRet;
}

int CRegRespApi::DecodeMessage(std::string &a_strPayload)
{
		try{
				rabbit::document cDoc;
				rabbit::object cBody;

				cDoc.parse(a_strPayload);

				cBody = cDoc["BODY"];

				m_nCode = cBody["code"].as_int();
				m_strText = cBody["text"].as_string();

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

int CRegRespApi::EncodeMessage(std::vector<char>& a_vecPayload)
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

int CRegRespApi::EncodeMessage(std::string &a_strPayload)
{
		try{
				rabbit::object cRoot;
				rabbit::object cBody = cRoot["BODY"];

				cBody["code"] = m_nCode;
				cBody["text"] = m_strText;

				a_strPayload = cRoot.str();
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


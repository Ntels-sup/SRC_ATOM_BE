#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include "CCliRegReqApi.hpp"

CCliRegReqApi::CCliRegReqApi()
{
}

CCliRegReqApi::~CCliRegReqApi()
{
}

int CCliRegReqApi::Init(char *a_szUserId)
{
		m_strUserId = a_szUserId;

		return CCliRegApi::RESULT_OK;
}

int CCliRegReqApi::Init(std::string &a_strUserId)
{

		m_strUserId = a_strUserId;

		return CCliRegApi::RESULT_OK;
}

int CCliRegReqApi::DecodeMessage(std::vector<char>& a_vecPayload)
{
		int nRet = 0;
		std::string  strPayload; 

		strPayload = &a_vecPayload[0];

		nRet = DecodeMessage(strPayload);

		return nRet;
}

int CCliRegReqApi::DecodeMessage(std::string &a_strPayload)
{
		try{
				rabbit::document cDoc;
				rabbit::object cBody;

				cDoc.parse(a_strPayload);

				cBody = cDoc["BODY"];

				m_strUserId = cBody["userid"].as_string();

		} catch(rabbit::type_mismatch   e) {
				m_strErrString = e.what();
				return CCliRegApi::RESULT_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString = e.what();
				return CCliRegApi::RESULT_PARSING_ERROR;
		} catch(...) {
				m_strErrString = "Unknown Error";
				return CCliRegApi::RESULT_PARSING_ERROR;
		}

		return CCliRegApi::RESULT_OK;
}

int CCliRegReqApi::EncodeMessage(std::vector<char>& a_vecPayload)
{
		int nRet = CCliRegApi::RESULT_OK;
		std::string strPayload;

		nRet = EncodeMessage(strPayload);
		if(nRet != CCliRegApi::RESULT_OK){
				return nRet;
		}

		a_vecPayload.clear();
		a_vecPayload.insert(a_vecPayload.end(), strPayload.begin(), strPayload.end());

		return CCliRegApi::RESULT_OK;
}

int CCliRegReqApi::EncodeMessage(std::string &a_strPayload)
{
		try{
				rabbit::object cRoot;
				rabbit::object cBody = cRoot["BODY"];

				cBody["userid"] = m_strUserId;

				a_strPayload = cRoot.str();
				//strncpy(a_chData, cRoot.str().c_str(), cRoot.str().size());

				//*a_nDataLen = cRoot.str().size();
		} catch(rabbit::type_mismatch   e) {
				m_strErrString = e.what();
				return CCliRegApi::RESULT_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString = e.what();
				return CCliRegApi::RESULT_PARSING_ERROR;
		} catch(...) {
				m_strErrString = "Unknown Error";
				return CCliRegApi::RESULT_PARSING_ERROR; }

		return CCliRegApi::RESULT_OK;
}


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include "CCliRegRspApi.hpp"

CCliRegRspApi::CCliRegRspApi()
{
		m_nCode = 0;
}

CCliRegRspApi::~CCliRegRspApi()
{
}

int CCliRegRspApi::Init(int a_nCode)
{
		m_nCode = a_nCode; 

		return CCliRegApi::RESULT_OK;
}

int CCliRegRspApi::DecodeMessage(std::vector<char>& a_vecPayload)
{
		int nRet = 0;
		std::string  strPayload; 

		strPayload = &a_vecPayload[0];

		nRet = DecodeMessage(strPayload);

		return nRet;
}

int CCliRegRspApi::DecodeMessage(std::string &a_strPayload)
{
		try{
				rabbit::document cDoc;
				rabbit::object cBody;

				cDoc.parse(a_strPayload);

				cBody = cDoc["BODY"];

				m_nCode = cBody["code"].as_int();

		} catch(rabbit::type_mismatch   e) {
				m_strErrString = e.what();
				return RESULT_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString = e.what();
				return RESULT_PARSING_ERROR;
		} catch(...) {
				m_strErrString = "Unknown Error";
				return RESULT_PARSING_ERROR;
		}

		return CCliRegApi::RESULT_OK;
}

int CCliRegRspApi::EncodeMessage(std::vector<char>& a_vecPayload)
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

int CCliRegRspApi::EncodeMessage(std::string &a_strPayload)
{
		try{
				rabbit::object cRoot;
				rabbit::object cBody = cRoot["BODY"];

				cBody["code"] = m_nCode;

				a_strPayload = cRoot.str();
		} catch(rabbit::type_mismatch   e) {
				m_strErrString = e.what();
				return RESULT_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString = e.what();
				return RESULT_PARSING_ERROR;
		} catch(...) {
				m_strErrString = "Unknown Error";
				return RESULT_PARSING_ERROR;
		}

		return CCliRegApi::RESULT_OK;
}


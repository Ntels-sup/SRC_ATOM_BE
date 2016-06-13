#include <stdio.h>
#include <stdarg.h>
#include "CCliRsp.hpp"

CCliRsp::CCliRsp()
{
	Init(0, SEQ_TYPE_END, 0);
}

CCliRsp::CCliRsp(unsigned int a_nSessionId)
{
	Init(a_nSessionId, SEQ_TYPE_END, 0);
}

CCliRsp::CCliRsp(unsigned int a_nSessionId, int a_nSeqType)
{
	Init(a_nSessionId, a_nSeqType, 0);
}

CCliRsp::CCliRsp(unsigned int a_nSessionId, int a_nSeqType, unsigned int a_nResultCode)
{
	Init(a_nSessionId, a_nSeqType, a_nResultCode);
}

void CCliRsp::Init(unsigned int a_nSessionId, int a_nSeqType, unsigned int a_nResultCode)
{
	m_nSessionId = a_nSessionId;
	SetSeqType(a_nSeqType);
	m_nResultCode = a_nResultCode;
	m_nMaxBufferLen = 0;
	m_szBuffer = NULL;
}

CCliRsp::~CCliRsp()
{
	if(m_szBuffer != NULL){
		delete[] m_szBuffer;
	}
}

void CCliRsp::SetSeqType(int a_nSeqType)
{

	if((a_nSeqType != SEQ_TYPE_CONTINUE) &&
			(a_nSeqType != SEQ_TYPE_END)) {
		m_nSeqType = SEQ_TYPE_END;
	}

	m_nSeqType = a_nSeqType;
}

void CCliRsp::ResetHeader(unsigned int a_nSessionId, int a_nSeqType, unsigned int a_nResultCode)
{
	m_nSessionId = a_nSessionId;
	m_nSeqType = a_nSeqType;
	m_nResultCode = a_nResultCode;
}

void CCliRsp::SetText(std::string &a_strText)
{
	m_strText = a_strText;
}

void CCliRsp::SetText(const char *a_szText)
{
	m_strText = a_szText;
}

void CCliRsp::SetTextAppend(std::string &a_strText)
{
	m_strText.append(a_strText);
}

void CCliRsp::SetTextAppend(const char *a_szText)
{
	m_strText.append(a_szText);
}

void CCliRsp::NPrintf(unsigned int a_nMaxLength, const char *fmt,...)
{
	va_list ap;

	if(m_nMaxBufferLen <= a_nMaxLength){
		if(m_szBuffer != NULL){
			delete[] m_szBuffer;
		}
		m_szBuffer = new char[a_nMaxLength];
	}

	va_start(ap,fmt);

	vsnprintf(m_szBuffer, a_nMaxLength, fmt, ap);

	va_end(ap);

	m_strText.append(m_szBuffer, strlen(m_szBuffer));
}

void CCliRsp::ClearAll()
{
	m_nSessionId = 0;
	m_nSeqType = SEQ_TYPE_END;
	m_nResultCode = 0;

	m_strText.clear();
}

void CCliRsp::ClearData()
{
	m_strText.clear();
}

int CCliRsp::EncodeMessage(std::vector<char>& a_vecData)
{
	int nRet = RESULT_OK;
	std::string strData;

	nRet = EncodeMessage(strData);
	if(nRet != RESULT_OK){
		return nRet;
	}

	a_vecData.clear();
	a_vecData.insert(a_vecData.end(), strData.begin(), strData.end());

	return RESULT_OK;
}

int CCliRsp::EncodeMessage(std::string &a_strData)
{
	try {
		rabbit::object cRoot;
		rabbit::object cBody = cRoot["BODY"];

		cBody["session_id"] = m_nSessionId;
		cBody["seq_type"] = m_nSeqType;
		cBody["result_code"] = m_nResultCode;
		cBody["result"] = m_strResult;
		cBody["text"] = m_strText;

		a_strData = cRoot.str();

	} catch(rabbit::type_mismatch   e) {
		m_strErrString.append(e.what());
		return CCliApi::RESULT_PARSING_ERROR;
	} catch(rabbit::parse_error e) {
		m_strErrString.append(e.what());
		return CCliApi::RESULT_PARSING_ERROR;
	} catch(...) {
		m_strErrString.append("Unknown Error");
		return CCliApi::RESULT_PARSING_ERROR;
	}

	return RESULT_OK;
}

int CCliRsp::DecodeMessage(std::vector<char>& a_vecData)
{
	int nRet = 0;
	std::string  strPayload;

	//strPayload = &a_vecData[0];
	
	strPayload.append(&a_vecData[0], a_vecData.size());

	nRet = DecodeMessage(strPayload);

	return nRet;
}

int CCliRsp::DecodeMessage(std::string &a_strData)
{
	try {
		rabbit::document doc;
		rabbit::object cBody;
		rabbit::array cArgArray;
		doc.parse(a_strData);

		cBody = doc["BODY"];

		m_nSessionId = cBody["session_id"].as_uint();
		m_nSeqType = cBody["seq_type"].as_uint();
		m_nResultCode = cBody["result_code"].as_uint();
		m_strResult = cBody["result"].as_string();
		m_strText = cBody["text"].as_string();

	} catch(rabbit::type_mismatch   e) {
		m_strErrString.append(e.what());
		return CCliApi::RESULT_PARSING_ERROR;
	} catch(rabbit::parse_error e) {
		m_strErrString.append(e.what());
		return CCliApi::RESULT_PARSING_ERROR;
	} catch(...) {
		m_strErrString.append("Unknown Error");
		return CCliApi::RESULT_PARSING_ERROR;
	}

	return RESULT_OK;
}


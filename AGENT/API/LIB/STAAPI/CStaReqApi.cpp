#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include "STAAPI.h"
#include "CStaReqApi.hpp"

CStaReqApi::CStaReqApi()
{
}

CStaReqApi::~CStaReqApi()
{
}

time_t CStaReqApi::GetTime()
{
		return m_collectTime;
}

/* message format 
  time[8]
 */
int CStaReqApi::DecodeMessage(std::string &a_strData)
{
		try {
				rabbit::document doc;
				rabbit::object cBody;
				doc.parse(a_strData);

				cBody = doc["BODY"];
				m_collectTime= cBody["COLLECT_TIME"].as_int64();
		} catch(rabbit::type_mismatch   e) {
				m_strParsingError.append(e.what());
				return STAAPI_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strParsingError.append(e.what());
				return STAAPI_PARSING_ERROR;
		} catch(...) {
				m_strParsingError.append("Unknown Error");
				return STAAPI_PARSING_ERROR;
		}

		return STAAPI_OK;
}

int CStaReqApi::DecodeMessage(char *a_chData, unsigned int a_nDataLen)
{
		int nRet = STAAPI_OK;
		std::string strDecodeData;

		strDecodeData.append(a_chData, a_nDataLen);

		nRet = DecodeMessage(strDecodeData);
		if(nRet != STAAPI_OK){
				return nRet;
		}


		return STAAPI_OK;
}

int CStaReqApi::EncodeMessage(std::string &a_chData)
{
		try{
				rabbit::object cRoot;
				rabbit::object cBody = cRoot["BODY"];

				cBody["COLLECT_TIME"] = (uint64_t)m_collectTime;

				a_chData = cRoot.str();
		} catch(rabbit::type_mismatch   e) {
				m_strParsingError.append(e.what());
				return STAAPI_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strParsingError.append(e.what());
				return STAAPI_PARSING_ERROR;
		} catch(...) {
				m_strParsingError.append("Unknown Error");
				return STAAPI_PARSING_ERROR;
		}

		return STAAPI_OK;
}

int CStaReqApi::EncodeMessage(char *a_chData, unsigned int a_nMaxData, unsigned int *a_nDataLen)
{
		int nRet = STAAPI_OK;
		std::string strData;

		nRet = EncodeMessage(strData);
		if(nRet != STAAPI_OK){
				return nRet;
		}

		if(strData.size() > a_nMaxData){
				return STAAPI_BUFFER_TOO_SMALL;
		}

		strncpy(a_chData, strData.c_str(), strData.size());

		*a_nDataLen = strData.size();

		return STAAPI_OK;
}

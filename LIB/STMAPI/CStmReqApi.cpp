#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
//#include "STMAPI.h"
#include "CStmReqApi.hpp"

CStmReqApi::CStmReqApi()
{
		m_nSessionId = 0;
		m_blnActFlag = true;
		m_startTime = 0;
		m_nCollectTime = 0;
}

CStmReqApi::~CStmReqApi()
{
}

CStmApi::eResult CStmReqApi::Init(int a_nSessionId, time_t a_startTime, int a_nCollectTime)
{
		m_nSessionId = a_nSessionId;
		m_startTime = a_startTime;
		m_nCollectTime = a_nCollectTime;

		return RESULT_OK;
}

/* Req
 * SessionId[4]
 * ActFlag[4]
 * StartTime[8]
 * CollectTime[4]
 * TableName[tlv]
 */
CStmApi::eResult CStmReqApi::DecodeMessage(char *a_chData, int a_nDataLen)
{
		std::string strActFlag;
		std::string strDecodeData;

		strDecodeData.append(a_chData, a_nDataLen);

		try{
				rabbit::document doc;
				rabbit::object cBody;

				doc.parse(strDecodeData);

				cBody = doc["BODY"];

				m_nSessionId = cBody["session_id"].as_int();
				strActFlag = cBody["act_flag"].as_string();
				if(strActFlag.compare("TRUE") == 0){
						m_blnActFlag = true;
				}
				else {
						m_blnActFlag = false;
				}

				m_startTime = cBody["start_time"].as_uint64();
				m_nCollectTime = cBody["collect_time"].as_int();

				m_strTableName = cBody["table_name"].as_string();
		} catch(rabbit::type_mismatch   e) {
				m_strErrString.append(e.what());
				return RESULT_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString.append(e.what());
				return RESULT_PARSING_ERROR;
		} catch(...) {
				m_strErrString.append("Unknown Error");
				return RESULT_PARSING_ERROR;
		}

		return RESULT_OK;
}

/* Req
 * SessionId[4]
 * ActFlag[4]
 * StartTime[8]
 * CollectTime[4]
 * TableName[tlv]
 */
CStmApi::eResult CStmReqApi::EncodeMessage(std::string &a_strData)
{
		try{
				rabbit::object cRoot;
				rabbit::object cBody = cRoot["BODY"];

				cBody["session_id"] = m_nSessionId;

				if(m_blnActFlag == true){
						cBody["act_flag"] = "TRUE";
				}
				else {
						cBody["act_flag"] = "FALSE";
				}

				cBody["start_time"] = (uint64_t)m_startTime;
				cBody["collect_time"] = (unsigned int)m_nCollectTime;

				cBody["table_name"] = m_strTableName;


				a_strData = cRoot.str();
		
		} catch(rabbit::type_mismatch   e) {
				m_strErrString.append(e.what());
				return RESULT_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString.append(e.what());
				return RESULT_PARSING_ERROR;
		} catch(...) {
				m_strErrString.append("Unknown Error");
				return RESULT_PARSING_ERROR;
		}

		return RESULT_OK;
}

CStmApi::eResult CStmReqApi::EncodeMessage(char *a_chData, unsigned int a_nMaxLen, unsigned int *a_nDataLen)
{
		CStmApi::eResult nRet = RESULT_OK;
		std::string strData;

		nRet = EncodeMessage(strData);
		if(nRet != RESULT_OK){
				return nRet;
		}

		if(strData.size() > a_nMaxLen){
				return RESULT_BUFFER_TOO_SMALL;
		}

		strncpy(a_chData, strData.c_str(), strData.size());

		*a_nDataLen = strData.size();

		return RESULT_OK;
}


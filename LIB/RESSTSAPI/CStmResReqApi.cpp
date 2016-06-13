#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include "CStmResReqApi.hpp"

CStmResReqApi::CStmResReqApi()
{
		m_lstRabbitObject = NULL;
}

CStmResReqApi::~CStmResReqApi()
{
        std::list<rabbit::object*>::iterator  objectIter;

        if(m_lstRabbitObject != NULL){
                for(objectIter = m_lstRabbitObject->begin();objectIter != m_lstRabbitObject->end();){
                        delete *objectIter;
                        m_lstRabbitObject->erase(objectIter++);
                }

                delete m_lstRabbitObject;
        }
}

rabbit::object *CStmResReqApi::GetRabbitObject()
{
        rabbit::object *cObject = NULL;

        if(m_lstRabbitObject == NULL){
                m_lstRabbitObject = new std::list<rabbit::object*>;
        }


        cObject = new rabbit::object;

        m_lstRabbitObject->push_back(cObject);

        return cObject;
}

int CStmResReqApi::Init(int a_nSessionId, time_t a_nCollectTime)
{
		m_nSessionId = a_nSessionId;
		m_nCollectTime = a_nCollectTime;

		return RESAPI_RESULT_OK;
}

int CStmResReqApi::InsertRscGrpId(char *a_szRscGrpId)
{
		std::string strResGrpId;

		strResGrpId = a_szRscGrpId;

		return InsertRscGrpId(strResGrpId);
}

int CStmResReqApi::InsertRscGrpId(std::string a_strRscGrpId)
{
		m_lstRscGrpId.push_back(a_strRscGrpId);

		return CStmResApi::RESAPI_RESULT_OK;
}

int CStmResReqApi::RscGrpIdSize()
{
		return m_lstRscGrpId.size();
}

std::string CStmResReqApi::GetFirstRscGrpId()
{
		std::string strResGrpId;

		if(m_lstRscGrpId.size() == 0){
				return strResGrpId;
		}

		strResGrpId = m_lstRscGrpId.front();

		m_lstRscGrpId.pop_front();

		return strResGrpId;

}

int CStmResReqApi::DecodeMessage(std::vector<char>& a_vecPayload)
{
        int nRet = 0;
        std::string  strPayload;

        strPayload = &a_vecPayload[0];

        nRet = DecodeMessage(strPayload);

        return nRet;
}

int CStmResReqApi::DecodeMessage(std::string &a_strPayload)
{
		unsigned int i = 0;

		try{
				rabbit::document cDoc;
				rabbit::object cBody;

				cDoc.parse(a_strPayload);

				cBody = cDoc["BODY"];
				m_nSessionId = cBody["session_id"].as_int();
				m_nCollectTime = cBody["collect_time"].as_uint64();

				rabbit::array rscGrpIdArray = cBody["rsc_grp_id_array"];

				m_lstRscGrpId.clear();

				for(i=0;i<rscGrpIdArray.size();i++){
						std::string strTmp;

						strTmp = rscGrpIdArray[i]["rsc_grp_id"].as_string();
						m_lstRscGrpId.push_back(strTmp);
				}


		} catch(rabbit::type_mismatch   e) {
				m_strErrString = e.what();
				return RESAPI_RESULT_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString = e.what();
				return RESAPI_RESULT_PARSING_ERROR;
		} catch(...) {
				m_strErrString = "Unknown Error";
				return RESAPI_RESULT_PARSING_ERROR;
		}

		return RESAPI_RESULT_OK;
}

int CStmResReqApi::EncodeMessage(std::vector<char>& a_vecPayload)
{
        int nRet = RESAPI_RESULT_OK;
        std::string strPayload;

        nRet = EncodeMessage(strPayload);
        if(nRet != RESAPI_RESULT_OK){
                return nRet;
        }

        a_vecPayload.clear();
        a_vecPayload.insert(a_vecPayload.end(), strPayload.begin(), strPayload.end());

        return RESAPI_RESULT_OK;
}

int CStmResReqApi::EncodeMessage(std::string &a_strPayload)
{
		std::string strData;
		rabbit::object *cObjectValue = NULL;
		std::list<std::string>::iterator iter;

		try{
				rabbit::object cRoot;
				rabbit::object cBody = cRoot["BODY"];

				cBody["session_id"] = m_nSessionId;
				cBody["collect_time"] = m_nCollectTime;

				rabbit::array rscGrpIdArray = cBody["rsc_grp_id_array"];

				for(iter = m_lstRscGrpId.begin(); iter != m_lstRscGrpId.end(); iter++){
						strData = *iter;

						cObjectValue = GetRabbitObject();

						(*cObjectValue)["rsc_grp_id"] = strData;

						rscGrpIdArray.push_back(*cObjectValue);

						cObjectValue = NULL;
				}

				a_strPayload = cRoot.str();

		} catch(rabbit::type_mismatch   e) {
				m_strErrString = e.what();
				return RESAPI_RESULT_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString = e.what();
				return RESAPI_RESULT_PARSING_ERROR;
		} catch(...) {
				m_strErrString = "Unknown Error";
				return RESAPI_RESULT_PARSING_ERROR;
		}

		return RESAPI_RESULT_OK;
}

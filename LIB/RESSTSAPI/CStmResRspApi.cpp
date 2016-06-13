#include "CResGroup.hpp"
#include "CStmResApi.hpp"
#include "CStmResRspApi.hpp"

CStmResRspApi::CStmResRspApi()
{
		m_nSessionId = 0;
		m_nResultCode = 0;
		//m_lstRabbitObject = NULL;
		//m_lstRabbitArray = NULL;
}

CStmResRspApi::~CStmResRspApi()
{
		//std::list<rabbit::object*>::iterator  objectIter;
		//std::list<rabbit::array*>::iterator  arrayIter;

		EraseResGroupAll();

#if 0
		if(m_lstRabbitObject != NULL){
				for(objectIter = m_lstRabbitObject->begin();objectIter != m_lstRabbitObject->end();){
						delete *objectIter;
						m_lstRabbitObject->erase(objectIter++);
				}

				delete m_lstRabbitObject;
		}

		if(m_lstRabbitArray != NULL){
				for(arrayIter = m_lstRabbitArray->begin();arrayIter != m_lstRabbitArray->end();){
						delete *arrayIter;
						m_lstRabbitArray->erase(arrayIter++);
				}

				delete m_lstRabbitArray;
		}
#endif
}

CResGroup& CStmResRspApi::operator[] (std::string a_strRscId)
{
		std::map<std::string, CResGroup*>::iterator iter;

		iter = m_mapResGroupMap.find(a_strRscId);
		if(iter != m_mapResGroupMap.end()){
				return *iter->second;
		}
		else {
				CResGroup *rscGroup = NULL;

				rscGroup= new CResGroup(a_strRscId);

				m_mapResGroupMap.insert(
								std::map<std::string,CResGroup*>::value_type(a_strRscId, rscGroup));

				return *rscGroup;
		}
}

CResGroup* CStmResRspApi::GetFirstResGroup()
{
		std::map<std::string, CResGroup*>::iterator iter;
		CResGroup *cResGroup = NULL;

		if(m_mapResGroupMap.size() == 0){
				return NULL;
		}

		iter = m_mapResGroupMap.begin();

		cResGroup = iter->second;

		m_mapResGroupMap.erase(iter);


		return cResGroup;
}

CResGroup* CStmResRspApi::GetFirstResGroupP()
{
		std::map<std::string, CResGroup*>::iterator iter;

		if(m_mapResGroupMap.size() == 0){
				return NULL;
		}

		iter = m_mapResGroupMap.begin();

		return iter->second;
}

CResGroup* CStmResRspApi::GetResGroup(unsigned int a_nIndex)
{
		int i = 0;
		std::map<std::string, CResGroup*>::iterator iter;

		if(a_nIndex > m_mapResGroupMap.size()){
				return NULL;
		}

		for(i = 0, iter = m_mapResGroupMap.begin();(i != 0) && (iter != m_mapResGroupMap.end());i++, iter++){ 
		}

		return iter->second;
}

int CStmResRspApi::EraseResGroupAll()
{
		std::map<std::string, CResGroup*>::iterator iter;

		for(iter = m_mapResGroupMap.begin();iter != m_mapResGroupMap.end();){ 
				delete iter->second;
				m_mapResGroupMap.erase(iter++);
		}

		return CStmResApi::RESAPI_RESULT_OK;

}

int CStmResRspApi::EraseResGroup(std::string a_strRscId)
{
		std::map<std::string, CResGroup*>::iterator iter;

		iter = m_mapResGroupMap.find(a_strRscId);
		if(iter != m_mapResGroupMap.end()){
				return CStmResApi::RESAPI_RESULT_OK;
		}

		delete iter->second;
		m_mapResGroupMap.erase(iter);

		return CStmResApi::RESAPI_RESULT_OK;
}

int CStmResRspApi::EncodeMessage(std::string &a_strData)
{
		std::map<std::string, CResGroup*>::iterator iter;
		CResGroup *cResGroup = NULL;
		int i = 0;
		int nRet = 0;

		try {
				rabbit::object cRoot;
				rabbit::object cBody = cRoot["BODY"];
				rabbit::object *cRecord = NULL;

				cBody["session_id"] = m_nSessionId;
				cBody["result_code"] = m_nResultCode;

				rabbit::array cTableArray = cBody["res_group_array"];

				cRecord = new rabbit::object[m_mapResGroupMap.size()];

				for(i=0, iter = m_mapResGroupMap.begin();iter != m_mapResGroupMap.end(); iter++, i++){
						cResGroup = iter->second;
						nRet = cResGroup->GetEncodeMessage(cRecord[i]);
						if(nRet != CStmResApi::RESAPI_RESULT_OK){
								return nRet;
						}

						cTableArray.push_back(cRecord[i]);
				}

				a_strData = cRoot.str();

				delete[] cRecord;
		} catch(rabbit::type_mismatch   e) {
				m_strErrString.append(e.what());
				return CStmResApi::RESAPI_RESULT_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString.append(e.what());
				return CStmResApi::RESAPI_RESULT_PARSING_ERROR;
		} catch(...) {
				m_strErrString.append("Unknown Error");
				return CStmResApi::RESAPI_RESULT_PARSING_ERROR;
		}

		return CStmResApi::RESAPI_RESULT_OK;
}

int CStmResRspApi::DecodeMessage(std::string &a_strData)
{
		int nRet = 0;
		unsigned int i = 0;
		std::string strTableName;
		std::string strTableType;
		std::string strRscId;

		try {
				rabbit::document doc;
				rabbit::object cBody;
				rabbit::object cResGroupObject;
				rabbit::array cArray;
				doc.parse(a_strData);

				cBody = doc["BODY"];

				m_nSessionId = cBody["session_id"].as_int();
				m_nResultCode = cBody["result_code"].as_int();

				cArray = cBody["res_group_array"];

				for(i=0;i<cArray.size();i++){
						CResGroup *cResGroup = NULL;

						cResGroupObject = cArray[i];

						strRscId = cResGroupObject["rsc_id"].as_string();

						cResGroup = new CResGroup(strRscId);
						
						nRet = cResGroup->GetDecodeMessage(cResGroupObject);
						if(nRet != CStmResApi::RESAPI_RESULT_OK){
								delete cResGroup;
								return nRet;
						}

						m_mapResGroupMap.insert(std::map<std::string,CResGroup*>::value_type(strRscId, cResGroup));

				}
		} catch(rabbit::type_mismatch   e) {
				m_strErrString.append(e.what());
				return CStmResApi::RESAPI_RESULT_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString.append(e.what());
				return CStmResApi::RESAPI_RESULT_PARSING_ERROR;
		} catch(...) {
				m_strErrString.append("Unknown Error");
				return CStmResApi::RESAPI_RESULT_PARSING_ERROR;
		}

		return CStmResApi::RESAPI_RESULT_OK;
}


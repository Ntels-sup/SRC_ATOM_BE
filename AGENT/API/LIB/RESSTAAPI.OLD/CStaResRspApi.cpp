#include "CStaResApi.hpp"
#include "CStaResRspApi.hpp"

CStaResRspApi::CStaResRspApi()
{
		m_lstRabbitObject = NULL;
		m_lstRabbitArray = NULL;
}

CStaResRspApi::~CStaResRspApi()
{
		std::list<rabbit::object*>::iterator  objectIter;
		std::list<rabbit::array*>::iterator  arrayIter;

		EraseResTableAll();

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
}

CResTableData& CStaResRspApi::operator[] (std::string a_strRscGrpId)
{
		std::map<std::string, CResTableData*>::iterator iter;

		iter = m_mapResTableMap.find(a_strRscGrpId);
		if(iter != m_mapResTableMap.end()){
				return *iter->second;
		}
		else {
				CResTableData *resTableData = NULL;

				resTableData = new CResTableData(a_strRscGrpId);

				m_mapResTableMap.insert(std::map<std::string,CResTableData*>::value_type(a_strRscGrpId, resTableData));

				return *resTableData;
		}
}

CResTableData* CStaResRspApi::GetFirstResTableData()
{
		std::map<std::string, CResTableData*>::iterator iter;
		CResTableData *cResTableData = NULL;

		if(m_mapResTableMap.size() == 0){
				return NULL;
		}

		iter = m_mapResTableMap.begin();

		cResTableData = iter->second;

		m_mapResTableMap.erase(iter);


		return cResTableData;
}

CResTableData* CStaResRspApi::GetFirstResTableDataP()
{
		std::map<std::string, CResTableData*>::iterator iter;

		if(m_mapResTableMap.size() == 0){
				return NULL;
		}

		iter = m_mapResTableMap.begin();

		return iter->second;
}

CResTableData* CStaResRspApi::GetResTableData(unsigned int a_nIndex)
{
		int i = 0;
		std::map<std::string, CResTableData*>::iterator iter;

		if(a_nIndex > m_mapResTableMap.size()){
				return NULL;
		}

		for(i = 0, iter = m_mapResTableMap.begin();(i != 0) && (iter != m_mapResTableMap.end());i++, iter++){ 
		}

		return iter->second;
}

int CStaResRspApi::EraseResTableAll()
{
		std::map<std::string, CResTableData*>::iterator iter;

		for(iter = m_mapResTableMap.begin();iter != m_mapResTableMap.end();){ 
				delete iter->second;
				m_mapResTableMap.erase(iter++);
		}

		return CStaResApi::RESAPI_RESULT_OK;

}

int CStaResRspApi::EraseResTable(std::string a_strResTableName)
{
		std::map<std::string, CResTableData*>::iterator iter;

		iter = m_mapResTableMap.find(a_strResTableName);
		if(iter != m_mapResTableMap.end()){
				return CStaResApi::RESAPI_RESULT_OK;
		}

		delete iter->second;
		m_mapResTableMap.erase(iter);

		return CStaResApi::RESAPI_RESULT_OK;
}

rabbit::object *CStaResRspApi::GetRabbitObject()
{
        rabbit::object *cObject = NULL;

        if(m_lstRabbitObject == NULL){
                m_lstRabbitObject = new std::list<rabbit::object*>;
        }


        cObject = new rabbit::object;

        m_lstRabbitObject->push_back(cObject);

        return cObject;
}

rabbit::array *CStaResRspApi::GetRabbitArray()
{
        rabbit::array *cArray = NULL;

        if(m_lstRabbitArray == NULL){
                m_lstRabbitArray = new std::list<rabbit::array*>;
        }

        cArray = new rabbit::array;

        m_lstRabbitArray->push_back(cArray);

        return cArray;
}

int CStaResRspApi::EncodeResTableData(CResTableData *a_cResTableData, rabbit::object *a_cRecord)
{
        std::string strData;
        std::list<std::string>::iterator priIter;
        std::list<int>::iterator valueIter;

        try{
                (*a_cRecord)["rsc_grp_id"] = a_cResTableData->GetRscGrpId();
                (*a_cRecord)["prc_date"] = a_cResTableData->GetPrcDate();
                (*a_cRecord)["node_no"] = a_cResTableData->GetNodeNo();
                (*a_cRecord)["column_order_ccd"] = a_cResTableData->GetColumnOrderCCD();
                (*a_cRecord)["stat_data"] = a_cResTableData->GetStatData();
        } catch(rabbit::type_mismatch   e) {
                m_strErrString.append(e.what());
                return CStaResApi::RESAPI_RESULT_PARSING_ERROR;
        } catch(rabbit::parse_error e) {
                m_strErrString.append(e.what());
                return CStaResApi::RESAPI_RESULT_PARSING_ERROR;
        } catch(...) {
                m_strErrString.append("Unknown Error");
                return CStaResApi::RESAPI_RESULT_PARSING_ERROR;
        }

		return CStaResApi::RESAPI_RESULT_OK;
}

int CStaResRspApi::EncodeMessage(std::string &a_strData)
{
        std::map<std::string, CResTableData*>::iterator iter;
        int i = 0;
        int nRet = 0;

        try {
                rabbit::object cRoot;
                rabbit::object cBody = cRoot["BODY"];
                rabbit::object *cRecord = NULL;

                rabbit::array cTableArray = cBody["res_table_array"];

                cRecord = new rabbit::object[m_mapResTableMap.size()];

                for(i=0, iter = m_mapResTableMap.begin();iter != m_mapResTableMap.end(); iter++, i++){
                        nRet = EncodeResTableData(iter->second, &cRecord[i]);
                        if(nRet != CStaResApi::RESAPI_RESULT_OK){
                                return nRet;
                        }

                        cTableArray.push_back(cRecord[i]);
                }

                a_strData = cRoot.str();

                delete[] cRecord;
        } catch(rabbit::type_mismatch   e) {
                m_strErrString.append(e.what());
                return CStaResApi::RESAPI_RESULT_PARSING_ERROR;
        } catch(rabbit::parse_error e) {
                m_strErrString.append(e.what());
                return CStaResApi::RESAPI_RESULT_PARSING_ERROR;
        } catch(...) {
                m_strErrString.append("Unknown Error");
                return CStaResApi::RESAPI_RESULT_PARSING_ERROR;
        }

        return CStaResApi::RESAPI_RESULT_OK;

}

int CStaResRspApi::DecodeTableData(rabbit::object *a_cRecord, CResTableData **a_cResTableData)
{
		std::string strDecodeData;
		std::string strResGrpId;
		time_t prcDate;
		int nNodeNo;
		std::string strColumnOrderCCD;
		double m_dStatData;

		CResTableData *cResTableData = NULL;

		try {
				strResGrpId = (*a_cRecord)["rsc_grp_id"].as_string();
				prcDate = (*a_cRecord)["prc_date"].as_uint64();
				nNodeNo = (*a_cRecord)["node_no"].as_uint();
				strColumnOrderCCD = (*a_cRecord)["column_order_ccd"].as_string();
				m_dStatData = (*a_cRecord)["stat_data"].as_double();

				cResTableData = new CResTableData(strResGrpId);

				cResTableData->Init(prcDate, nNodeNo, strColumnOrderCCD, m_dStatData);

		} catch(rabbit::type_mismatch   e) {
				m_strErrString.append(e.what());
				return CStaResApi::RESAPI_RESULT_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString.append(e.what());
				return CStaResApi::RESAPI_RESULT_PARSING_ERROR;
		} catch(...) {
				m_strErrString.append("Unknown Error");
				return CStaResApi::RESAPI_RESULT_PARSING_ERROR;
		}

		(*a_cResTableData) = cResTableData;

		return CStaResApi::RESAPI_RESULT_OK;
}

int CStaResRspApi::DecodeMessage(std::string &a_strData)
{
        int nRet = 0;
        unsigned int i = 0;
        std::string strTableName;
        std::string strTableType;
		std::string strRscGrpId;

        try {
                rabbit::document doc;
                rabbit::object cBody;
                rabbit::object cResTableObject;
                rabbit::array cArray;
                doc.parse(a_strData);

                cBody = doc["BODY"];

                cArray = cBody["res_table_array"];

                for(i=0;i<cArray.size();i++){
                        CResTableData *cResTableData = NULL;

                        cResTableObject = cArray[i];

                        nRet = DecodeTableData(&cResTableObject, &cResTableData);
                        if(nRet != CStaResApi::RESAPI_RESULT_OK){
                                return nRet;
                        }

					strRscGrpId = cResTableData->GetRscGrpId();
				m_mapResTableMap.insert(std::map<std::string,CResTableData*>::value_type(strRscGrpId, cResTableData));
#if 0
                        m_mapResTableMap.insert(
                                        std::map<std::string,CResTableData*>::value_type(cResTableData->GetRscGrpId(), cResTableData)
                                        );
#endif

                }
		} catch(rabbit::type_mismatch   e) {
				m_strErrString.append(e.what());
				return CStaResApi::RESAPI_RESULT_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				m_strErrString.append(e.what());
				return CStaResApi::RESAPI_RESULT_PARSING_ERROR;
		} catch(...) {
				m_strErrString.append("Unknown Error");
				return CStaResApi::RESAPI_RESULT_PARSING_ERROR;
		}

		return CStaResApi::RESAPI_RESULT_OK;
}


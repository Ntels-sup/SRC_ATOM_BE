#include "CResGroup.hpp"

CResGroup::CResGroup(std::string &a_strRscId)
{
		m_strRscId =  a_strRscId;
}

CResGroup::~CResGroup()
{
		EraseResTableDataAll();
}

int CResGroup::EraseResTableDataAll()
{
		std::map<std::string, CResTableData*>::iterator iter;

		for(iter = m_mapResTableDataMap.begin();iter != m_mapResTableDataMap.end();){
				delete iter->second;
				m_mapResTableDataMap.erase(iter++);
		}

		return CStmResApi::RESAPI_RESULT_OK;
}
 
CResTableData& CResGroup::operator[] (std::string a_strColumnOrder)
{
		std::map<std::string, CResTableData*>::iterator iter;

		iter = m_mapResTableDataMap.find(a_strColumnOrder);
		if(iter != m_mapResTableDataMap.end()){
				return *iter->second;
		}
		else {
				CResTableData *resTableData = NULL;

				resTableData = new CResTableData(a_strColumnOrder);

				m_mapResTableDataMap.insert(
								std::map<std::string,CResTableData*>::value_type(a_strColumnOrder, resTableData));

				return *resTableData;
		}

}

CResTableData* CResGroup::GetResTableData(unsigned int a_nIndex)
{
        int i = 0;
        std::map<std::string, CResTableData*>::iterator iter;

        if(a_nIndex > m_mapResTableDataMap.size()){
                return NULL;
        }

        for(i = 0, iter = m_mapResTableDataMap.begin();(i != 0) && (iter != m_mapResTableDataMap.end());i++, iter++){
        }

        return iter->second;
}

CResTableData* CResGroup::GetFirstResTableData()
{
        std::map<std::string, CResTableData*>::iterator iter;
        CResTableData *cResTableData = NULL;

        if(m_mapResTableDataMap.size() == 0){
                return NULL;
        }

        iter = m_mapResTableDataMap.begin();

        cResTableData = iter->second;

        m_mapResTableDataMap.erase(iter);


        return cResTableData;
}

CResTableData* CResGroup::GetFirstResTableDataP()
{
        std::map<std::string, CResTableData*>::iterator iter;

        if(m_mapResTableDataMap.size() == 0){
                return NULL;
        }

        iter = m_mapResTableDataMap.begin();

        return iter->second;
}

int CResGroup::GetEncodeMessage(rabbit::object &a_cRecord)
{
		int nRet = 0;
		int i = 0;
		std::string strData;
		std::map<std::string, CResTableData*>::iterator iter;
		CResTableData *cResTableData = NULL;
		rabbit::object *cRecord = NULL;
		rabbit::array *cTableArray = NULL;

		try{
				a_cRecord["rsc_id"] = m_strRscId;

				cTableArray = GetRabbitArray();

				*(cTableArray) = a_cRecord["res_table_array"];

				for(i=0, iter = m_mapResTableDataMap.begin();iter != m_mapResTableDataMap.end(); iter++, i++){
						cResTableData = iter->second;
						cRecord = GetRabbitObject();
						nRet = cResTableData->GetEncodeMessage(*cRecord);
						if(nRet != CStmResApi::RESAPI_RESULT_OK){
								return nRet;
						}

						cTableArray->push_back(*cRecord);
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

int CResGroup::GetDecodeMessage(rabbit::object &a_cRecord)
{
		int nRet = 0;
		unsigned int i = 0;
		std::string strData;
		std::map<std::string, CResTableData*>::iterator iter;
		std::string strColumnOrder;
		CResTableData *cResTableData = NULL;
		rabbit::object cRecord;
		rabbit::array cArray;

		try{
				//cArray = GetRabbitArray();

				cArray = a_cRecord["res_table_array"];


				for(i=0;i<cArray.size();i++){
						cRecord = cArray[i];

						strColumnOrder = cRecord["column_order_ccd"].as_string();
						cResTableData = new CResTableData(strColumnOrder);

						nRet = cResTableData->GetDecodeMessage(cRecord);
						if(nRet != CStmResApi::RESAPI_RESULT_OK){
								delete cResTableData;
								return nRet;
						}

						m_mapResTableDataMap.insert(
										std::map<std::string,CResTableData*>::value_type(strColumnOrder, cResTableData));
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


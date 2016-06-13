#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include <stdio.h>
#include <string.h>
#include "STAAPI.h"
#include "CStaAnsApi.hpp"

/* message format 
CollectTime[8]
TableType[4] 1 :  HIST, 2 : STS
TableName[tlv]
priKeyCount [4]
priKey[tlv]
..
valueCount[4]
value[tlv]
...
*/

CStaAnsApi::CStaAnsApi()
{
		m_lstRabbitObject = NULL;
		m_lstRabbitArray = NULL;
}

CStaAnsApi::~CStaAnsApi()
{
		std::list<rabbit::object*>::iterator  objectIter;
		std::list<rabbit::array*>::iterator  arrayIter;

		EraseTableAll();

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

CTableData& CStaAnsApi::operator[] (std::string a_strTableName)
{
		std::map<std::string, CTableData*>::iterator iter;

		iter = m_mapTableMap.find(a_strTableName);
		if(iter != m_mapTableMap.end()){
				return *iter->second;
		}
		else {
				CTableData *tableData = NULL;

				tableData = new CTableData(a_strTableName);

				m_mapTableMap.insert(std::map<std::string,CTableData*>::value_type(a_strTableName, tableData));

				return *tableData;
		}
}

CTableData* CStaAnsApi::GetFirstTableData()
{
		std::map<std::string, CTableData*>::iterator iter;
		CTableData *cTableData = NULL;

		if(m_mapTableMap.size() == 0){
				return NULL;
		}

		iter = m_mapTableMap.begin();

		cTableData = iter->second;

		m_mapTableMap.erase(iter);


		return cTableData;
}

CTableData* CStaAnsApi::GetFirstTableDataP()
{
		std::map<std::string, CTableData*>::iterator iter;

		if(m_mapTableMap.size() == 0){
				return NULL;
		}

		iter = m_mapTableMap.begin();

		return iter->second;
}

CTableData* CStaAnsApi::GetTableData(unsigned int a_nIndex)
{
		int i = 0;
		std::map<std::string, CTableData*>::iterator iter;

		if(a_nIndex > m_mapTableMap.size()){
				return NULL;
		}

		for(i = 0, iter = m_mapTableMap.begin();(i != 0) && (iter != m_mapTableMap.end());i++, iter++){ 
		}

		return iter->second;


}

int CStaAnsApi::EraseTableAll()
{
		std::map<std::string, CTableData*>::iterator iter;

		for(iter = m_mapTableMap.begin();iter != m_mapTableMap.end();){ 
				delete iter->second;
				m_mapTableMap.erase(iter++);
		}

		return STAAPI_OK;

}

int CStaAnsApi::EraseTable(std::string a_strTableName)
{
		std::map<std::string, CTableData*>::iterator iter;

		iter = m_mapTableMap.find(a_strTableName);
		if(iter != m_mapTableMap.end()){
				return STAAPI_OK;
		}

		delete iter->second;
		m_mapTableMap.erase(iter);

		return STAAPI_OK;
}

rabbit::object *CStaAnsApi::GetRabbitObject()
{
        rabbit::object *cObject = NULL;

        if(m_lstRabbitObject == NULL){
                m_lstRabbitObject = new std::list<rabbit::object*>;
        }


        cObject = new rabbit::object;

        m_lstRabbitObject->push_back(cObject);

        return cObject;
}

rabbit::array *CStaAnsApi::GetRabbitArray()
{
        rabbit::array *cArray = NULL;

        if(m_lstRabbitArray == NULL){
                m_lstRabbitArray = new std::list<rabbit::array*>;
        }

        cArray = new rabbit::array;

        m_lstRabbitArray->push_back(cArray);

        return cArray;
}

int CStaAnsApi::EncodeTableData(CTableData *a_cTableData, rabbit::object *a_cRecord)
{
		int nData = 0;
		int nTableType = 0;
		std::string strData;
		std::list<std::string> *lstPrimaryKey = NULL;
		std::list<int> *lstValue = NULL;
		std::list<std::string>::iterator priIter;
		std::list<int>::iterator valueIter;

		lstPrimaryKey = a_cTableData->GetPrimaryKeyP();
		lstValue = a_cTableData->GetValueP();

		try{
				rabbit::object *cObject= NULL;
				rabbit::array *cArray = NULL;

				(*a_cRecord)["TABLE_NAME"] = a_cTableData->GetTableName();
				(*a_cRecord)["COLLECT_TIME"] = (uint64_t)a_cTableData->GetCollectTime();

				nTableType = a_cTableData->GetTableType();
				if( nTableType == STAAPI_TABLE_TYPE_STS){
						(*a_cRecord)["TABLE_TYPE"] = "STS";
				}
				else if( nTableType == STAAPI_TABLE_TYPE_HIST){
						(*a_cRecord)["TABLE_TYPE"] = "HIST";
				}
				else {
						STAAPI_LOG(STAAPI_ERR,"Invalid TableType(type=%d)\n",nTableType);
						return STAAPI_INVALID_TABLE_TYPE;
				}

				if(lstValue == NULL){
						STAAPI_LOG(STAAPI_ERR,"Value not exist\n");
						return STAAPI_VALUE_NOT_EXIST;
				}

				(*a_cRecord)["TABLE_NAME"] = a_cTableData->GetTableName();

				cArray = GetRabbitArray();

				*cArray = (*a_cRecord)["PRIMARY_KEY_ARRAY"];

				if(lstPrimaryKey != NULL){
						/* set primary key */
						for(priIter = lstPrimaryKey->begin();priIter != lstPrimaryKey->end();priIter++){
								cObject = GetRabbitObject();
								strData = *priIter;

								(*cObject)["PRIMARY_KEY"] = strData;
								(*cArray).push_back(*cObject);
						}
				}

				cArray = GetRabbitArray();
				*cArray = (*a_cRecord)["VALUE_ARRAY"];
				for(valueIter = lstValue->begin();valueIter != lstValue->end();valueIter++){
						cObject = GetRabbitObject();
						nData = *valueIter;

						(*cObject)["VALUE"] = nData;
						(*cArray).push_back(*cObject);
				}
		} catch(rabbit::type_mismatch   e) {
				STAAPI_LOG(STAAPI_ERR,"JSON Type Mismatch(%s)\n",e.what());
				return STAAPI_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				STAAPI_LOG(STAAPI_ERR,"JSON Parsing Error(%s)\n",e.what());
				return STAAPI_PARSING_ERROR;
		} catch(...) {
				m_strParsingError.append("Unknown Error");
				return STAAPI_PARSING_ERROR;
		}

		return STAAPI_OK;
}

int CStaAnsApi::EncodeMessage(std::string &a_strData)
{
		std::map<std::string, CTableData*>::iterator iter;
		int i = 0;
		int nRet = STAAPI_OK;

		try {
				rabbit::object cRoot;
				rabbit::object cBody = cRoot["BODY"];
				rabbit::object *cRecord = NULL;

				rabbit::array cTableArray = cBody["TABLE_ARRAY"];

				cRecord = new rabbit::object[m_mapTableMap.size()];

				for(i=0, iter = m_mapTableMap.begin();iter != m_mapTableMap.end(); iter++, i++){
						nRet = EncodeTableData(iter->second, &cRecord[i]);
						if(nRet != STAAPI_OK){
								return nRet;
						}

						cTableArray.push_back(cRecord[i]);
				}

				a_strData = cRoot.str();

				delete[] cRecord;
		} catch(rabbit::type_mismatch   e) {
				STAAPI_LOG(STAAPI_ERR,"JSON Type Mismatch(%s)\n",e.what());
				return STAAPI_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				STAAPI_LOG(STAAPI_ERR,"JSON Parsing Error(%s)\n",e.what());
				return STAAPI_PARSING_ERROR;
		} catch(...) {
				m_strParsingError.append("Unknown Error");
				return STAAPI_PARSING_ERROR;
		}

		return STAAPI_OK;

}

int CStaAnsApi::EncodeMessage(char *a_chData, unsigned int a_nMaxLen, unsigned int *a_nDataLen)
{
		int nRet = STAAPI_OK;
		std::string strBuffer;

		nRet = EncodeMessage(strBuffer);
		if(nRet != STAAPI_OK){
				return nRet;
		}

		if(strBuffer.size() > a_nMaxLen) {
				STAAPI_LOG(STAAPI_ERR,"Buffer is small(dataSize=%lu, bufferSize=%d)\n",strBuffer.size(), a_nMaxLen);
				return STAAPI_BUFFER_TOO_SMALL;
		}
		strncpy(a_chData, strBuffer.c_str(), (size_t)strBuffer.size());
		(*a_nDataLen) = strBuffer.size();

		return STAAPI_OK;
}

int CStaAnsApi::DecodeTableData(rabbit::object *a_cRecord, CTableData **a_cTableData)
{
		int nRet = 0;
		unsigned int i = 0;
		std::string strDecodeData;
		std::string strTableName;
		std::string strTableType;
		time_t collectTime = 0;
		int nTableType = 0;
		CTableData *cTableData = NULL;

		try {
				strTableName = (*a_cRecord)["TABLE_NAME"].as_string();
				if(strTableName.size() == 0){
						STAAPI_LOG(STAAPI_ERR,"Table name not exist\n");
						return STAAPI_TABLE_NAME_NOT_EXIST;
				}

				cTableData = new CTableData(strTableName);

				collectTime = (*a_cRecord)["COLLECT_TIME"].as_int64();

				strTableType = (*a_cRecord)["TABLE_TYPE"].as_string();
				if(strncasecmp(strTableType.c_str(),"STS", 3) == 0){
						nTableType = STAAPI_TABLE_TYPE_STS;
				}
				else if(strncasecmp(strTableType.c_str(),"HIST", 3) == 0){
						nTableType = STAAPI_TABLE_TYPE_HIST;
				}
				else {
						STAAPI_LOG(STAAPI_ERR,"Invalid Table Type(%s)\n",strTableType.c_str());
						return STAAPI_INVALID_TABLE_TYPE;
				}

				nRet = cTableData->SetTableInfo(collectTime, nTableType);
				if(nRet != STAAPI_OK){
						delete cTableData;
						STAAPI_LOG(STAAPI_ERR,"Table Setting failed(ret=%d, ERR:%s)\n",nRet, cTableData->GetError());
						return STAAPI_TABLE_SETTING_FAEILD;
				}

				rabbit::array cPrimaryKeyArray = (*a_cRecord)["PRIMARY_KEY_ARRAY"];

				if(cPrimaryKeyArray.size() != 0){

						for(i=0;i<cPrimaryKeyArray.size();i++){
								std::string strPrimaryKey;

								strPrimaryKey = cPrimaryKeyArray[i]["PRIMARY_KEY"].as_string();

								nRet = cTableData->InsertPrimaryKey(strPrimaryKey);
								if(nRet != STAAPI_OK){
										delete cTableData;
										STAAPI_LOG(STAAPI_ERR,"Primary Key insert failed(ret=%d, ERR:%s)\n",
														nRet, cTableData->GetError());
										return STAAPI_PRIMARY_KEY_INSERT_FAILED;
								}
						}
				}

				rabbit::array cValueArray = (*a_cRecord)["VALUE_ARRAY"];

				if(cValueArray.size() == 0){
						delete cTableData;
						STAAPI_LOG(STAAPI_ERR,"VALUE NOT EXIST(TableName=%s\n", strTableName.c_str());
						return STAAPI_VALUE_NOT_EXIST;
				}

				for(i = 0; i<cValueArray.size(); i++){
						cTableData->InsertValue(cValueArray[i]["VALUE"].as_int());
						if(nRet != STAAPI_OK){
								delete cTableData;
								STAAPI_LOG(STAAPI_ERR,"Value insert failed(ret=%d, ERR:%s)\n",
												nRet, cTableData->GetError());
								return STAAPI_VALUE_INSERT_FAILED;
						}
				}
		} catch(rabbit::type_mismatch   e) {
				STAAPI_LOG(STAAPI_ERR,"JSON Type Mismatch(%s)\n",e.what());
				return STAAPI_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				STAAPI_LOG(STAAPI_ERR,"JSON Parsing Error(%s)\n",e.what());
				return STAAPI_PARSING_ERROR;
		} catch(...) {
				m_strParsingError.append("Unknown Error");
				return STAAPI_PARSING_ERROR;
		}

		(*a_cTableData) = cTableData;

		return STAAPI_OK;
}

int CStaAnsApi::DecodeMessage(std::string &a_strData)
{
		int nRet = STAAPI_OK;
		unsigned int i = 0;
		std::string strTableName;
		std::string strTableType;

		try {
				rabbit::document doc;
				rabbit::object cBody;
				rabbit::object cTableObject;
				rabbit::array cArray;
				doc.parse(a_strData);

				cBody = doc["BODY"];

				cArray = cBody["TABLE_ARRAY"];

				for(i=0;i<cArray.size();i++){
						CTableData *cTableData = NULL;

						cTableObject = cArray[i];

						nRet = DecodeTableData(&cTableObject, &cTableData);
						if(nRet != STAAPI_OK){
								return nRet;
						}

						m_mapTableMap.insert(
										std::map<std::string,CTableData*>::value_type(cTableData->GetTableName(), cTableData)
										);

				}
		} catch(rabbit::type_mismatch   e) {
				STAAPI_LOG(STAAPI_ERR,"JSON Type Mismatch(%s)\n",e.what());
				return STAAPI_PARSING_ERROR;
		} catch(rabbit::parse_error e) {
				STAAPI_LOG(STAAPI_ERR,"JSON Parsing Error(%s)\n",e.what());
				return STAAPI_PARSING_ERROR;
		} catch(...) {
				m_strParsingError.append("Unknown Error");
				return STAAPI_PARSING_ERROR;
		}

		return STAAPI_OK;
}

int CStaAnsApi::DecodeMessage(char *a_chData, unsigned int a_nDataLen)
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


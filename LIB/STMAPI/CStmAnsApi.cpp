#include <stdio.h>
#include <string.h>
//#include "STMAPI.h"
#include "CStmAnsApi.hpp"

CStmAnsApi::CStmAnsApi()
{
		m_nSessionId = 0;
		m_nTableType = 0;
		m_nResultCode = RESULT_CODE_SUCCESS;
		m_lstCollectValue = NULL;
		m_lstRabbitObject = NULL;
		m_lstRabbitArray = NULL;
}

CStmAnsApi::~CStmAnsApi()
{
		CCollectValue *cValue = NULL;
		list<CCollectValue*>::iterator  iter;
		list<rabbit::object*>::iterator  objectIter;
		list<rabbit::array*>::iterator  arrayIter;

		if(m_lstCollectValue != NULL){
				for(iter = m_lstCollectValue->begin();iter != m_lstCollectValue->end();){
						cValue = *iter;
						delete cValue;
						m_lstCollectValue->erase(iter++);
				}

				delete m_lstCollectValue;
		}

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

CStmApi::eResult CStmAnsApi::Init(int a_nSessionId, std::string &a_strPackageName, std::string &a_strTableName, int a_nTableType)
{
		m_nSessionId = a_nSessionId;

		if((a_nTableType != TABLE_TYPE_STS) &&
						//(a_nTableType != TABLE_TYPE_RES) &&
						(a_nTableType != TABLE_TYPE_HIST)){
				STMAPI_LOG(STMAPI_ERR,"Invalid type (type=%d)\n",a_nType);
				return RESULT_INVALID_TABLE_TYPE;
		}
		m_strPackageName = a_strPackageName;
		m_nTableType = a_nTableType;
		m_strTableName = a_strTableName;

		return RESULT_OK;
}

CStmApi::eResult  CStmAnsApi::SetCollectValue(list<CCollectValue*> *collectValue)
{
		m_lstCollectValue = collectValue;

		return RESULT_OK;
}

/* 
 * Resp
 * sessionId[4]
 * ResultCode[4]
 * Type[4]
 * PackageId[4]
 * tableName[tlv]
 * ValueCont[4]
 * {
 * primaryKeyCont[4]
 * primaryKey[tlv]
 * valueCount[4]
 * value[tlv]
 * }
 */
CStmApi::eResult CStmAnsApi::EncodeCollectValue(CCollectValue *a_cValue, rabbit::object *a_cRecord)
{
		int nData = 0;
		list<string> *lstPrimaryKey = NULL;
		list<int> *lstValue = NULL;
		string strData;
		list<string>::iterator priIter;
		list<int>::iterator valueIter;

		rabbit::object *cObjectValue = NULL;
		rabbit::array *cArrayValue = NULL;

		lstPrimaryKey  = a_cValue->GetPrimaryKeyListP();
		if(lstPrimaryKey != NULL){
				cArrayValue = GetRabbitArray();

				*cArrayValue = (*a_cRecord)["primary_key_array"];

				for(priIter = lstPrimaryKey->begin();priIter != lstPrimaryKey->end();priIter++){
						strData = *priIter;

						cObjectValue = GetRabbitObject();

						(*cObjectValue)["primary_key"] = strData;

						cArrayValue->push_back(*cObjectValue);

						cObjectValue = NULL;
				}
		}

		/* set data */
		lstValue = a_cValue->GetValueListP();

		cArrayValue = GetRabbitArray();
		*cArrayValue = (*a_cRecord)["value_array"];

		for(valueIter = lstValue->begin();valueIter != lstValue->end();valueIter++){
				nData = *valueIter;

				cObjectValue = GetRabbitObject();

				(*cObjectValue)["value"] = nData;
				
				cArrayValue->push_back(*cObjectValue);

				cObjectValue = NULL;
		}

		return RESULT_OK;
}

rabbit::object *CStmAnsApi::GetRabbitObject()
{
		rabbit::object *cObject = NULL;

		if(m_lstRabbitObject == NULL){
				m_lstRabbitObject = new list<rabbit::object*>;
		}


		cObject = new rabbit::object;

		m_lstRabbitObject->push_back(cObject);

		return cObject;
}

rabbit::array *CStmAnsApi::GetRabbitArray()
{
		rabbit::array *cArray = NULL;

		if(m_lstRabbitArray == NULL){
				m_lstRabbitArray = new list<rabbit::array*>;
		}

		cArray = new rabbit::array;

		m_lstRabbitArray->push_back(cArray);

		return cArray;
}

/* 
 * Resp
 * sessionId[4]
 * Type[4]
 * PackageId[4]
 * tableName[tlv]
 * RecordCount[4]
 * {
 * primaryKeyCont[4]
 * primaryKey[tlv]
 * valueCount[4]
 * value[tlv]
 * }
 */
CStmApi::eResult CStmAnsApi::EncodeMessage(char *a_chData, unsigned int a_nMaxLen, unsigned int *a_nDataLen)
{
		int i = 0;
		CStmApi::eResult nRet = RESULT_OK;
		list<CCollectValue*>::iterator  iter;

		try{
				rabbit::object cRoot;
				rabbit::object cBody = cRoot["BODY"];
				rabbit::object *cRecord = NULL;

				cBody["session_id"] = (unsigned int)m_nSessionId;
				cBody["result_code"] = (unsigned int)m_nResultCode;

				if(m_nResultCode == RESULT_CODE_SUCCESS){
						if( m_nTableType == TABLE_TYPE_STS){
								cBody["table_type"] = "STS";
						}
						else if( m_nTableType == TABLE_TYPE_HIST){
								cBody["table_type"] = "HIST";
						}
						else {
								return RESULT_INVALID_TABLE_TYPE;
						}

						cBody["package_name"] = m_strPackageName;

						cBody["table_name"] = m_strTableName;

						rabbit::array cRecordArray = cBody["record_array"];

						cRecord = new rabbit::object[m_lstCollectValue->size()];

						for(i = 0, iter = m_lstCollectValue->begin();iter != m_lstCollectValue->end();iter++, i++){
								nRet = EncodeCollectValue(*iter, &cRecord[i]);
								if(nRet != RESULT_OK){
										return nRet;
								}

								cRecordArray.push_back(cRecord[i]);
						}

				}

				if((unsigned int)cRoot.str().size() > a_nMaxLen){
						delete[] cRecord;

						return RESULT_BUFFER_TOO_SMALL;
				}

				strncpy(a_chData, cRoot.str().c_str(), cRoot.str().size());

				*a_nDataLen = cRoot.str().size();

				delete[] cRecord;
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

void CStmAnsApi::ClearCollectValue()
{
		CCollectValue *cCollectValue = NULL;
		list<CCollectValue*>::iterator iter;

		if(m_lstCollectValue != NULL){
				for(iter = m_lstCollectValue->begin();iter != m_lstCollectValue->end();){
						cCollectValue = *iter;

						delete cCollectValue;

						m_lstCollectValue->erase(iter++);	
				}

				delete m_lstCollectValue;
		}

		m_lstCollectValue = NULL;
}

CStmApi::eResult CStmAnsApi::MakeCollectValue(rabbit::array *a_cRecordArray)
{
		unsigned int i =0;
		CCollectValue *cCollectValue = NULL;
		list<string> *lstPrimaryKey = NULL;
		list<int> *lstValue = NULL;


		try{
				rabbit::array cPrimaryKeyArray;
				rabbit::array cValueArray;

				lstPrimaryKey = new list<string>;
				cPrimaryKeyArray = (*a_cRecordArray)["primary_key_array"];
				for(i=0;i<cPrimaryKeyArray.size();i++){
						string strTmp;
						strTmp = cPrimaryKeyArray[i]["primary_key"].as_string();
						lstPrimaryKey->push_back(strTmp);
				}

				cValueArray = (*a_cRecordArray)["value_array"];
				if(cValueArray.size() == 0){
						return RESULT_VALUE_NOT_EXIST;
				}

				lstValue = new list<int>;
				for(i=0;i<cValueArray.size();i++){
						lstValue->push_back(cValueArray[i]["value"].as_int());
				}

				cCollectValue = new CCollectValue();

				cCollectValue->SetPrimaryKey(lstPrimaryKey);
				cCollectValue->SetValue(lstValue);

				if(m_lstCollectValue == NULL){
						m_lstCollectValue = new list<CCollectValue*>;
				}

				m_lstCollectValue->push_back(cCollectValue);
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

CStmApi::eResult CStmAnsApi::DecodeMessage(char *a_chData, int a_nDataLen)
{
		CStmApi::eResult nRet = RESULT_OK;
		unsigned int i = 0;
		std::string strDecodeData;
		std::string strTableType;

		strDecodeData.append(a_chData, a_nDataLen);

		try{
				rabbit::document doc;
				rabbit::object cBody;

				doc.parse(strDecodeData);

				cBody = doc["BODY"];

				m_nSessionId = cBody["session_id"].as_int();
				m_nResultCode = cBody["result_code"].as_int();

				if(m_nResultCode != RESULT_CODE_SUCCESS){
						return RESULT_OK;
				}

				if(m_nResultCode == RESULT_CODE_SUCCESS){
						strTableType = cBody["table_type"].as_string();
						m_strPackageName = cBody["package_name"].as_string();

						if(strncasecmp(strTableType.c_str(),"STS", 3) == 0){
								m_nTableType = TABLE_TYPE_STS;
						}
						else if(strncasecmp(strTableType.c_str(),"HIST", 3) == 0){
								m_nTableType = TABLE_TYPE_HIST;
						}
						else {
								return RESULT_INVALID_TABLE_TYPE;
						}

						m_strTableName = cBody["table_name"].as_string();

						rabbit::array cRecordArray = cBody["record_array"];

						ClearCollectValue();

						m_lstCollectValue = new list<CCollectValue*>;

						for(i=0;i<cRecordArray.size();i++){
								rabbit::array cArrayValue;
								cArrayValue = cRecordArray[i];

								nRet = MakeCollectValue(&cArrayValue);
								if(nRet != RESULT_OK){
										return nRet;
								}
						}
				}
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

CStmApi::eResult CStmAnsApi::EncodeErrorMessage(int a_nSessionId, int a_nErrorCode, char *a_chData, 
				unsigned int a_nMaxLen, unsigned int *a_nDataLen)
{
		list<CCollectValue*>::iterator  iter;

		try{
				rabbit::object cRoot;
				rabbit::object cBody = cRoot["BODY"];
				rabbit::object *cRecord = NULL;

				cBody["session_id"] = (unsigned int)a_nSessionId;
				cBody["result_code"] = (unsigned int)a_nErrorCode;

				if((unsigned int)cRoot.str().size() > a_nMaxLen){
						delete[] cRecord;

						return RESULT_BUFFER_TOO_SMALL;
				}

				strncpy(a_chData, cRoot.str().c_str(), cRoot.str().size());

				*a_nDataLen = cRoot.str().size();

				delete[] cRecord;
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

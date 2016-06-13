#include <stdio.h>
#include <string.h>
#include <time.h>
#include "STAAPI.h"
#include "CTableData.hpp"

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

CTableData::CTableData(std::string a_strTableName)
{
		m_strTableName.append(a_strTableName);
        m_nTableType = STAAPI_TABLE_TYPE_STS;
        m_nCollectTime = time(NULL);
		m_lstPrimaryKey = NULL;
		m_lstValue = NULL;
}

CTableData::~CTableData()
{
		if(m_lstPrimaryKey != NULL){
				delete m_lstPrimaryKey;
		}
		if(m_lstValue != NULL){
				delete m_lstValue;
		}
}

int CTableData::SetTableInfo(time_t a_nCollectTime, int a_nTableType)
{
		if((a_nTableType != STAAPI_TABLE_TYPE_HIST) &&
						(a_nTableType != STAAPI_TABLE_TYPE_STS)){
				STAAPI_LOG(STAAPI_ERR,"Invalid Table Type(%d)\n",a_nTableType);
				return STAAPI_INVALID_TABLE_TYPE;
		}

		m_nTableType = a_nTableType;
		m_nCollectTime = a_nCollectTime;

		return STAAPI_OK;
}

int CTableData::InsertPrimaryKey(std::string a_strPrimaryKey)
{
		if(m_lstPrimaryKey == NULL){
				m_lstPrimaryKey = new std::list<std::string>;
		}

		m_lstPrimaryKey->push_back(a_strPrimaryKey);

		return STAAPI_OK;
}

int CTableData::InsertValue(int a_nValue)
{
		if(m_lstValue == NULL){
				m_lstValue = new std::list<int>;
		}

		m_lstValue->push_back(a_nValue);

		return STAAPI_OK;
}

std::list<std::string> *CTableData::GetPrimaryKey()
{
		std::list<std::string> *tmp = NULL;
		tmp = m_lstPrimaryKey;
		m_lstPrimaryKey = NULL;

		return tmp;
}

std::list<int> *CTableData::GetValue()
{
		std::list<int> *tmp = NULL;
		tmp = m_lstValue;
		m_lstValue = NULL;

		return tmp;
}


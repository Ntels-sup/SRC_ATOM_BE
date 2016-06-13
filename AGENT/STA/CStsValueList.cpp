#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include "CGlobal.hpp"
#include "CStsValue.hpp"
#include "CStsValueList.hpp"
#include "CCollectValue.hpp"
#include "CStmAnsApi.hpp"

CStsValueList::CStsValueList()
{
    m_lstPrimaryKey = NULL;
    m_lstValue = new list<CStsValue*>;
}

CStsValueList::~CStsValueList()
{
    list<CStsValue*>::iterator iter;
   // CStsValue *cValue = NULL;

    if(m_lstPrimaryKey != NULL){
        while(m_lstPrimaryKey->size() != 0){
            m_lstPrimaryKey->pop_front();
        }

        delete m_lstPrimaryKey;
    }

#if 0
    for(iter = m_lstValue->begin();iter != m_lstValue->end();iter++){
        cValue = *iter;

        delete cValue;
    }
#else 
    while(m_lstValue->size()){
        delete m_lstValue->front();
        m_lstValue->pop_front();
    }
#endif

    delete m_lstValue;
}

int CStsValueList::InsertValue(time_t a_collectStartTime, time_t a_collectEndTime, list<int> *a_lstValue, 
				string *a_strFileName)
{
	int nRet = STA_OK;
	list<CStsValue*>::iterator iter;
	CStsValue *cValue = NULL;

	for(iter = m_lstValue->begin();iter != m_lstValue->end();iter++){
		cValue = *iter;

		nRet = cValue->CheckKey(a_collectStartTime, a_collectEndTime);
		if(nRet == STA_OK){
			STA_LOG(STA_ERR,"Value key duplicate (drop data)\n");
			delete a_lstValue;
			if((a_strFileName != NULL) && 
					(a_strFileName->size() != 0)){
				remove(a_strFileName->c_str());
			}
			return STA_NOK;
		}
	}

	cValue = new CStsValue();

	DEBUG_LOG("INSERT VALUE(START=%ld, END=%lu, DIFF=%lu)\n",
			a_collectStartTime, a_collectEndTime,
			a_collectEndTime - a_collectStartTime);

	cValue->InsertValue(a_collectStartTime, a_collectEndTime, a_lstValue, a_strFileName);

	if(a_strFileName != NULL){
		cValue->SetAggregationFlag();
	}

	m_lstValue->push_back(cValue);

	return STA_OK;
}

int CStsValueList::FileWrite(string a_strFileName, CStsValue *cValue)
{
    FILE *fd = NULL;
    list<string>::iterator primaryIter;
    list<int>::iterator valueIter;
    list<int> *lstValue = NULL;

    fd = fopen(a_strFileName.c_str(), "w");
    if(fd == NULL){
        STA_LOG(STA_ERR,"File open failed(fileName=%s, errno=%d(%s))\n",
                a_strFileName.c_str(), errno, strerror(errno));
        return STA_NOK;
    }

    /* write primary key */
    for(primaryIter = m_lstPrimaryKey->begin();primaryIter != m_lstPrimaryKey->end();primaryIter++){
        fprintf(fd,"%s:",(*primaryIter).c_str());
    }
    fprintf(fd,"\n");

    /* write value */
    lstValue = cValue->GetValueP();

    for(valueIter = lstValue->begin();valueIter != lstValue->end(); valueIter++){
        fprintf(fd,"%d:",(*valueIter));
    }

    fclose(fd);

    return STA_OK;
}

int CStsValueList::Aggregation(time_t a_endTime, string a_strDir, string a_strTableName, int a_nIndex)
{
    int nRet = 0;
    time_t collectEndTime = 0;
    bool blnCollectFlag = false;
    list<CStsValue*>::iterator iter;
    list<int> *lstValue = NULL;
    CStsValue *cCollectValue = NULL;
    CStsValue *cValue = NULL;
    int nFileNameLen = 0;
    char chFileName[FILE_NAME_LEN];
    string strFileName;

    for(iter = m_lstValue->begin();iter != m_lstValue->end();){
        cValue = *iter;

        collectEndTime = cValue->GetCollectEndTime();
        if(collectEndTime > a_endTime){
            break;
        }

        if(cCollectValue == NULL){
            if(cValue->GetAggregationFlag() == true){
                iter++;
                continue;
            }
            cCollectValue = cValue;
            cCollectValue->SetAggregationFlag();
            iter++;
            blnCollectFlag = false;
            continue;
        }

        if((cValue->GetCollectEndTime() - cCollectValue->GetCollectStartTime()) > 60){
            iter++;
            /* write value */
            nFileNameLen = snprintf(chFileName,FILE_NAME_LEN,"%lu_%lu_%d_%s%s",
                    cCollectValue->GetCollectStartTime(), 
                    cCollectValue->GetCollectEndTime(), a_nIndex, a_strTableName.c_str(), FILE_LOG_EXT);
            strFileName.clear();
            strFileName.append(a_strDir);
            strFileName.append("/");
            strFileName.append(chFileName);

            DEBUG_LOG("FILENAME=%s\n",strFileName.c_str());
            nRet = FileWrite(strFileName, cCollectValue);
            if(nRet != STA_OK){
                STA_LOG(STA_ERR,"File write failed(nRet=%d)\n",nRet);
            }

            cCollectValue->SetFileName(strFileName);

            cCollectValue = cValue;
            cCollectValue->SetAggregationFlag();

            blnCollectFlag = true;
            continue;
        }

        lstValue = cValue->GetValue();
        cCollectValue->UpdateValue(lstValue);
        cCollectValue->SetCollectEndTime(cValue->GetCollectEndTime());
        delete cValue;

        m_lstValue->erase(iter++);
        blnCollectFlag = false;
    }

    if((cCollectValue != NULL) && (blnCollectFlag == false)){
        /* write value */
        nFileNameLen = snprintf(chFileName,FILE_NAME_LEN,"%lu_%lu_%d_%s%s",
                cCollectValue->GetCollectStartTime(), 
                cCollectValue->GetCollectEndTime(), a_nIndex, a_strTableName.c_str(), FILE_LOG_EXT);
        strFileName.clear();
        strFileName.append(a_strDir);
        strFileName.append("/");
        strFileName.append(chFileName);

        DEBUG_LOG("FILENAME=%s\n",strFileName.c_str());
        nRet = FileWrite(strFileName, cCollectValue);
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"File write failed(nRet=%d)\n",nRet);
        }

        cCollectValue->SetFileName(strFileName);
    }

    return STA_OK;
}

CStsValue *CStsValueList::GetCollectValue(time_t a_startTime, int a_nCollectTime)
{
    time_t collectEndTime = 0;
    list<CStsValue*>::iterator iter;
    list<int> *lstValue = NULL;
    CStsValue *cValue = NULL;
    CStsValue *cCollectValue = NULL;

    for(iter = m_lstValue->begin();iter != m_lstValue->end();){
        cValue = *iter;

        collectEndTime = cValue->GetCollectEndTime();
        if(collectEndTime <= a_startTime){
            iter++;
            continue;
        }

        if(collectEndTime > (a_startTime + a_nCollectTime)){
            break;
        }

        if(cCollectValue == NULL){
            cCollectValue = cValue;
            cCollectValue->SetCollectTime(a_startTime, a_startTime + (time_t)a_nCollectTime);
        }
        else {
            lstValue = cValue->GetValue();
            cCollectValue->UpdateValue(lstValue);
            delete lstValue;
            delete cValue;
        }

        m_lstValue->erase(iter++);
    }

    return cCollectValue;
}

int CStsValueList::GetValueCount()
{
    return m_lstValue->size();
}

int CStsValueList::SetPrimaryKey(list<string> *a_lstPrimaryKey)
{
    if(m_lstPrimaryKey != NULL){
        delete m_lstPrimaryKey;
    }
    m_lstPrimaryKey = a_lstPrimaryKey;

    return STA_OK;
}

list<string>* CStsValueList::GetPrimaryKey()
{
    list<string> *lstCopyPrimary = NULL;
    list<string>::iterator iter;

    if(m_lstPrimaryKey == NULL){
        return NULL;
    }

    lstCopyPrimary = new list<string>;

    for(iter=m_lstPrimaryKey->begin();iter != m_lstPrimaryKey->end();iter++){
        lstCopyPrimary->push_back(*iter);
    }

    return lstCopyPrimary;
}

int CStsValueList::CheckKey(list<string> *a_lstPrimaryKey)
{
    int nRet = STA_OK;
    list<string>::iterator orgIter;
    list<string>::iterator checkIter;
    string orgString;
    string checkString;

    if((a_lstPrimaryKey == NULL) && (m_lstPrimaryKey == NULL)){
        return STA_OK;
    }

    if((a_lstPrimaryKey == NULL) || (m_lstPrimaryKey == NULL)){
        return STA_NOK;
    }

    if(a_lstPrimaryKey->size() != m_lstPrimaryKey->size()){
        return STA_NOK;
    }

    for(orgIter = m_lstPrimaryKey->begin(), checkIter = a_lstPrimaryKey->begin();
            orgIter != m_lstPrimaryKey->end();orgIter++, checkIter++){
        orgString = *orgIter;
        checkString = *checkIter;

        nRet = orgString.compare(checkString);
        if(nRet != 0){
            return STA_NOK;
        }
    }

    return STA_OK;
}

int CStsValueList::DropStsValue(time_t a_dropTime)
{
    time_t nCollectTime = 0;
    list<CStsValue*>::iterator iter;
    CStsValue *cValue = NULL;

    //for(iter = m_lstValue->begin();iter != m_lstValue->end();iter++){
    for(iter = m_lstValue->begin();iter != m_lstValue->end();){
        cValue = *iter;

        nCollectTime = cValue->GetCollectEndTime();
        if(nCollectTime > a_dropTime){
            break;
        }

        m_lstValue->erase(iter++);

        delete cValue;
    }

    return STA_OK;
}

int CStsValueList::DropAll()
{
    return STA_OK;
}


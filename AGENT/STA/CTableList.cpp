#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include "STA.h"
#include "CGlobal.hpp"
#include "CTableList.hpp"

CTableList::CTableList()
{
}

CTableList::~CTableList()
{
    map<string, CTableInfo*>::iterator iter;

    for(iter = m_mapTableMap.begin(); iter != m_mapTableMap.end();iter++){
        delete iter->second;
    }
}

int CTableList::InsertTableInfo(CTableInfo *a_cTable)
{
    string strTableName;

    if(a_cTable == NULL){
        STA_LOG(STA_ERR,"Table class not exist\n");
        return STA_NOK;
    }

    strTableName = a_cTable->GetTableName();

    m_mapTableMap.insert(map<string,CTableInfo*>::value_type(strTableName, a_cTable));

    return STA_OK;
}

CTableInfo* CTableList::SearchTableInfo(string a_strTableName)
{
    map<string, CTableInfo*>::iterator iter;

    iter = m_mapTableMap.find(a_strTableName);
    if(iter != m_mapTableMap.end()){
        return iter->second;
    }

    return NULL;
}

int CTableList::DeleteTableInfo(string a_strTableName)
{
    map<string, CTableInfo*>::iterator iter;

    iter = m_mapTableMap.find(a_strTableName);
    if(iter != m_mapTableMap.end()){
        delete iter->second;
        m_mapTableMap.erase(iter);
        return STA_OK;
    }

    STA_LOG(STA_ERR,"Table delete failed(table not exist)(tableName=%s\n", a_strTableName.c_str());
    return STA_NOK;
}

int CTableList::Aggregation(int a_endTime, string a_strDir)
{
    int nRet = 0;
    map<string, CTableInfo*>::iterator iter;
    CTableInfo *cTable = NULL;

    for(iter = m_mapTableMap.begin(); iter != m_mapTableMap.end();iter++){
        cTable = iter->second;
        nRet = cTable->Aggregation(a_endTime, a_strDir);
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"Table aggreagation failed(tableName=%s, endTime=%d, nRet=%d)\n",
                    cTable->GetTableName().c_str(), a_endTime, nRet);
            continue;
        }
    }
    return STA_OK;
}

time_t CTableList::GetNumberTime(const char *chSrc, unsigned int nLen)
{
    time_t nTot = 0;

    while(nLen != 0){
        nTot = nTot * 10 + *chSrc - '0';
        chSrc++;
        nLen--;
    }

    return nTot;
}

int CTableList::GetNumber(const char *chSrc, unsigned int nLen)
{
    int nTot = 0;

    while(nLen != 0){
        nTot = nTot * 10 + *chSrc - '0';
        chSrc++;
        nLen--;
    }

    return nTot;
}

int CTableList::ParseFileName(string a_strFileName, string &a_strTableName, time_t &a_startTime, time_t &a_endTime, 
				int &a_nSequence)
{
    unsigned int i = 0;
    int nIndex=0;
    int nStartTokenCur = 0;

    if(strncasecmp(&a_strFileName.c_str()[a_strFileName.size() - FILE_LOG_EXT_LEN],FILE_LOG_EXT, FILE_LOG_EXT_LEN)!=0){
        STA_LOG(STA_ERR,"Invalid ext(str=%s)\n", a_strFileName.c_str());
        return STA_NOK;
    }

    for(i=0;i<a_strFileName.size();i++){
        if(a_strFileName.c_str()[i] == '_'){
            switch(nIndex){
                case 0: /* START TIME */
                    {
                        a_startTime =  GetNumberTime(a_strFileName.c_str(), i);
                    }
                    break;
                case 1: /* END TIME */
                    {
                        /* end Time */
                        a_endTime = GetNumberTime(&a_strFileName.c_str()[nStartTokenCur], 
                                i - nStartTokenCur);

                    }
                    break;
                case 2: /* SEQUENCE AND TABLE NAME */
                    {
                        a_nSequence = GetNumber(&a_strFileName.c_str()[nStartTokenCur], 
                                i - nStartTokenCur);


                        i++;
                        if((i) == a_strFileName.size()){
                            STA_LOG(STA_ERR,"File Name not exist(str=%s)\n",
                                    a_strFileName.c_str());
                            return STA_NOK;
                        }
                        a_strTableName.append(&a_strFileName.c_str()[i],
                                (a_strFileName.size()-FILE_LOG_EXT_LEN-i));
                        return STA_OK;
                    }
                    break;
            };
            nStartTokenCur = i;
            nStartTokenCur++;
            nIndex++;
        }
    }

    return STA_OK;
}

int CTableList::LoadTableData(string a_strFileName, list<string> **a_lstPrimaryKey, list<int> **a_lstValue) 
{
    int i = 0;
    string strPrimaryKey;
    list<string> *lstPrimaryKey = NULL;
    list<int> *lstValue = NULL;
    int nLineIndex = 0;
    FILE *fd = NULL;
    int nCur = 0;
    int nLineLen = 0;
    int nStartTokenCur = 0;
    char chLine[FILE_LINE_BUFFER_LEN];
    string strToken;

    fd = fopen(a_strFileName.c_str(), "r");
    if(fd == NULL){
        STA_LOG(STA_ERR,"File not exist(%s)\n",a_strFileName.c_str());
        return STA_NOK;
    }

    while((fgets(&chLine[nCur], FILE_LINE_BUFFER_LEN, fd))){
        nLineLen = strlen(chLine);
        nStartTokenCur = 0;

        if((chLine[nLineLen-1] != '\n') && (nLineIndex == 0)){
            STA_LOG(STA_ERR,"Invalid primary key foramt (dataLine=%s)\n",chLine);
            if(lstPrimaryKey != NULL){
                delete lstPrimaryKey;
            }
            if(lstValue != NULL){
                delete lstValue;
            }
            fclose(fd);
            return STA_NOK;
        }
        else if((chLine[nLineLen-1] != ':') && (nLineIndex == 1)){
            STA_LOG(STA_ERR,"Invalid data foramt (dataLine=%s)\n",chLine);
            if(lstPrimaryKey != NULL){
                delete lstPrimaryKey;
            }
            if(lstValue != NULL){
                delete lstValue;
            }
            fclose(fd);
            return STA_NOK;

        }

        for(i=0;i<nLineLen;i++){
            if(chLine[i] == ':'){ /* find token */
                if(nLineIndex == 0){
                    if(lstPrimaryKey == NULL){
                        lstPrimaryKey = new list<string>;
                    }
                    strPrimaryKey.clear();
                    strPrimaryKey.append(&chLine[nStartTokenCur], i - nStartTokenCur);
                    lstPrimaryKey->push_back(strPrimaryKey);
                }
                else {
                    if(lstValue == NULL){
                        lstValue = new list<int>;
                    }
                    lstValue->push_back(GetNumber(&chLine[nStartTokenCur], i - nStartTokenCur));
                }
                i++;
                nStartTokenCur = i;
            }
        }
        nLineIndex++;
    }

    *a_lstPrimaryKey = lstPrimaryKey;
    *a_lstValue = lstValue;
    fclose(fd);

    return STA_OK;
}

int CTableList::LoadTableFromFile(string a_strDir)
{
    int nRet = 0;
    DIR *stDirInfo = NULL;
    struct dirent *stDirEntry;
    time_t startTime = 0;
    time_t endTime = 0;
    int nSequence = 0;
    CTableInfo *cTable = NULL;
    string strFileName;
    string strTableName;
    list<string> *lstPrimaryKey = NULL;
    list<int> *lstValue = NULL;

    stDirInfo = opendir(a_strDir.c_str());
    if(stDirInfo == NULL){
        STA_LOG(STA_ERR,"STA LOG Directory not exist(path=%s)\n", a_strDir.c_str());
        return STA_OK;
    }

    while((stDirEntry = readdir(stDirInfo)) != NULL){
        if((strcasecmp(stDirEntry->d_name,".") == 0) ||
                (strcasecmp(stDirEntry->d_name,"..") == 0)){
            continue;
        }
        strFileName.clear();
        strFileName.append(stDirEntry->d_name);
        strTableName.clear();
        nRet = ParseFileName(strFileName, strTableName, startTime, endTime, nSequence);
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"STA LOG parse failed(nRet=%d)\n",nRet);
            continue;
        }

        if(startTime >= endTime){
            STA_LOG(STA_ERR,"Invalid Time sequence(starTime=%lu, endTime=%lu)\n",startTime, endTime);
            remove(strFileName.c_str());
        }

        STA_LOG(STA_ERR,"LOAD TABLE(NAME=%s)(STIME=%lu, ETIME=%lu, DIFF=%lu\n", strFileName.c_str(),
                startTime, endTime, endTime - startTime);
        cTable = SearchTableInfo(strTableName);
        if(cTable == NULL){
            cTable = new CTableInfo(strTableName);
            InsertTableInfo(cTable);
        }

        /* Load Table info */
        strFileName.insert(0,"/");
        strFileName.insert(0,a_strDir);
        lstPrimaryKey = NULL;
        lstValue = NULL;
        nRet = LoadTableData(strFileName, &lstPrimaryKey, &lstValue);
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"STA LOG Load failed(nRet=%d)\n",nRet);
            delete cTable;
            continue;
        }

        nRet = cTable->InsertValue(startTime,endTime, lstPrimaryKey, lstValue, &strFileName);
        if(nRet != STA_OK){
            if(lstPrimaryKey != NULL){
                delete lstPrimaryKey; 
            }
            //delete lstValue;
            delete cTable;
            STA_LOG(STA_ERR,"STS Value insert failed(nRet=%d)\n",nRet);
        }
    }

    closedir(stDirInfo);

    return STA_OK;
}

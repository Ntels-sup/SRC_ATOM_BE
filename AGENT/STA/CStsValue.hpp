#ifndef __CSTSVALUE_HPP__
#define __CSTSVALUE_HPP__
#include <string>
#include <list>
#include <time.h>

#include "STA.h"

using namespace std;

class CStsValue{
    private:
        bool m_blnAggregationFlag;
        time_t m_collectStartTime;
        time_t m_collectEndTime;
        string m_strFileName;
        list<int> *m_lstValue;
    public:
        CStsValue();
        ~CStsValue();
        void SetFileName(string a_strFileName){ m_strFileName = a_strFileName; };
        void ChangeFileName(string a_strFileName){	m_strFileName = a_strFileName; };
        void DelFilenName() { m_strFileName.empty(); };
        bool FileExist() {
            if(m_strFileName.size() != 0){ 
                return true; 
            } 
            else {
                return false; 
            } 
        };
        int InsertValue(time_t a_collectStartTime, time_t a_collectEndTime, list<int> *a_lstValue, string *a_strFileName);
        int UpdateValue(list<int> *a_lstValue);
        void SetCollectTime(time_t a_collectStartTime, time_t a_collectEndTime){
            m_collectStartTime = a_collectStartTime;
            m_collectEndTime = a_collectEndTime;
        };
        void SetCollectEndTime(time_t a_collectEndTime){ m_collectEndTime = a_collectEndTime; };
        void SetAggregationFlag(){ m_blnAggregationFlag = true; };
        bool GetAggregationFlag(){ return m_blnAggregationFlag; };
        int CheckKey(time_t a_collectStartTime, time_t a_collectEndTime);
        void UpdateCollectTime(time_t a_collectStartTime, time_t a_collectEndTime);
        time_t GetCollectStartTime() { return m_collectStartTime; };
        time_t GetCollectEndTime() { return m_collectEndTime; };
        list<int> *GetValue() {
            list<int> *cTmp = NULL;
            cTmp = m_lstValue; 
            m_lstValue = NULL; 
            return cTmp;
        }
        list<int> *GetValueP() {
            return m_lstValue; 
        }
};

#endif

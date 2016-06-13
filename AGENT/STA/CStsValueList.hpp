#ifndef __CSTSVALUELIST_HPP__
#define __CSTSVALUELIST_HPP__

#include <list>
#include <string>
#include "CStsValue.hpp"

using namespace std;

class CStsValueList{
    private:
        list<string> *m_lstPrimaryKey;
        list<CStsValue*> *m_lstValue;

    public:
        CStsValueList();
        ~CStsValueList();
        int InsertValue(time_t a_nCollectStartTime, time_t a_nCollectEndTime, 
                list<int> *a_lstValue, string *a_strFileName);
        int FileWrite(string a_strFileName, CStsValue *cValue);
        int Aggregation(time_t a_endTime, string a_strDir, string a_strTableName, int a_nIndex);
        CStsValue *GetCollectValue(time_t a_startTime, int a_nCollectTime);
        int CheckKey(list<string> *a_lstPrimaryKey);
        int GetValueCount();
        int SetPrimaryKey(list<string> *a_lstPrimaryKey);
        int DropStsValue(time_t a_dropTime);
        list<string>* GetPrimaryKey();
        int DropAll();
};

#endif

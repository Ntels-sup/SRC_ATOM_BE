#include <stdio.h>
#include "STMAPI.h"
#if 0
#include"CCollectValue.hpp"
#include"CStmReqEncApi.hpp"
#include"CStmReqDecApi.hpp"
#include"CStmAnsEncApi.hpp"
#include"CStmAnsDecApi.hpp"
#endif
#include "CStmAnsApi.hpp"
#include "CStmReqApi.hpp"

int func()
{
		int i = 0;
		char testData[1024];
		unsigned int testDataLen = 0;
		string tableName = "TEST_TABLE";
		string primaryKey = "TEST_KEY";
		CStmAnsApi encApi;
		CStmAnsApi decApi;
		list<CCollectValue*> *collectList = NULL;
		CCollectValue *cCollectValue = NULL;

		encApi.Init(1, 1, tableName, STMAPI_TABLE_TYPE_STS);
		collectList = new list<CCollectValue*>;
		for(i=0;i<3;i++){
				list<string> *m_lstPrimaryKey = new list<string>;
				list<int> *m_lstValue = new list<int>;

				cCollectValue = new CCollectValue;

				m_lstPrimaryKey->push_back(primaryKey);
				m_lstValue->push_back(10);
				m_lstValue->push_back(11);

				cCollectValue->SetPrimaryKey(m_lstPrimaryKey);
				cCollectValue->SetValue(m_lstValue);

				collectList->push_back(cCollectValue);
		}

		encApi.SetCollectValue(collectList);
		encApi.EncodeMessage(testData, 1024, &testDataLen);

		testData[testDataLen] = '\0';

		printf("ANSWER = %s\n", testData);

		decApi.DecodeMessage(testData,testDataLen);

		return 0;
}

int func2()
{
		char testData[1024];
		unsigned int testDataLen = 0;
		CStmReqApi encApi;
		CStmReqApi decApi;

		encApi.Init(1, time(NULL), 5);

		encApi.SetTableName("TEST_TABLE");

		encApi.EncodeMessage(testData, 1024, &testDataLen);

		testData[testDataLen] = '\0';
		printf("REQUEST = %s\n",testData);

		decApi.DecodeMessage(testData, testDataLen);
		return 0;
}


int main()
{
		func();
		func2();

		return 0;
}

#include <stdio.h>
#include "string.h"
#include "STAAPI.h"
#include "CTableData.hpp"
#include "CStaAnsApi.hpp"
#include "CStaReqApi.hpp"

int func()
{
		CStaAnsApi encApi;
		CStaAnsApi decApi;
		time_t tm = time(NULL);

		encApi["TEST_TABLE"].SetTableInfo(STAAPI_TABLE_TYPE_STS, tm);
		encApi["TEST_TABLE"].InsertPrimaryKey("PRI1");
		encApi["TEST_TABLE"].InsertPrimaryKey("PRI2");
		encApi["TEST_TABLE"].InsertValue(10);
		encApi["TEST_TABLE"].InsertValue(20);

		encApi["TEST_TABLE_2"].SetTableInfo(STAAPI_TABLE_TYPE_STS, tm);
		encApi["TEST_TABLE_2"].InsertPrimaryKey("PRI3");
		encApi["TEST_TABLE_2"].InsertPrimaryKey("PRI4");
		encApi["TEST_TABLE_2"].InsertValue(30);
		encApi["TEST_TABLE_2"].InsertValue(40);

#if 0
		std::string testBuffer;
		encApi.EncodeMessage(testBuffer);
		printf("STR:%s\n",testBuffer.c_str());
		decApi.DecodeMessage(testBuffer);

#else
		unsigned int encLen = 0;
		char EncBuffer[1024];
		encApi.EncodeMessage(EncBuffer, 1024, &encLen);
		EncBuffer[encLen] = '\0';
		printf("CHAR:%s\n",EncBuffer);
		decApi.DecodeMessage(EncBuffer, encLen);
#endif

		return 0;
}

int func2()
{
		CStaReqApi decApi;
		CStaReqApi encApi;

		encApi.SetTime(time(NULL));

#if 1
		std::string testBuffer;
		encApi.EncodeMessage(testBuffer);
		printf("%s\n",testBuffer.c_str());
		decApi.DecodeMessage(testBuffer);

#else
		unsigned int encLen = 0;
		char EncBuffer[1024];
		encApi.EncodeMessage(EncBuffer, 1024, &encLen);
		EncBuffer[encLen] = '\0';
		printf("%s\n",EncBuffer);
		decApi.DecodeMessage(EncBuffer, encLen);
#endif

		return 0;
}

int main()
{
		func();
		func2();
		return 0;
}

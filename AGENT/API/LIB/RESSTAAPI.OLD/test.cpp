#include <stdio.h>
#include "string.h"
#include <string>
#include <iostream>
#include "CStaResRspApi.hpp"
#include "CStaResReqApi.hpp"

using namespace std;

int func()
{
		CStaResReqApi encApi;
		CStaResReqApi decApi;
		std::string strData;

		encApi.InsertRscGrpId((char*)"RSC_GRP_01");
		encApi.InsertRscGrpId((char*)"RSC_GRP_02");
		encApi.InsertRscGrpId((char*)"RSC_GRP_03");

		encApi.EncodeMessage(strData);

		decApi.DecodeMessage(strData);

		strData = decApi.GetFirstRscGrpId();
		cout << strData << endl;
		strData = decApi.GetFirstRscGrpId();
		cout << strData << endl;
		strData = decApi.GetFirstRscGrpId();
		cout << strData << endl;

		return 0;
}

int func2()
{
		CStaResRspApi encApi;
		CStaResRspApi decApi;
		std::string strData;

		encApi["RSC_GRP_01"].Init(time(NULL), 1, (char*)"5", 5.01);
		encApi["RSC_GRP_01"].Init(time(NULL), 6, (char*)"5", 5.01);
		encApi["RSC_GRP_02"].Init(time(NULL), 2, (char*)"6", 5.02);
		encApi["RSC_GRP_03"].Init(time(NULL), 3, (char*)"7", 6.03);

		encApi.EncodeMessage(strData);	

		printf("RSP MSG=%s\n", strData.c_str());

		decApi.DecodeMessage(strData);

		cout << decApi.GetTableCount() << endl;

		cout<< "prc=" << decApi["RSC_GRP_01"].GetPrcDate() << endl;
		cout<< "nodeNo=" << decApi["RSC_GRP_01"].GetNodeNo() << endl;
		cout<< "ColumnOrderCCD=" << decApi["RSC_GRP_01"].GetColumnOrderCCD() << endl;
		cout<< "StatData=" << decApi["RSC_GRP_01"].GetStatData() << endl;

		cout<< "prc=" << decApi["RSC_GRP_02"].GetPrcDate() << endl;
		cout<< "nodeNo=" << decApi["RSC_GRP_02"].GetNodeNo() << endl;
		cout<< "ColumnOrderCCD=" << decApi["RSC_GRP_02"].GetColumnOrderCCD() << endl;
		cout<< "StatData=" << decApi["RSC_GRP_02"].GetStatData() << endl;

		cout<< "prc=" << decApi["RSC_GRP_03"].GetPrcDate() << endl;
		cout<< "nodeNo=" << decApi["RSC_GRP_03"].GetNodeNo() << endl;
		cout<< "ColumnOrderCCD=" << decApi["RSC_GRP_03"].GetColumnOrderCCD() << endl;
		cout<< "StatData=" << decApi["RSC_GRP_03"].GetStatData() << endl;

		return 0;
}

int main()
{
		func();
		func2();
		return 0;
}

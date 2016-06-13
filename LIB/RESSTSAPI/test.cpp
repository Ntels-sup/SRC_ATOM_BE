#include <stdio.h>
#include "string.h"
#include <string>
#include <iostream>
#include "CStmResRspApi.hpp"
#include "CStmResReqApi.hpp"

using namespace std;

int func()
{
		CStmResReqApi encApi;
		CStmResReqApi decApi;
		std::string strData;

		encApi.Init(1, time(NULL));
		encApi.InsertRscGrpId((char*)"RSC_GRP_01");
		encApi.InsertRscGrpId((char*)"RSC_GRP_02");
		encApi.InsertRscGrpId((char*)"RSC_GRP_03");

		encApi.EncodeMessage(strData);
		printf("REQ MSG=%s\n", strData.c_str());

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
		CStmResRspApi encApi;
		CStmResRspApi decApi;
		std::string strData;

		encApi.Init(1, CStmResApi::RESAPI_RESULT_CODE_SUCCESS);
		encApi["RSC_GRP_01"]["1"].Init(time(NULL), 1, 5.01);
		encApi["RSC_GRP_01"]["2"].Init(time(NULL), 1, 3.01);
		encApi["RSC_GRP_01"]["1"].Init(time(NULL), 6, 5.01);
		encApi["RSC_GRP_02"]["2"].Init(time(NULL), 2, 5.02);
		encApi["RSC_GRP_03"]["3"].Init(time(NULL), 3, 6.03);

		encApi.EncodeMessage(strData);	

		printf("RSP MSG=%s\n", strData.c_str());

		decApi.DecodeMessage(strData);

		cout << decApi.GetResGroupCount() << endl;

		cout << "GRP_01 count" << decApi["RSC_GRP_01"].GetResTableDataCount() << endl;
		cout<< "prc=" << decApi["RSC_GRP_01"]["1"].GetPrcDate() << endl;
		cout<< "nodeNo=" << decApi["RSC_GRP_01"]["1"].GetNodeNo() << endl;
		cout<< "columnOrder=" << decApi["RSC_GRP_01"]["1"].GetColumnOrder() << endl;
		cout<< "StatData=" << decApi["RSC_GRP_01"]["1"].GetStatData() << endl;

		cout<< "\n\n" << endl;

		cout << "GRP_02 count" << decApi["RSC_GRP_02"].GetResTableDataCount() << endl;
		cout<< "prc=" << decApi["RSC_GRP_02"]["2"].GetPrcDate() << endl;
		cout<< "nodeNo=" << decApi["RSC_GRP_02"]["2"].GetNodeNo() << endl;
		cout<< "ColumnOrder=" << decApi["RSC_GRP_02"]["2"].GetColumnOrder() << endl;
		cout<< "StatData=" << decApi["RSC_GRP_02"]["2"].GetStatData() << endl;
		cout<< "\n\n" << endl;

		cout << "GRP_03 count" << decApi["RSC_GRP_03"].GetResTableDataCount() << endl;
		cout<< "prc=" << decApi["RSC_GRP_03"]["3"].GetPrcDate() << endl;
		cout<< "nodeNo=" << decApi["RSC_GRP_03"]["3"].GetNodeNo() << endl;
		cout<< "ColumnOrder=" << decApi["RSC_GRP_03"]["3"].GetColumnOrder() << endl;
		cout<< "StatData=" << decApi["RSC_GRP_03"]["3"].GetStatData() << endl;
		cout<< "\n\n" << endl;

		return 0;
}

int main()
{
		func();
		func2();
		return 0;
}

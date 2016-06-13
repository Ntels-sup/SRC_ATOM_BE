#include <stdio.h>
#include "CRegReqApi.hpp"
#include "CRegRespApi.hpp"

void func1()
{
		int nRet = 0;
		CRegReqApi encApi;
		CRegReqApi decApi1;
		CRegReqApi decApi2;
		std::string strData;
		std::vector<char> vecData;


		nRet = encApi.Init((char*)"ATOM", (char*)"EMS", (char*)"SWM", 12);

		encApi.EncodeMessage(strData);
		encApi.EncodeMessage(vecData);

		decApi1.DecodeMessage(strData);
		decApi2.DecodeMessage(vecData);

}

void func2()
{
		int nRet = 0;
		CRegRespApi encApi;
		CRegRespApi decApi1;
		CRegRespApi decApi2;
		std::string strData;
		std::vector<char> vecData;


		nRet = encApi.Init(2, (char*)"OK");

		encApi.EncodeMessage(strData);
		encApi.EncodeMessage(vecData);

		decApi1.DecodeMessage(strData);
		decApi2.DecodeMessage(vecData);

printf("END(%d, %s)\n", decApi2.GetCode(), decApi2.GetText().c_str());
}

int main()
{

		func1();
		func2();

		return 0;
}

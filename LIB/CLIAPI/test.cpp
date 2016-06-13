#include <iostream>
#include <string>
#include "CCliReq.hpp"
#include "CCliRsp.hpp"

void CmdReq()
{
	unsigned int i = 0;
	unsigned int j = 0;
	std::string output;
	//CCliReq encReq(1, 1000, (char*)"ADD-DIAM-PEER");
	CCliReq encReq(1, 1000, (char*)"ADD-DIAM-PEER", "OFCS");
	CCliReq decReq;

	try {
		encReq["HOST"] = "S-CSCF.STK.COM";
		encReq["REALM"] = "STK.COM";
		encReq["IP"] = "192.168.0.1";
		encReq["IP"] = "192.168.0.2";
		encReq["PORT"] = 3868;

		// throw test 
		//std::cout << "TEST = " << encReq[30][0].GetString() << std::endl;

		if(encReq["PEER"].GetCount() != 0){
			std::cout << "PEER = " << encReq["PEER"][0].GetString() << std::endl;
		}
		for(i=0;i<encReq["IP"].GetCount();i++){
			std::cout << "IP = " << encReq["IP"][i].GetString() << std::endl;
		}
		encReq["IP"].PopFirst();
		std::cout << "IP POP FIRST" << std::endl;
		for(i=0;i<encReq["IP"].GetCount();i++){
			std::cout << "IP = " << encReq["IP"][i].GetString() << std::endl;
		}

		std::cout << "PORT =" << encReq["PORT"][0].GetNumber() << std::endl;

		encReq.Pop("PORT");
		//std::cout << "PORT =" << encReq["PORT"][0].GetNumber() << std::endl;
		//
		encReq.PopLast();

		for(i=0;i<encReq.GetCount();i++){
			std::cout << "INDEX[" << i << "]=" << encReq[i][0].GetString() << std::endl;
		}

		encReq.EncodeMessage(output);

		std::cout << "JSON : " << std::endl;
		std::cout << output << std::endl;

		decReq.DecodeMessage(output);


		std::cout << "SESSIONID : " << decReq.GetSessionId() << std::endl;
		if(decReq.PkgIsExist()){
		std::cout << "PKG : " << decReq.GetPkgName() << std::endl;
		}
		std::cout << "COMMAND CODE : " << decReq.GetCmdCode() << std::endl;
		std::cout << "COMMAND : " << decReq.GetCmdName() << std::endl;

		for(i=0;i<decReq.GetCount();i++){
			std::cout << decReq[i].GetName() << "(" << decReq[i].GetCount() << ")" <<" : ";
			for(j=0;j<decReq[i].GetCount();j++){
				if(decReq[i][j].IsNumber() == true){
					std::cout << " " << decReq[i][j].GetNumber();
				}
				else {
					std::cout << " "<< decReq[i][j].GetString();
				}

			}
			std::cout << std::endl;
		}

	}
	catch(std::string &e){
		std::cout << e << std::endl;

	}
}

void CmdRsp()
{
	std::string output;
	CCliRsp encApi(1, CCliRsp::SEQ_TYPE_END, 1);
	CCliRsp decApi;

	encApi.NPrintf(1024, "HOST            REALM      IP           PORT\n");
	encApi.NPrintf(1024, "-------------------------------------------------\n");
	encApi.NPrintf(1024, "%-15s %-10s %-12s %-10d\n","CSCF.STK.COM", "STK.COM", "192.168.0.1", 3868);
	encApi.NPrintf(1024, "%-15s %-10s %-12s %-10d\n","TAS.STK.COM", "STK.COM", "192.168.0.1", 3868);
	encApi.NPrintf(1024, "%-15s %-10s %-12s %-10d\n","HLR.STK.COM", "STK.COM", "192.168.0.1", 3868);
	encApi.NPrintf(1024, "-------------------------------------------------\n");

	encApi.EncodeMessage(output);

	std::cout << "JSON : " << std::endl;
	std::cout << output << std::endl;

	decApi.DecodeMessage(output);

	std::cout << "SESSION : "<< decApi.GetSessionId() << std::endl;
	std::cout << "SEQTYPE : "<< decApi.GetSeqType() << std::endl;
	std::cout << "TEXT : " << std::endl;
	std::cout << decApi.GetText() << std::endl;


}

int main()
{
	CmdReq();
	CmdRsp();

	return 0;
}

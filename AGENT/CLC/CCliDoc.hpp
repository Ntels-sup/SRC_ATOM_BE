#ifndef __CCLIDOC_HPP__
#define __CCLIDOC_HPP__
#include "lnkLst.h"
#include "cli.h"
#include "CCliReq.hpp"

using namespace std;

class CCliDoc{
	private:
	public:
		CCliDoc() {};
		~CCliDoc() {};
		int ArgEncode(CliArgDoc *a_stArgDoc, CCliReq &a_cReq);
		int Encode(CliDoc *a_stDoc, CCliReq &a_cReq);
};	

#endif

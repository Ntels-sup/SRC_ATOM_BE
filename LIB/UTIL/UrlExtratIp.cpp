#ifdef UNIT_TEST
#include <iostream>
#include <cstdio>
#endif
#include <string>

using std::string;

bool UrlExtratIp(const char* a_szUrl, std::string& a_strIp)
{
	if (a_szUrl == NULL) {
		return false;
	}

	string strUrl = a_szUrl;
	
	int nBegin = strUrl.find_first_of("/");
	if (nBegin <= 0) {
		return false;
	}
	nBegin += 2;

	int nEnd = strUrl.find_first_of(":/", nBegin);
	if (nEnd <= 0) {
		return false;
	}

	a_strIp = strUrl.substr(nBegin, nEnd - nBegin);
	return true;
}

#ifdef UNIT_TEST
using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
	string strUrl = "http://100.100.1.23:3000/D:/Test/PKG_REPOSITORY/vOFCS/vOFCS_AP/1/vOFCS_AP_20160329.tar.gz";
	string strIp;

	UrlExtratIp(strUrl.c_str(), strIp);

	cout << strUrl << endl;
	cout << strIp << endl;


	return 0;
}

#endif

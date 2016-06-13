#ifndef __CCLIROUTE_HPP__
#define __CCLIROUTE_HPP__
#include <string>
#include <list>

using namespace std;

class CCliRoute{
	private:
		typedef struct {
			unsigned int unCmdCode;
			string strCmdName;
			string strPkgName;
			string strDest;
			int unNodeNo;
			int unDstNo;
		} STRouteInfo;

		list<STRouteInfo*> m_lstRoute;
	public:
		CCliRoute();
		~CCliRoute();
		int Insert(unsigned int a_unCmdCode, string &a_strCmdName, string &a_strPkgName, string &a_strDest);
		//int Insert(unsigned int a_unCmdCode, const char *a_szCmdName, const char *a_szPkgName, const char *a_szDest);
		int Insert(unsigned int a_unCmdCode, const char *a_szCmdName, const char *a_szPkgName, unsigned int a_szDest);
		int Delete(string &a_strCmdName, string &a_strPkgName);
		int FindDestinationNo(string &a_strCmdName, string &a_strPkgName, int *unDstNo, int *unDstNodeNo);
		string *FindDestination(string &a_strCmdName, string &a_strPkgName);
};

#endif

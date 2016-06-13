#ifndef __CMAIN_HPP__
#define __CMAIN_HPP__
#include "CModule.hpp"
#include "CModuleIPC.hpp"
#include <list>
#include "CAppInfo.hpp"
#include "CEmsInfo.hpp"

using namespace std;

class CMain{
	private:
		CAppInfo *m_cAppInfo;
		CEmsInfo *m_cEmsInfo;
		CConfig m_cConfig;
		CModule::ST_MODULE_OPTIONS *stOption;
		CTableList *m_cCollectTableList;
		string m_strStsDumpLogPath;
		CModuleIPC *cIPC;

	public:
		CMain(const char *a_szPackageName, const char *a_szCfgFile, int &nRet);
		~CMain();
		int Run();
		int ReceiveProcess();
};

#endif

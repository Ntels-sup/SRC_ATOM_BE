#ifndef __CGLOBAL_HPP__
#define __CGLOBAL_HPP__
#include <string>
#include "CModule.hpp"
#include "CModuleIPC.hpp"
#include "CFileLog.hpp"

class CGlobal{
	private:
		static CGlobal *cInstance;
		CFileLog *cLog;
		CModule::ST_MODULE_OPTIONS *stOption;
		CModuleIPC *cIpc;
		CAddress *cAddr;

		CGlobal();
		~CGlobal();
	public:
		static CGlobal *GetInstance();
		void SetModuleOption(CModule::ST_MODULE_OPTIONS *a_stOption) {
			stOption = a_stOption; 
			cIpc = stOption->m_pclsModIpc;
			cAddr = stOption->m_pclsAddress;
		};

		int GetLocalNodeNo() {
			return stOption->m_nNodeNo;
		};

		int GetLocalProcNo() {
			return stOption->m_nProcNo;
		};

		CModuleIPC *GetModuleIPC(){
			return cIpc;
		};

		CAddress *GetAddress(){
			return cAddr;
		}

		static int GetCmdCode(std::string &a_strCmdCode){
			if(a_strCmdCode.size() < 10){
				return 0;
			}

			return atoi(&a_strCmdCode.c_str()[4]);
		};

		CFileLog *GetLogP();
		int ForceDir(const char *a_szDir);
};

#endif

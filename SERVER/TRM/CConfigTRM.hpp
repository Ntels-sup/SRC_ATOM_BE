#ifndef CCONFIGTRM_HPP__
#define CCONFIGTRM_HPP__

#include <string>
#include "TRM_Define.hpp"

using namespace std;

class CConfigTRM
{
	private:
		static CConfigTRM* _instance; 	// singletom instance

	public:
		CConfigTRM();
		~CConfigTRM();

		struct ST_DB_CFG
		{	
			string		m_strDbAddr;
			unsigned int m_nDbPort;
			string		m_strDbName;
			string		m_strDbuserId;
			string		m_strDbPasswd;
		} DB;
		struct ST_NM_CFG
		{
			unsigned int m_nServPort;
			string		m_strNMAddr;
			unsigned int m_nNMPort;
		} NM;

		struct ST_LOG_CFG
		{
			string		m_strLogPath;
			string		m_strModulePath;
			string		m_strProcName;
            string      m_strPkgName;
            string      m_strNodeName;
            string      m_strTrmName;
            string      m_strNmName;
            string      m_strWsmName;
			unsigned int m_nDuplCnt;
			string		m_nLevel;
		} LOG;

		struct ST_TRM_LOG
		{
			string		m_strTraceDataPath;
			unsigned int m_nTimeout;
			unsigned int m_nTraceCnt;
			unsigned int m_nAutoExpireTime;
		} TRM;

		static 	CConfigTRM& Instance(void);
		bool 	Loading(void);
		int		Initial();
};

#endif

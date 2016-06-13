#ifndef CCONFIGBJM_HPP__
#define CCONFIGBJM_HPP__

#include <string>
#include "BJM_Define.hpp"

using namespace std;

class CConfigBJM
{
	private:
		static 			CConfigBJM* _instance; 	// singletom instance

	public:
		CConfigBJM();
		~CConfigBJM();

		struct ST_DB_CFG
		{	
			string		m_strDbAddr;
			int			m_nDbPort;
			string		m_strDbName;
			string		m_strDbuserId;
			string		m_strDbPasswd;
		} DB;
		struct ST_NM_CFG
		{
			int			m_nServPort;
			string		m_strNMAddr;
			int			m_nNMPort;
		} NM;

		struct ST_LOG_CFG
		{
			string		m_strLogPath;
			string		m_strModulePath;
			string		m_strProcName;
			string		m_strPkgName;
			string		m_strNodeName;
			string		m_strBjmName;
			string		m_strNmName;
			string		m_strWsmName;
			int			m_nDuplCnt;
			string		m_nLevel;
		} LOG;

		struct ST_BJM_CFG
		{
			int 		m_nTimecount;
		} BJM;

		static 			CConfigBJM& Instance(void);
		bool 			Loading(void);
        int     		Initial();

};

#endif

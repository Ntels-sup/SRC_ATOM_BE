/* vim:ts=4:sw=4
 */

/**
 * \file	CConfigNa.hpp
 * \brief	
 *
 * $Author: junls&ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CONFIGNA_HPP_
#define CONFIGNA_HPP_

#include "CFileLog.hpp"
#include "DB.hpp"
#include "CLQManager.hpp"

#include <string>
#include <vector>


class CConfigNa
{
public:
	std::string	m_strCfgFile;
	bool		m_bIsVnfMode;
	int			m_nVnfWaitTime;

	std::string	m_strLogPath;
	std::string m_strModPath;

	std::string m_strServIp;
	int			m_nServPort;

	std::string	m_strUuid;
	std::string	m_strPkgName;
	std::string	m_strNodeType;
	std::string	m_strNodeName;
	std::string	m_strProcName;
	std::string	m_strNodeIp;
	int			m_nNodeNo;
	int			m_nProcNo;
	std::string	m_strVersion;

	std::string	m_strDbIp;
	int			m_nDbPort;
	std::string	m_strDbUser;
	std::string	m_strDbPasswd;
	std::string	m_strDbName;

	std::vector<std::string> m_vecModule;
	
	std::string	m_strPkgMngPath;

	DB*			m_pclsDB;

public:
	static CConfigNa& Instance(void);

	bool	LoadCfgFile(const char* a_szCfgFile = NULL);

	bool	ProbeEmsIP(void);
	bool	ProbeModule(std::vector<std::string>& a_vecLoadModule);

	int		ProbeNodeNo(const char* a_szPkgName, const char* a_szNodeType);
	int 	ProbeProcNo(const char* a_szPkgName,
							 const char* a_szNodeType, const char* a_szProcName);

	bool	DBSetup(void);
	bool	AppQInit(void);

	void	ConfigPrint(CFileLog* a_pclsLog);

private:
	static CConfigNa*  m_pclsInstance;          // singleton instance
	CConfigNa();
	CConfigNa(CConfigNa&) {}
	~CConfigNa();
};

#endif

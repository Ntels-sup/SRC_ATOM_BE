/* vim:ts=4:sw=4
 */

/**
 * \file	CConfigPra.hpp
 * \brief	
 *
 * $Author: junls&ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CONFIGPRA_HPP_
#define CONFIGPRA_HPP_

#include <string>
#include <list>

#include "DB.hpp"

#include "CProcessManager.hpp"


class CConfigPra
{
public:
	std::string	m_strCfgFile;
	std::string	m_strLogPath;

	std::string	m_strPkgName;
	std::string	m_strNodeType;
	std::string	m_strNodeName;
	std::string	m_strProcName;
	int			m_nNodeNo;
	int			m_nProcNo;

	std::string	m_strDbIp;
	int			m_nDbPort;
	std::string	m_strDbUser;
	std::string	m_strDbPasswd;
	std::string	m_strDbName;

	DB*			m_pclsDB;


public:
	static CConfigPra& Instance(void);

	bool	LoadCfgFile(const char* a_szCfgFile = NULL);

	int		LoadProcess(std::list<CProcessManager::ST_APPINFO>& a_lstProcess);

	bool	LoadProcessFile(void);
	bool	SaveProcessFile(void);

	void	ConfigPrint(void);

private:
	static CConfigPra*  m_pclsInstance;          // singleton instance
	CConfigPra();
	CConfigPra(CConfigPra&) {}
	~CConfigPra() {}
};

#endif

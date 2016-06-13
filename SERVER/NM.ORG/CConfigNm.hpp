/* vim:ts=4:sw=4
 */

/**
 * \file	CConfigNm.hpp
 * \brief	
 *
 * $Author: junls&ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CONFIGNA_HPP_
#define CONFIGNA_HPP_

#include <string>
#include <vector>

#include "CFileLog.hpp"


class CConfigNm
{
public:
	std::vector<std::string> m_vecModule;

	std::string	m_strLogPath;
	std::string	m_strListenIp;
	int			m_nListenPort;

	std::string	m_strModPath;
	
	std::string m_strDbIp;
	int			m_nDbPort;
	std::string m_strDbUser;
	std::string m_strDbPasswd;
	std::string m_strDbName;

	std::string m_strPkgName;
	std::string m_strNodeType;
	std::string	m_strProcName;
	
	int			m_nNodeNo;
	int			m_nProcNo;

public:
	static CConfigNm& Instance(void);

	bool		LoadCfgFile(const char* a_szCfgFile);
	bool		ProbeModule(std::vector<std::string> a_vecLoadModule);
	void		ConfigPrint(CFileLog* a_pclsLog);

private:
	static CConfigNm*  m_pcInstance;          // singleton instance

	CConfigNm();
	~CConfigNm() {}
	CConfigNm(const CConfigNm&) {}
};

#endif

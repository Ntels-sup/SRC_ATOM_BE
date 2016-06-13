/* vim:ts=4:sw=4
 */
/**
 * \file	CAddress.hpp
 * \brief	Node ID, Process ID 정보 조회
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#ifndef CADDRESS_HPP_
#define CADDRESS_HPP_

#include <map>
#include <string>
#include <list>

#include "CFileLog.hpp"

class CAddress
{
public:
	struct ST_ADDR {
		std::string	m_strPkgName;
		std::string	m_strNodeType;
		std::string	m_strProcName;
		int			m_nNodeNo;
		int			m_nProcNo;
		bool		m_bApp;
	};

public:
	static CAddress& Instance(void);

	bool	SetLocalPkg(std::string& a_strPkgName, std::string& a_strNodeType);
	bool	SetLocalNodeNo(int a_nNodeNo);
	bool	SetNaProcNo(int a_nProcNo);
	bool	SetAtomAddress(void);

	int		Lookup(const char* a_szPkgName, const char* a_szNodeType,
					const char* a_szProcName, std::list<ST_ADDR>& a_lstAddr);
	int		LookupAtom(const char* a_szProcName, int& a_nNodeNo, int& a_nProcNo);
	int		LookupAgentApp(const char* a_szProcName, std::list<ST_ADDR>& a_lstAddr);

	bool	AddApp(const char* a_szPkgName, const char* a_szNodeType, 
									const char* a_szProcName, int a_nProcNo);

	void	PrintTable(CFileLog* a_pclsLog, int a_nLogLevel);

private:
	bool	Regist(const char* a_szPkgName, const char* a_szNodeType, 
						const char* a_szProcName, int a_nNodeNo, int a_nProcNo, 
														bool a_bIsApp = false);
	bool	RegistLocal(const char* a_szProcName, int a_nProcNo, bool a_bIsApp = false);

private:
	std::map<std::string, ST_ADDR>	m_mapAddr;		// key: process name
	pthread_mutex_t	m_tMutex;

	std::string		m_strLocalPkgName;
	std::string		m_strLocalNodeType;
	int				m_nLocalNodeNo;
	int				m_nNaProcNo;
	
	static CAddress* m_pclsInstance;
	CAddress();
	CAddress(CAddress&) {}
	~CAddress();
};

#endif //CADDRESS_HPP_

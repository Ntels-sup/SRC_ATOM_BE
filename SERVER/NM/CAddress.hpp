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

	void	SetDefaultPkg(const char* a_strPkgName);
	void	SetDefaultNode(const char* a_strNodeType, int a_nNodeNo);

	int		Lookup(const char* a_szPkgName, const char* a_szNodeType,
					const char* a_szProcName, std::list<ST_ADDR>& a_lstAddr);
	int		LookupAtom(const char* a_szProcName, int& a_nNodeNo, int& a_nProcNo);
	int		LookupAgentApp(const char* a_szProcName, std::list<ST_ADDR>& a_lstAddr);

	bool	AddAtom(const char* a_strPkgName, const char* a_szNodeType, 
							const char* a_strProcName, int a_nProcNo);
	bool	AddApp(const char* a_strPkgName, const char* a_szNodeType,
							const char* a_strProcName, int a_nProcNo);							
	bool	Remove(const char* a_szPkgName, const char* a_szNodeType, 
							const char* a_szProcName, int a_nProcNo);

private:
	std::multimap<std::string, ST_ADDR>	m_mapAddr;		// key: process name
	pthread_mutex_t	m_tMutex;

	std::string		m_strDefPkgName;
	std::string		m_strDefNodeTYpe;
	int				m_nDefNodeNo;
	
	static CAddress* m_pclsInstance;
	CAddress();
	CAddress(CAddress&) {}
	~CAddress();
};

#endif //CADDRESS_HPP_

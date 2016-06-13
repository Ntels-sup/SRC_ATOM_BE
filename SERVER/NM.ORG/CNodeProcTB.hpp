/* vim:ts=4:sw=4
 */

/**
 * \file	CNodeProcTB.hpp
 * \brief	
 *
 * $Author: junls&ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CNODEPROCTB_HPP_
#define CNODEPROCTB_HPP_

#include <map>
#include <string>

class CNodeProcTB
{
public:
	struct ST_INFO {
		std::string	m_strPkgName;
		int			m_nNodeProcNo;
		std::string m_strNodeProcName;
		std::string	m_strNodeVersion;
		std::string	m_strNodeType;
		std::string	m_strIp;
	};
	
public:
	static CNodeProcTB& Instance(void);

	bool	AddNode(int a_nId, ST_INFO& a_stNode);
	int		GetNode(int a_nId, ST_INFO& a_stNode);

	bool	AddAtomProc(int	a_nId, ST_INFO& a_stNode);
	int		GetAtomProc(int a_nId, ST_INFO& a_stNode);
	int		GetAtomProcNo(const char* a_szProcName);

	void	Delete(int a_nId);

private:
	CNodeProcTB();
	~CNodeProcTB() {}
	CNodeProcTB(const CNodeProcTB&) {}
	//CNodeProcTB& operator=(const CNodeProcTB&) {}

	std::map<int, ST_INFO>	m_mapNode;		// key: id (socket)
	std::map<int, ST_INFO>	m_mapAtom;		// key: id (socket)
	pthread_mutex_t			m_tMutex;

	static CNodeProcTB*  m_pclsInstance;          // singleton instance
};

#endif // NODEPROCTB_HPP_

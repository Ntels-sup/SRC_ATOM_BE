/* vim:ts=4:sw=4
 */

/**
 * \file	CDBInOut.hpp
 * \brief	
 *
 * $Author: junls&ntels.com $
 * $Date: $
 * $Id: $
 */

#include <string>

#include "MariaDB.hpp"


class CDBInOut
{
public:
	int			m_nNodeNo;
	std::string	m_strNodeName;
	int			m_nProcNo;
	
public:
	CDBInOut();
	~CDBInOut();
	
	int		FindNodeNo(std::string& a_strUuid);
	int		FindNodeNo(std::string& a_strPkgName, std::string& a_strNodeType);
	int		FindProcNo(std::string& a_strPkgName, std::string& a_strNodeType,
												std::string& a_strProcName);
	int		NodeCreate(std::string& a_strPkgName, std::string& a_strNodeType,
						std::string& a_strUuid, std::string& a_strIp);

private:
	bool	DBOpen();
	void	DBClose();
	
	DB*		m_pclsDB;
};

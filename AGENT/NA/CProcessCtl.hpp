/* vim:ts=4:sw=4
 */
/**
 * \file	CProcessCtl.hpp
 * \brief	APP 프로세스 관리
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
clss CProcessCtl
{
public:
	CProcessCtl(char* argc, char *argv[]);
	Init(
	
	
	int GetNodeNod(void) { return atoi(getenv("ATOM_NODENO")); }
	int GetProcNo(void); { return atoi(getenv("ATOM_PROCNO")); }

private:


};
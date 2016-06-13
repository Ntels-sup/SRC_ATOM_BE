
#ifndef __C_ATOM_CTL_CMD_HPP__
#define __C_ATOM_CTL_CMD_HPP__

#include "CAtomCtlCmdInterface.hpp"
#include "CProtocol.hpp"

#include <vector>

class CAtomCtlCmd
{
public:
    CAtomCtlCmd();
    ~CAtomCtlCmd();

    void    Register(CAtomCtlCmdInterface * a_pClsInterface);
    bool    Do(CProtocol * a_pCls, CModuleIPC * _ipc);
    bool    Do(CProtocol * a_pCls, CMesgExchSocket * _sock);


private:
    bool    work(CProtocol * a_pCls);
    void    responseMsg(CProtocol *  a_pCls,
                        bool         a_bResult,
                        const char * a_text);


private:
    std::vector<CAtomCtlCmdInterface *>     m_vecCls;

    std::vector<char>                       m_vec;
    std::string                             m_str;

};


#endif // __C_ATOM_CTL_CMD_HPP__

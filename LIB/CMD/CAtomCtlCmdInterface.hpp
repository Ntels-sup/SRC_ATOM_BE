#ifndef __CATOM_CTL_CMD_INTERFACE_HPP__
#define __CATOM_CTL_CMD_INTERFACE_HPP__

#include "CProtocol.hpp"

class CAtomCtlCmdInterface
{
public:
    CCmdInterface();
    virtual ~CCmdInterface() = 0;


    virtual bool    InitCmd(CProtocol& a_clsRecv);
    virtual bool    ReConfigCmd(CProtocol& a_clsRecv);
    virtual bool    StopCmd(CProtocol& a_clsRecv);
    virtual bool    LogLvCmd(CProtocol& a_clsRecv);

};

#endif // __CMD_ATOM_CTL_CMD_INTERFACE_HPP__

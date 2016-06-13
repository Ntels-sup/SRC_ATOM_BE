
#ifndef __NODE_INTERFACE_HPP__
#define __NODE_INTERFACE_HPP__

#include "CProtocol.hpp"
#include "CModuleIPC.hpp"
#include "CAddress.hpp"

#include "NodeData.hpp"
#include "CommonCode.hpp"

#include <vector>

class NodeInterface
{
public:
   explicit  NodeInterface();
    ~NodeInterface();

    bool    Init(ST_Profile & _profile,
                 CModuleIPC * _ipc,
                 CAddress   * _addr);

    NodeData &  GetData();
    CProtocol & GetRecvProtocol() { return m_rProtocol; }

    bool        SendRequest(int             a_nCommandCode,
                            std::string   & a_strB);
    bool        SendNotify(int              a_nCommandCode,
                           std::string    & a_strB);
    bool        Send(NodeData & a_sNodeData);

private:
    int         generateSequenceId();
    void        setHeader(int a_nCommandCode);

private:
    unsigned short                       m_seq;

    NodeData                             m_nodeData;

    CProtocol                            m_rProtocol;
    CProtocol                            m_sProtocol;
    std::vector<char>                    m_vec;

    CModuleIPC *                         m_pIpc;
    CAddress   *                         m_pAddr;

    ST_AtomAddr                          m_node;

    ST_AtomAddr                          m_myProc;
    ST_AtomAddr                          m_praProc;
    ST_AtomAddr                          m_naProc;
    // ST_AtomAddr                          m_rsaProc;
};


#endif // __NODE_INTERFACE_HPP__

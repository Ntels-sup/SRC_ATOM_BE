
#ifndef NODE_DATA_HPP
#define NODE_DATA_HPP

#include "ExaCommonDef.hpp"

class NodeData
{
public:
    NodeData();
    ~NodeData();

    void    Clear();
    bool    IsFill() { return m_nCommandCode != 0; }

    int     GetCommand() { return m_nCommandCode; }
    void    SetCommand(int a_nCommandCode) { m_nCommandCode = a_nCommandCode; }
    void    SetError() { m_isError = true; }

    std::string & GetDstYn();
    std::string & GetBody() { return m_B; }
    bool    IsError() { return m_isError; }

private:
    int             m_nCommandCode;
    bool            m_isError;

    std::string     m_B;
};


#endif // NODE_DATA_HPP

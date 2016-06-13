
#include "NodeData.hpp"

extern CFileLog *   gLog;

NodeData::
NodeData()
{
    m_B.resize(512, '\0');
}

NodeData::
~NodeData()
{
    // Empty
}

void NodeData::
Clear()
{
    m_nCommandCode  = 0;
    m_isError       = false;

    m_B.clear();
}


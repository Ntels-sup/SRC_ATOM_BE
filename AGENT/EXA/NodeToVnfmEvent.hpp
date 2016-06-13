
#ifndef __NODE_TO_VNFM_EVENT_HPP__
#define __NODE_TO_VNFM_EVENT_HPP__

#include "NodeData.hpp"
#include "HttpData.hpp"
#include "DashBoard.hpp"
#include "TimeOutCheck.hpp"

#include "ContentsProvider.hpp"

class NodeToVnfmEvent
{
public:
    NodeToVnfmEvent();
    ~NodeToVnfmEvent();

    bool        Init(ContentsProvider * a_pResourceAndTpsProvider);
    HttpData &  Receive(NodeData & a_rNodeData, DashBoard & a_board);
    HttpData &  ReportPerf(DashBoard & a_board);
    HttpData &  MakeKeepAliveForEvent(DashBoard & a_board);

private:
    bool        ready(NodeData & a_rNodeData,       DashBoard & a_board);
    bool        started(NodeData & a_rNodeData,     DashBoard & a_board);
    bool        stopped(NodeData & a_rNodeData,     DashBoard & a_board);
    bool        subscriber(NodeData & a_rNodeData,  DashBoard & a_board);
    bool        event(NodeData & a_rNodeData,       DashBoard & a_board);
    bool        perf(NodeData & a_rNodeData,        DashBoard & a_board);
    bool        keepAlive(NodeData & a_rNodeData,   DashBoard & a_board);
    bool        unknown(NodeData & a_rNodeData,     DashBoard & a_board);

    bool        perfTps(NodeData & a_rNodeData,     DashBoard & a_board);
    bool        isValidReportPerf(std::string & a_time);


private:

    TimeOutCheck        m_timerForReportTPS;
    TimeOutCheck        m_timerForReportCPU;
    TimeOutCheck        m_timerForReportMEM;
    TimeOutCheck        m_timerForReportDISK;
    TimeOutCheck        m_timerForReportNET;

    ContentsProvider * m_pResourceAndTpsProvider;

    HttpData           m_sHttpData;
};

#endif // __NODE_TO_VNFM_EVENT_HPP__

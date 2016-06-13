#include "CResTableData.hpp"

CResTableData::CResTableData(std::string &a_strColumnOrder)
{
    m_prcDate = 0;
    m_nNodeNo = 0;
    m_dStatData = 0;

    m_strColumnOrder = a_strColumnOrder;
}

CResTableData::~CResTableData()
{
}

int CResTableData::Init(time_t a_prcDate, int a_nNodeNo, double a_dStatData)
{
    m_prcDate = a_prcDate;
    m_nNodeNo = a_nNodeNo;
    m_dStatData = a_dStatData;

    return CStmResApi::RESAPI_RESULT_OK;
}

int CResTableData::GetEncodeMessage(rabbit::object &a_cRecord)
{
    std::string strData;
    std::list<std::string>::iterator priIter;
    std::list<int>::iterator valueIter;

    try{
        a_cRecord["prc_date"] = m_prcDate;
        a_cRecord["node_no"] = m_nNodeNo;
        a_cRecord["column_order_ccd"] = m_strColumnOrder;
        a_cRecord["stat_data"] = m_dStatData;
    } catch(rabbit::type_mismatch   e) {
        m_strErrString.append(e.what());
        return CStmResApi::RESAPI_RESULT_PARSING_ERROR;
    } catch(rabbit::parse_error e) {
        m_strErrString.append(e.what());
        return CStmResApi::RESAPI_RESULT_PARSING_ERROR;
    } catch(...) {
        m_strErrString.append("Unknown Error");
        return CStmResApi::RESAPI_RESULT_PARSING_ERROR;
    }

    return CStmResApi::RESAPI_RESULT_OK;
}

int CResTableData::GetDecodeMessage(rabbit::object &a_cRecord)
{
    std::string strData;
    std::list<std::string>::iterator priIter;
    std::list<int>::iterator valueIter;

    try{
        m_prcDate = a_cRecord["prc_date"].as_uint64();
        m_nNodeNo = a_cRecord["node_no"].as_uint();
        m_dStatData = a_cRecord["stat_data"].as_double();
    } catch(rabbit::type_mismatch   e) {
        m_strErrString.append(e.what());
        return CStmResApi::RESAPI_RESULT_PARSING_ERROR;
    } catch(rabbit::parse_error e) {
        m_strErrString.append(e.what());
        return CStmResApi::RESAPI_RESULT_PARSING_ERROR;
    } catch(...) {
        m_strErrString.append("Unknown Error");
        return CStmResApi::RESAPI_RESULT_PARSING_ERROR;
    }

    return CStmResApi::RESAPI_RESULT_OK;
}


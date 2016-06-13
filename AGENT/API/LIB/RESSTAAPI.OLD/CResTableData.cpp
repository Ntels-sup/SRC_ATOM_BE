#include "CResTableData.hpp"

CResTableData::CResTableData(std::string &a_strRscGrpId)
{
		m_prcDate = 0;
		m_nNodeNo = 0;
		m_dStatData = 0;

		m_strRscGrpId = a_strRscGrpId;
}

CResTableData::~CResTableData()
{
}

int CResTableData::Init(time_t a_prcDate, int a_nNodeNo, std::string &a_strColumnOrderCCD, double a_dStatData)
{
		m_prcDate = a_prcDate;
		m_nNodeNo = a_nNodeNo;
		m_strColumnOrderCCD = a_strColumnOrderCCD;
		m_dStatData = a_dStatData;

		return CStaResApi::RESAPI_RESULT_OK;
}

int CResTableData::Init(time_t a_prcDate, int a_nNodeNo, char *a_strColumnOrderCCD, double a_dStatData)
{
		m_prcDate = a_prcDate;
		m_nNodeNo = a_nNodeNo;
		m_strColumnOrderCCD = a_strColumnOrderCCD;
		m_dStatData = a_dStatData;

		return CStaResApi::RESAPI_RESULT_OK;
}

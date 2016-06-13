#ifndef __CRESTABLEDATA_HPP__
#define __CRESTABLEDATA_HPP__

#include <list>
#include <time.h>
#include <string>

#include "CStaResApi.hpp"

class CResTableData{
		private:
				std::string m_strRscGrpId;
				time_t m_prcDate;
				int m_nNodeNo;
				std::string m_strColumnOrderCCD;
				double m_dStatData;

		public:
				CResTableData(std::string &a_strRscGrpId); 
				~CResTableData(); 
				void InsertRscGrpId(char *a_szRscGrpId) { m_strRscGrpId = a_szRscGrpId; };
				void InsertRscGrpId(std::string a_strRscGrpId) { m_strRscGrpId = a_strRscGrpId; };
				int Init(time_t a_prcDate, int a_nNodeNo, std::string &a_strColumnOrderCCD, double a_dStatData);
				int Init(time_t a_prcDate, int a_nNodeNo, char *a_strColumnOrderCCD, double a_dStatData);
				std::string GetRscGrpId() { return m_strRscGrpId; };
				int GetNodeNo() { return m_nNodeNo; };
				time_t GetPrcDate() { return m_prcDate; };
				std::string GetColumnOrderCCD() { return m_strColumnOrderCCD; };
				double GetStatData() { return m_dStatData; };
};

#endif

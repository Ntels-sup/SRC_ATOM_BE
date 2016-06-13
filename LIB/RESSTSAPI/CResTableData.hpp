#ifndef __CRESTABLEDATA_HPP__
#define __CRESTABLEDATA_HPP__

#include <list>
#include <time.h>
#include <string>

#include "CStmResApi.hpp"

class CResTableData : public CStmResApi{
		private:
				//std::string m_strRscId;
				time_t m_prcDate;
				int m_nNodeNo;
				std::string m_strColumnOrder;
				double m_dStatData;

		public:
				CResTableData(std::string &a_strColumnOrder); 
				~CResTableData(); 
				int Init(time_t a_prcDate, int a_nNodeNo, double a_dStatData);
				std::string GetColumnOrder() { return m_strColumnOrder; };
				int GetNodeNo() { return m_nNodeNo; };
				time_t GetPrcDate() { return m_prcDate; };
				double GetStatData() { return m_dStatData; };
				int GetEncodeMessage(rabbit::object &a_cRecord);
				int GetDecodeMessage(rabbit::object &a_cRecord);
};

#endif

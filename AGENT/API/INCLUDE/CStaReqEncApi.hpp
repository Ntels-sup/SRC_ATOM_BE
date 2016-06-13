#ifndef __CSTA_REQENCAPI_HPP__
#define __CSTA_REQENCAPI_HPP__

#include <time.h>

class CStaReqEncApi{
		private:
				time_t m_time;
		public:

				CStaReqEncApi();
				~CStaReqEncApi();
				void SetTime(time_t a_time) { m_time = a_time; };
				time_t GetTime() { return m_time; };
				int EncodeMessage(char *a_chData, int a_nMaxData, int *a_nDataLen);
};

#endif

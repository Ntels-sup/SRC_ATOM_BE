#include <list>
#include <string>

using namespace std;

class CStaReqDecApi{
		private:
				time_t m_nTime;

		public:
				CStaReqDecApi() {};
				~CStaReqDecApi() {};
				time_t GetTime();
				int DecRequestMessage(char *a_chData, int a_nDataLen);
};

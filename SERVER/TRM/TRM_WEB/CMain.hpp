#ifndef __CMAIN_HPP__
#define __CMAIN_HPP__
#include <iostream>
#include <list>
#include <cstring>
#include "TRM_Define.hpp"
#include "CScheduler.hpp"
#include "CNMSession.hpp"

using namespace std;

class CMain{
        private:
				CScheduler *m_cscheduler;
				CNMSession *m_cnmsession;

        public:
                CMain();
                ~CMain();

				int init();
				int Final();
                int Run();
};

#endif


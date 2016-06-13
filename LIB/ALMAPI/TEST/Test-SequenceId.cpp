
// #include <stdio.h>

#include <iostream>
#include "SequenceId.hpp"
#include "CFileLog.hpp"

CFileLog *   gAlmLog = NULL;

void initLog()
{
	if(gAlmLog == NULL)
    {
        int ret = 0;
        gAlmLog    = new (std::nothrow) CFileLog(&ret);

        if(ret < 0)
        {
            delete gAlmLog;
            exit(0);
        }
    }

    std::string 	path(getenv("HOME"));
    path += "/LOG";
    gAlmLog->Initialize((char *)path.c_str(), "", "TEST-SEQUENCE-ID", -1, LV_DEBUG);
}

int main()
{

	initLog();

	std::string 	path = "/UDBS/ATOM/DATA/ALM";
	std::string 	file = "test.last";

	SequenceId	sid;

	std::cout << "TEST 1 - NORMAL" << std::endl;
	sid.Init(path.c_str(), file.c_str());
	std::cout << sid.Get() << std::endl;
	std::cout << sid.Generate() << std::endl;
	std::cout << sid.Get() << std::endl;

	std::cout << "TEST 2" << std::endl;
	std::string 	abs_file = path + "/" + file;
	remove(abs_file.c_str());

	std::cout << sid.Get() << std::endl;
	std::cout << sid.Generate() << std::endl;
	std::cout << sid.Get() << std::endl;

	std::cout << "TEST 3" << std::endl;
	remove(abs_file.c_str());
	sid.Init(path.c_str(), file.c_str());
	std::cout << sid.Get() << std::endl;
	std::cout << sid.Generate() << std::endl;
	std::cout << sid.Get() << std::endl;

	std::cout << "TEST 4" << std::endl;
	sid.Set(4294967294);
	std::cout << sid.Get() << std::endl;
	std::cout << sid.Generate() << std::endl;
	std::cout << sid.Get() << std::endl;
	std::cout << sid.Generate() << std::endl;
	std::cout << sid.Get() << std::endl;
	std::cout << sid.Generate() << std::endl;
	std::cout << sid.Get() << std::endl;

	sid.Init(path.c_str(), file.c_str());
	std::cout << sid.Get() << std::endl;

	return 0;
}

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

#include <iostream>

using namespace rapidjson;
using std::cout;
using std::endl;
using std::string;

void write()
{
    rabbit::object docRoot;
    rabbit::object docBody = docRoot["BODY"];
    docBody["worst_status"] = "STOPPED";

   	rabbit::object o[3];
    rabbit::array docProcess = docBody["process_status"];
    for (int i = 0; i < 3; i++) {
        o[i]["procno"]     = 1;
        o[i]["procname"]   = "procname";
        o[i]["status"]     = "status";
        o[i]["startdate"]  = "20313-232-2323";
        o[i]["stopdate"]   = "20321-2323-232";
        o[i]["version"]    = "1.0.0.0";

        docProcess.push_back(o[i]);
    }

    cout << docRoot.str() << endl;
}

int main()
{
	write();

	return 0;
}

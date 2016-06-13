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
	cout << endl;
	cout << "----- WRITE" << endl;

	rabbit::object 	    root;
	rabbit::object      performance = root["performance"];
	performance["server-uuid"]  = "ABC";

	rabbit::array       performance_metrics = performance["performance_metrics"];

	/*--
		rabbit::object 	tps;

		tps["id"]           = "tps.";
	    tps["timestamp"]    = "TIME";
		tps["data_type"]    = "int";
		tps["unit"]         = "tps";
		tps["calculated"]   = "cur";
		tps["metric_type"]  = "Data Traffic";

		performance_metrics.push_back(tps);	
	--*/

	rabbit::object tps[3];
	for(int nLoop=0; nLoop < 3; nLoop++)
	{
		tps[nLoop]["id"]           = "tps." + nLoop;
	    tps[nLoop]["timestamp"]    = "TIME";
		tps[nLoop]["data_type"]    = "int";
		tps[nLoop]["unit"]         = "tps";
		tps[nLoop]["calculated"]   = "cur";
		tps[nLoop]["metric_type"]  = "Data Traffic";

		performance_metrics.push_back(tps[nLoop]);	
	}

	// std::string     body = root.str();
	// cout << body << endl;

	cout << root.str() << endl;
}

int main()
{
	write();

	return 0;
}

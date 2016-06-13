#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

#include <iostream>

using namespace rapidjson;
using std::cout;
using std::endl;
using std::string;

rabbit::object root;
rabbit::object stmReq = root["STMREQ"];

string strTest;
char test[8192];
int testLen = 0;

void write()
{

//rabbit::object root;

//rabbit::object stmReq = root["STMREQ"];

stmReq["SESSIONID"] = "1";
stmReq["TYPE"] = "RCD";

rabbit::object priKey = stmReq["PRIMARY KEY"];

priKey["PEER"] = "S-CSCF";

//rabbit::object value = stmReq["VALUE"];

//value["RCV_CNT"] = "10";
//value["SND_CNT"] = "10";
//
rabbit::object first;
first["COLUMN"] = "RCV_CNT";
first["VALUE"] = "10";
rabbit::object second;
second["COLUMN"] = "SND_CNT";
second["VALUE"] = "20";

//rabbit::array a = value["ARRAY"];
rabbit::array a = stmReq["VALUE"];

a.push_back(first);
a.push_back(second);



cout << stmReq.str() << endl;

strTest.append(stmReq.str());

}

void read()
{
    try {
		rabbit::document doc;
		//doc.parse(stmReq.str());
		doc.parse(strTest);

		cout << "SESSONID=" << doc["SESSIONID"].as_string() << endl;

		rabbit::array a = doc["VALUE"];
		cout << "array size " << a.size() << endl;

	rabbit::value v;
	for(auto nLoop=0u; nLoop < a.size();++nLoop){
			v = a.at(nLoop);

			string value;
			value = v["COLUMN"].as_string();
			cout << "COLUMN=" << value << endl;
			value = v["VALUE"].as_string();
			cout << "VALUE=" << value << endl;

	}
    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
    } catch(...) {
        cout << "Unknown Error" << endl;
    }
		printf("TEST\n");
}

int main()
{
write();

read();

return 0;
}

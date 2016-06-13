#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

#include <iostream>

using namespace rapidjson;
using std::cout;
using std::endl;
using std::string;

const char * text = ""
"{\"menu\": { \n"
"  \"id\": \"file\", \n"
"  \"value\": \"File\", \n"
"  \"INT\": 3, \n"
"  \"popup\": { \n"
"  \"menuitem\": [ \n"
"     {\"value\": \"New\", \"onclick\": \"CreateNewDoc()\"}, \n"
"     {\"value\": \"Open\", \"onclick\": \"OpenDoc()\"}, \n"
"     {\"value\": \"Close\", \"onclick\": \"CloseDoc()\"} \n"
"   ]\n"
" }\n"
"}}\n";

/*---
{"menu": {
  "id": "file",
  "value": "File",
  "popup": {
    "menuitem": [
      {"value": "New", "onclick": "CreateNewDoc()"},
      {"value": "Open", "onclick": "OpenDoc()"},
      {"value": "Close", "onclick": "CloseDoc()"}
    ]
  }
}}
----*/

void read()
{
	cout << "----- READ" << endl;
	cout << text << endl;
	
	try {

		rabbit::document 	doc;
		doc.parse(text);


		cout << endl;	
		cout << "---- STRING" << endl;

		string value = string(doc["menu"]["value"].as_string());
		cout << "[menu][value]    :" << "[" << value << "]" << endl;

		cout << endl;	
		cout << "---- INT" << endl;

		int INT = doc["menu"]["INT"].as_int();
		cout << "[menu][INT]    :" << "[" << INT << "]" << endl;

		cout << endl;	
		cout << "---- OBJECT" << endl;

		rabbit::object o = doc["menu"]["popup"];
		cout << "IS OBJECT        :" << o.is_object() << endl;
		cout << "HAS [menumitem]  :"      << o.has("menuitem") << endl;
		cout << "HAS [abc]        :"      << o.has("abc") << endl;


		cout << endl;	
		cout << "---- ARRAY" << endl;

		rabbit::array a = doc["menu"]["popup"]["menuitem"];
		cout << "menuitem SIZE    :" << a.size() << endl;
		cout << "menuitem IS EMPTY:" << a.empty() << endl;


		rabbit::value v;
		for(auto nLoop=0u; nLoop < a.size(); ++nLoop) {	
			v = a.at(nLoop);
			value = v["value"].as_string();
			cout << "value            :" << "[" << value << "]" << endl;
			value = v["onclick"].as_string();
			cout << "onclick          :" << "[" << value << "]" << endl;
		}

	} catch(rabbit::type_mismatch   e) {
		cout << e.what() << endl;
	} catch(rabbit::parse_error e) {
		cout << e.what() << endl;
	} catch(...) {
		cout << "Unknown Error" << endl;
	}
}

void write()
{
	cout << endl;
	cout << "----- WRITE" << endl;
	cout << text << endl;

	rabbit::object 	root;

	char temp[32];
	sprintf(temp, "file");

	rabbit::object  menu = root["menu"];
	menu["id"] 		= temp;
	menu["value"] 	= "File";
	
	rabbit::object popup = menu["popup"];
	rabbit::object menuitem = popup["menuitem"];

	rabbit::object first;

	first["value"] = "New";
	first["onclick"]= "CreateNewDoc()";

	rabbit::object second;
	second["value"] = "Open";
	second["onclick"]= "OpenDoc()";

	rabbit::object third;
	third["value"] = "Close";
	third["onclick"]= "CloseDoc()";

	rabbit::array 	a = popup[""];

	a.push_back(first);
	a.push_back(second);
	a.push_back(third);

	cout << root.str() << endl;
}

void J1()
{
	rabbit::object  root;

	rabbit::object  body = root["BODY"];
	body["nodeid"] = 1;
	body["procid"] = 2;
	body["auth"] = true;

	cout << root.str() << endl;

	return;
}

#include <vector>
#include <string>

struct stA
{
	std::string s;
	int 		n;
};

void C1()
{
	std::vector<stA> 	vec;

	stA 	arr[3];
	arr[0].s 	= "First";
	arr[0].n 	= 0;
	vec.push_back(arr[0]);

	arr[1].s 	= "Second";
	arr[1].n 	= 1;
	vec.push_back(arr[1]);

	arr[2].s 	= "Third";
	arr[2].n 	= 2;
	vec.push_back(arr[2]);


	rabbit::object  root;
    rabbit::object  body = root["BODY"];

    rabbit::array   metrix = body["metrix"];	
	
	for(auto iter=vec.begin(); iter != vec.end(); ++iter)
	{
		rabbit::object 	item;

		item["key"] = (*iter).s;
		item["value"] = (*iter).n;

		metrix.push_back(item);

		cout << "---------" << endl; 
	}

	cout << root.str() << endl;

	return ;
}		

int main()
{
	read();
	write();
	J1();
	C1();

	return 0;
}

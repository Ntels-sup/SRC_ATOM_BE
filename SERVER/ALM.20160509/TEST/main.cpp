#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>

/*--
struct ABC {

	int a;
	int b;
};
--*/

int main()
{

/*--
	struct ABC 	abc;

	std::cout << abc.a << std::endl;
--*/
	char temp[128];

	int len = sprintf(temp, "ABC %d", 123);

	printf("### [%d]\n", len);

	// sprintf(temp, "4294967295");
	sprintf(temp, "00000004294967295");

	unsigned int n = atoi(temp); 

	sprintf(temp, "0.0000004294967295");
	sprintf(temp, "%.10s", temp);
	std::cout << "*" << temp << std::endl;
	

	#define XYZ 10
	std::cout << n << std::endl;

	sprintf(temp, "%0d\n", "123");
	std::cout << temp << std::endl;

	std::string val(std::to_string((unsigned long long)n));

	std::string str(16, '0');
	str.replace(str.size() - val.size(), val.size(), val); 

	std::cout << str << std::endl;
	std::cout << str.size() << std::endl;
  
	sprintf(temp, "ABC");
	// std::cout << std::stoi(temp) << std::endl; 
	std::cout << atoi(temp) << std::endl; 


	return 0;
}

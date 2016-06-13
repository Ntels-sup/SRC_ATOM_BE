#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;

int StringSplit(const string& text, const char* separators, vector<string>& words)
{
	int n = text.length();
	int start, stop;

	start = text.find_first_not_of(separators);
	while ((start >= 0) && (start < n)) {
		stop = text.find_first_of(separators, start);
		if ((stop < 0) || (stop > n))
			stop = n;
		words.push_back(text.substr(start, stop - start));
		start = text.find_first_not_of(separators, stop+1);
	}

	return words.size();
}

#if UNIT_TEST
using std::cout;
using std::endl;
int main(int argc, char* argv[])
{
	string args = argv[1];
	std::cout << args << std::endl;

	vector<string> split;
	StringSplit(args, ",", split);
	for (int i=0; i < split.size(); i++) {
		cout << split[i] << endl;
	}
	
	return 0;
}
#endif

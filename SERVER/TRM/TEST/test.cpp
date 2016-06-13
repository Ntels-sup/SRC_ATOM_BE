#include <iostream>
#include <cstdio>
#include <cstring>

int main()
{
	char Filename[256];
	char NewFilename[256];
	
	strcpy(Filename, "Rename.txt");
	strcpy(NewFilename, "Tca.txt");
	if(rename(Filename, NewFilename) == -1)
	{
		perror("rename failure");
		return -1;
	}

	printf("change file name from %s to %s\n", Filename, NewFilename);

	return 0;

}

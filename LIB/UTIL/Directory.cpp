#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cstring>


bool CreatePath(const char* a_szPath, mode_t a_nMode)
{
	if (a_szPath == NULL) {
		return false;
	}

	char* szPath = strdup(a_szPath);
	char* p = szPath;

	while (*p != '\0') {
		p++;
		while (*p != '\0' && *p != '/') p++;

		char v = *p;
		*p = '\0';

		if (mkdir(szPath, a_nMode) == -1 && errno != EEXIST) {
			*p = v;
			free(szPath);
			return false;
		}

		*p = v;
	}
	free(szPath);

	return true;
}

bool DeletePath(const char* a_szPath)
{
	if (a_szPath == NULL) {
		return false;
	}
	DIR* pDir = opendir(a_szPath);
	if (pDir == NULL) {
		if (errno == ENOENT) {
			return true;
		}
		return false;
	}

	struct dirent *pEnt = NULL;
	struct stat stFileInfo;
	char szAbsFileName[FILENAME_MAX] = {0x00,};

	while (true) {
        pEnt = readdir(pDir);
        if (pEnt == NULL) {
            break;
        }
		snprintf(szAbsFileName, sizeof(szAbsFileName), "%s/%s", a_szPath, pEnt->d_name);
		if (lstat(szAbsFileName, &stFileInfo) < 0) {
			return false;
		}
		if (S_ISDIR(stFileInfo.st_mode)) {
			if(strcmp(pEnt->d_name, ".") && strcmp(pEnt->d_name, "..")) {
				DeletePath(szAbsFileName);
			}
		} else {
			remove(szAbsFileName);
		}
	}
	closedir(pDir);

	remove(a_szPath);

	return true;
}

#if 0
#include <cstdio>
int main(int argc, char* argv[])
{
	if (argc <= 1) {
		return 0;
	}
/*
	if (CreatePath(argv[1], 0766) == false) {
		fprintf(stderr, "create path failed\n");
		return 1;
	}
*/

	if (DeletePath(argv[1]) == false) {
		fprintf(stderr, "delete path failed\n");
		return 1;
	}

	return 0;
}
#endif

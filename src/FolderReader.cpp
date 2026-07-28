#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
	#ifdef _DEBUG
		#include <stdlib.h>
		#include <crtdbg.h>
	#endif
#else
	#include <sys/types.h>
	#include <dirent.h>
#endif

#include <string>
#include <assert.h>

#include "FolderReader.h"

#if defined(_WIN32) || defined(_WIN64)
FolderReader::FolderReader(char *name) {
	WIN32_FIND_DATAA data = {};
	HANDLE find = {};

	std::string pattern(name);
	pattern.append("/*");

	if ((find = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
		do {
			size_t len = strlen(data.cFileName);
			char* ptr = (char*)malloc((len + 1) * sizeof(char));
			assert(ptr != NULL);
			memset(ptr, 0, len + 1);
			strncpy(ptr, data.cFileName, len);
			this->listings.emplace_back(ptr);
		}
		while (FindNextFileA(find, &data) != 0);

		FindClose(find);
	}
}
#else
FolderReader::FolderReader(char *name) {
	DIR* dirp = opendir(name);
	struct dirent* dp = {};

	while ((dp = readdir(dirp)) != NULL) {
		size_t len = strlen(dp->d_name);
		char* ptr = (char*)malloc((len + 1) * sizeof(char));
		assert(ptr != NULL);
		memset(ptr, 0, len + 1);
		strncpy(ptr, dp->d_name, len);
		this->listings.emplace_back(ptr);
	}
	closedir(dirp);
}
#endif

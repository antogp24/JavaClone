#pragma once

#include <vector>

class FolderReader {
public:
	FolderReader(char* name);

	const std::vector<char*> &get_listings() {
		return this->listings;
	}

	~FolderReader() {
		for (char *str : listings) {
			if (str != NULL) {
				free(str);
				str = NULL;
			}
		}
	}

private:
	std::vector<char*> listings;
};


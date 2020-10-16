#pragma once
#include <string>

class FileString : public std::string
{
	public:
		FileString(const std::string& filename);
		std::string substr(long pos, long len);
		unsigned long length(void) const;
		long find_first_of(const char* ok,
				long startPosition = 0);
		long find_first_not_of(const char* notOk,
				long startPosition = 0);
		long find(const char* toFind, long startPosition = 0);
		char operator[](long offset);

	private:
		FILE* inFile;
		long filesize;
		long currentBlock;
		std::string blockStr;
};

class IOError
{
};


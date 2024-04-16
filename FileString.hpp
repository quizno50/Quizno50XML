#pragma once
#include <string>

class FileString;

struct StringRef
{
	StringRef();
	StringRef(FileString* fs, size_t s, size_t l);
	operator std::string() const;
	bool operator <(StringRef const& rhs) const;
	bool operator ==(StringRef const& rhs) const;

	FileString* _fs;
	size_t _start;
	size_t _len;
};

class FileString : public std::string
{
	public:
		FileString(const std::string& filename);
		std::string substr(size_t pos, size_t len);
		size_t length(void) const;
		size_t find_first_of(const char* ok,
				size_t startPosition = 0);
		size_t find_first_not_of(const char* notOk,
				size_t startPosition = 0);
		size_t find(const char* toFind, size_t startPosition = 0);
		size_t find(char toFind, size_t startPosition = 0);
		char operator[](size_t offset);

	private:
		FILE* inFile;
		size_t filesize;
		size_t currentBlock;
		std::string blockStr;
};

class IOError
{
};


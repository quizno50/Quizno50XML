#pragma once
#include <string>
#include <memory>
#include <mutex>

class FileString;
struct StringRef;

struct StringT
{
	StringT();
	virtual ~StringT();
	virtual operator std::string();
};

struct StringLit : public StringT
{
	StringLit();
	virtual ~StringLit();
	StringLit(std::string const& nv);
	StringLit(StringRef& ref);
	operator std::string();

	std::string _lit;
};

struct StringRef : public StringT
{
	StringRef();
	virtual ~StringRef();
	StringRef(FileString* fs, size_t s, size_t l);
	virtual operator std::string();
	bool operator <(StringRef const& rhs) const;
	bool operator ==(StringRef const& rhs) const;
	bool operator <(std::string const& rhs);
	bool operator ==(std::string const& rhs);

	FileString* _fs;
	size_t _start;
	size_t _len;
};

class FileString : public std::string
{
	public:
		FileString(const std::string& filename);
		std::shared_ptr<StringT> substr(size_t pos, size_t len);
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
		std::mutex _io_mtx;
};

class IOError
{
};


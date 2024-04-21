#include "FileString.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include <mutex>

StringT::StringT() { }
StringT::~StringT() { }
StringT::operator std::string() { 
	return "";
}

StringLit::StringLit() : _lit("") { }
StringLit::~StringLit() { }
StringLit::StringLit(std::string const& nv) : _lit(nv) { }
StringLit::StringLit(StringRef& ref) : _lit(static_cast<std::string>(ref)) { }
StringLit::operator std::string() {
	return _lit;
}

StringRef::StringRef() : _fs(nullptr), _start(0), _len(0) {}
StringRef::~StringRef() {}
StringRef::StringRef(FileString* fs, size_t s, size_t l) {
	_fs = fs;
	_start = s;
	_len = l;
}

StringRef::operator std::string() {
	if (_fs != nullptr) {
		return *_fs->substr(_start, _len);
	} else { 
		return "";
	}
}

bool StringRef::operator<(StringRef const& rhs) const {
	return _fs < rhs._fs and _start < rhs._start and _len < rhs._len;
}

bool StringRef::operator==(StringRef const& rhs) const {
	return _fs == rhs._fs and _start == rhs._start and _len == rhs._len;
}

bool StringRef::operator<(std::string const& rhs) {
	return static_cast<std::string>(*this) < rhs;
}

bool StringRef::operator==(std::string const& rhs) {
	return static_cast<std::string>(*this) == rhs;
}

FileString::FileString(const std::string& filename)
{
	struct stat s;

	currentBlock = -1;
	if (stat(filename.c_str(), &s) != 0)
	{
		throw IOError();
	}
	this->filesize = s.st_size;

	this->inFile = fopen(filename.c_str(), "r");
	if (this->inFile == nullptr)
	{
		throw IOError();
	}
}

std::shared_ptr<StringT> FileString::substr(size_t pos, size_t len)
{
	char* buffer = nullptr;
	size_t blkNum = pos / 8192;
	size_t nextBlock = (blkNum + 1) * 8192;
	std::string retStr;
	std::lock_guard<std::mutex> doingIo(_io_mtx);

	if (pos < 0 || len < 0)
	{
		throw IOError();
	}

	if (blkNum != currentBlock)
	{
		buffer = new char[8192];
		if (buffer == nullptr)
		{
			throw IOError();
		}
		currentBlock = blkNum;
		fseek(this->inFile, blkNum * 8192, SEEK_SET);
		if (fread(buffer, 8192, 1, this->inFile) == 0
				&& ferror(this->inFile))
		{
			throw IOError();
		}
		this->blockStr = std::string(buffer, 8192);
		delete[] buffer;
	}

	retStr = this->blockStr.substr(pos - blkNum * 8192, len);

	// Do we cross a block boundry?
	if (pos + len > nextBlock)
	{
		// Read in everything necessary after the block boundry.
		size_t readSize = pos + len - nextBlock;
		char* buffer = new char[readSize];
		fseek(this->inFile, nextBlock, SEEK_SET);
		if (fread(buffer, readSize, 1, this->inFile) == 0
				&& ferror(this->inFile))
		{
			throw IOError();
		}
		retStr += std::string(buffer, readSize);
		delete[] buffer;
	}

	return std::make_shared<StringLit>(retStr);
}

size_t FileString::length(void) const
{
	return filesize;
}

size_t FileString::find_first_of(const char* ok,
		size_t startPosition)
{
	size_t result = std::string::npos, pos = startPosition;
	while (result == std::string::npos && pos < filesize)
	{
		std::string buf = *this->substr(pos, 8192);
		result = buf.find_first_of(ok);
		if (result == std::string::npos)
		{
			pos += 8192;
		}
	}
	result += pos;
	return result;
}

size_t FileString::find_first_not_of(const char* notOk,
		size_t startPosition)
{
	size_t result = std::string::npos, pos = startPosition;
	while (result == std::string::npos && pos < filesize)
	{
		std::string buf = *this->substr(pos, 8192);
		result = buf.find_first_not_of(notOk);
		if (result == std::string::npos)
		{
			pos += 8192;
		}
	}
	result += pos;
	return result;
}

size_t FileString::find(const char* toFind,
		size_t startPosition)
{
	size_t result = std::string::npos, pos = startPosition;
	while (result == std::string::npos && pos < filesize)
	{
		std::string buf = *this->substr(pos, 8192);
		result = buf.find(toFind);
		if (result == std::string::npos)
		{
			pos += 8192;
		}
	}
	result += pos;
	return result;
}

size_t FileString::find(char toFind, size_t startPosition)
{
	size_t result = std::string::npos, pos = startPosition;
	while (result == std::string::npos && pos < filesize)
	{
		std::string buf = *this->substr(pos, 8192);
		result = buf.find(toFind);
		if (result == std::string::npos)
		{
			pos += 8192;
		}
	}
	result += pos;
	return result;
}


char FileString::operator[](size_t offset)
{
	return static_cast<std::string>(*this->substr(offset, 1))[0];
}


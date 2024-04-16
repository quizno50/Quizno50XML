#include "FileString.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

std::string FileString::substr(long pos, long len)
{
	char* buffer = nullptr;
	long blkNum = pos / 8192;
	long nextBlock = (blkNum + 1) * 8192;
	std::string retStr;

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
		long readSize = pos + len - nextBlock;
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

	return retStr;
}

unsigned long FileString::length(void) const
{
	return filesize;
}

long FileString::find_first_of(const char* ok,
		long startPosition)
{
	long result = std::string::npos, pos = startPosition;
	while (result == std::string::npos && pos < filesize)
	{
		std::string buf = this->substr(pos, 8192);
		result = buf.find_first_of(ok);
		if (result == std::string::npos)
		{
			pos += 8192;
		}
	}
	result += pos;
	return result;
}

long FileString::find_first_not_of(const char* notOk,
		long startPosition)
{
	long result = std::string::npos, pos = startPosition;
	while (result == std::string::npos && pos < filesize)
	{
		std::string buf = this->substr(pos, 8192);
		result = buf.find_first_not_of(notOk);
		if (result == std::string::npos)
		{
			pos += 8192;
		}
	}
	result += pos;
	return result;
}

long FileString::find(const char* toFind,
		long startPosition)
{
	long result = std::string::npos, pos = startPosition;
	while (result == std::string::npos && pos < filesize)
	{
		std::string buf = this->substr(pos, 8192);
		result = buf.find(toFind);
		if (result == std::string::npos)
		{
			pos += 8192;
		}
	}
	result += pos;
	return result;
}

long FileString::find(char toFind, long startPosition)
{
	long result = std::string::npos, pos = startPosition;
	while (result == std::string::npos && pos < filesize)
	{
		std::string buf = this->substr(pos, 8192);
		result = buf.find(toFind);
		if (result == std::string::npos)
		{
			pos += 8192;
		}
	}
	result += pos;
	return result;
}


char FileString::operator[](long offset)
{
	return this->substr(offset, 1)[0];
}


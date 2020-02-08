#pragma once
#include <vector>
#include <string>

class Attribute
{
	public:
		Attribute(long keyLocale, long keyLen, long valLocale, long valLen) :
				keyLocale(keyLocale), keyLen(keyLen), valLocale(valLocale),
				valLen(valLen)
		{
		}
	private:
		long keyLocale;
		long keyLen;
		long valLocale;
		long valLen;
};

class Tag
{
	public:
		Tag() : name(-1), nameLen(-1) {}
		std::vector<Tag> children;
		std::vector<Attribute> attributes;
		long name;
		long nameLen;
		enum TagType {
			TAG_META,
			TAG_COMMENT,
			TAG_TEXT,
			TAG_NORMAL } type;
};

class Document
{
	public:
		std::vector<Tag> tags;
};

class MetaTag : public Tag
{
};

class CommentTag : public Tag
{
};

class TextNode : public Tag
{
};

class ParseError
{
	public:
		ParseError(std::string message, long location) : message(message),
				location(location)
		{
		}
		const std::string& getMessage()
		{
			return this->message;
		}
		long getLocation()
		{
			return this->location;
		}

	private:
		std::string message;
		long location;
};


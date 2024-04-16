#pragma once
#include <vector>
#include <map>
#include <string>
#include "FileString.hpp"

class XMLError
{
	protected:
		std::string message;
};

class NavigationError : public XMLError
{
	public:
		NavigationError(std::string msg);
	protected:
};

typedef std::pair<StringRef, StringRef> Attribute;

class Tag
{
	public:
		Tag();
		std::vector<Tag> children;
		std::map<StringRef, StringRef> attributes;
		StringRef name;
		Tag& operator/(const std::string& subTag);
		operator std::string() const;
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

class ParseError : public XMLError
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

void parseDocument(FileString& fullCode, long& currentLocale,
		Document& d);
size_t countTagChildren(const Tag& t);
size_t countDocumentTags(const Document& d);


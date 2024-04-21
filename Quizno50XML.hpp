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

typedef std::pair<std::string, std::string> Attribute;

class Tag
{
	public:
		enum TagType {
			TAG_META,
			TAG_COMMENT,
			TAG_TEXT,
			TAG_NORMAL };

		Tag();
		virtual ~Tag();
		Tag(TagType t) : type(t) { }
		std::vector<Tag> children;
		std::map<std::string, std::string> attributes;
		std::shared_ptr<StringT> name;
		virtual Tag& operator/(const std::string& subTag);
		virtual operator std::string() const;
		TagType type;
};

class NormalTag : public Tag
{
	public:
		NormalTag() : Tag(TAG_NORMAL) { }
};

class MetaTag : public Tag
{
	public:
		MetaTag() : Tag(TAG_META) { }
};

class CommentTag : public Tag
{
	public:
		CommentTag() : Tag(TAG_COMMENT) { }
};

class TextTag : public Tag
{
	public:
		TextTag() : Tag(TAG_TEXT) { }
};

class Document
{
	public:
		std::vector<Tag> tags;
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

void parseDocument(FileString& fullCode, size_t& currentLocale,
		Document& d);
size_t countTagChildren(const Tag& t);
size_t countDocumentTags(const Document& d);


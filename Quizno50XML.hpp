#pragma once
#include <vector>
#include <string>

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

class Attribute
{
	public:
		Attribute(const std::string& key, const std::string& val) :
				key(key), val(val)
		{
		}
	private:
		std::string key;
		std::string val;
};

class Tag
{
	public:
		Tag();
		std::vector<Tag> children;
		std::vector<Attribute> attributes;
		std::string name;
		Tag& operator/(const std::string& subTag);
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


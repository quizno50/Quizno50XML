#include "Quizno50XML.hpp"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ALPHA_LOWER "abcdefghijklmnopqrstuvwxyz"
#define ALPHA_UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define NUMBERS "0123456789"

NavigationError::NavigationError(std::string msg)
{
	message = msg;
}

Tag::Tag() : type(TAG_NORMAL)
{
}

Tag::~Tag()
{
}

Tag& Tag::operator/(std::string const& subTag)
{
	for (auto& child : children)
	{
		if (static_cast<std::string>(*child.name) == subTag)
			return child;
	}
	throw NavigationError("Couldn't find child tag named \"" + subTag + "\"");
}

size_t eatWhiteSpace(FileString& fullCode,
		size_t& currentLocale)
{
	currentLocale = fullCode.find_first_not_of(" \t\n\r", currentLocale);
	if (currentLocale == std::string::npos)
	{
		currentLocale = fullCode.length();
	}
	return currentLocale;
}

void readIdentifier(FileString& fullCode, size_t& currentLocale,
		size_t& idStart, size_t& idLen)
{
	idStart = currentLocale;
	currentLocale = fullCode.find_first_not_of(ALPHA_UPPER ALPHA_LOWER
			NUMBERS "_-;:.~", currentLocale);
	idLen = currentLocale - idStart;
	if (idLen <= 0)
	{
		currentLocale = idStart;
		idStart = std::string::npos;
		idLen = std::string::npos;
	}
}

size_t readSingleCharacterToken(FileString& fullCode,
		size_t& currentLocale, char tokenChar)
{
	size_t originalLocale = currentLocale;

	if (fullCode[currentLocale] == tokenChar)
	{
		++currentLocale;
	}
	else
	{
		return std::string::npos;
	}

	return originalLocale;
}

size_t readOpenTag(FileString& fullCode, size_t& currentLocale)
{
	return readSingleCharacterToken(fullCode, currentLocale, '<');
}

size_t readAssignment(FileString& fullCode,
		size_t& currentLocale)
{
	return readSingleCharacterToken(fullCode, currentLocale, '=');
}

void readValue(FileString& fullCode, size_t& currentLocale,
		size_t& valueLocale, size_t& valueLen)
{
	size_t originalLocale = currentLocale;
	size_t endLocale = currentLocale;
	bool firstGo = true;
	char quote_type = '\0';

	if (fullCode[currentLocale] == '"' || fullCode[currentLocale] == '\'')
	{
		quote_type = fullCode[currentLocale];
		++currentLocale;
	}
	valueLocale = currentLocale;
	while ((endLocale > 0 && firstGo)
			|| static_cast<std::string>(*fullCode.substr(currentLocale - 1, 2)) ==
			std::string("\\\""))
	{
		endLocale = fullCode.find(quote_type, currentLocale);
		if (endLocale == std::string::npos)
		{
			currentLocale = originalLocale;
			valueLocale = std::string::npos;
			valueLen = std::string::npos;
			return;
		}
		currentLocale = endLocale + 1;
		firstGo = false;
	}
	valueLen = endLocale - valueLocale;
}

void parseAttribute(FileString& fullCode, size_t& currentLocale,
		size_t& attrKey, size_t& attrKeyLen,
		size_t& attrVal, size_t& attrValLen)
{
	attrKey = attrKeyLen = attrVal = attrValLen = std::string::npos;
	readIdentifier(fullCode, currentLocale, attrKey, attrKeyLen);
	if (attrKey == std::string::npos) return;
	eatWhiteSpace(fullCode, currentLocale);
	if (readAssignment(fullCode, currentLocale) != std::string::npos)
	{
		eatWhiteSpace(fullCode, currentLocale);
		readValue(fullCode, currentLocale, attrVal, attrValLen);
	}
}

void parseAttributes(FileString& fullCode,
		size_t& currentLocale, std::map<std::string, std::string>& attrs)
{
	size_t attrVal, attrValLen, attrName, attrNameLen;
	parseAttribute(fullCode, currentLocale, attrName, attrNameLen,
			attrVal, attrValLen);
	while (attrName != std::string::npos)
	{
		attrs.emplace(Attribute(*fullCode.substr(attrName, attrNameLen),
				attrVal == std::string::npos ? "" :
				static_cast<std::string>(*fullCode.substr(attrVal,
				attrValLen))));
		attrVal = attrValLen = attrName = attrNameLen = std::string::npos;
		eatWhiteSpace(fullCode, currentLocale);
		parseAttribute(fullCode, currentLocale, attrName, attrNameLen,
				attrVal, attrValLen);
	}
}

void readEndTag(FileString& fullCode, size_t& currentLocale,
		size_t& endTagLen)
{
	size_t originalLocale = currentLocale;
	eatWhiteSpace(fullCode, currentLocale);
	if (fullCode[currentLocale] == '/' && fullCode[currentLocale + 1] == '>')
	{
		currentLocale += 2;
		endTagLen = 2;
	}
	else if (fullCode[currentLocale] == '>')
	{
		currentLocale += 1;
		endTagLen = 1;
	}
	else
	{
		currentLocale = originalLocale;
		endTagLen = std::string::npos;
	}
}

void parseCloseTag(FileString& fullCode, size_t& currentLocale,
		size_t& tagName, size_t& tagNameLen)
{
	size_t originalLocale = currentLocale;
	size_t tag, tagLen;
	if (fullCode[currentLocale] != '<')
	{
		size_t errorLocale = currentLocale;
		currentLocale = originalLocale;
		throw ParseError("Expected closing tag.", errorLocale);
	}
	currentLocale += 1;
	eatWhiteSpace(fullCode, currentLocale);
	if (fullCode[currentLocale] != '/')
	{
		size_t errorLocale = currentLocale;
		currentLocale = originalLocale;
		throw ParseError("Expected closing tag.", errorLocale);
	}
	++currentLocale;
	readIdentifier(fullCode, currentLocale, tag, tagLen);
	if (tag == std::string::npos)
	{
		tagName = std::string::npos;
		tagNameLen = std::string::npos;
		return;
	}
	eatWhiteSpace(fullCode, currentLocale);
	if (fullCode[currentLocale] != '>')
	{
		size_t errorLocale = currentLocale;
		currentLocale = originalLocale;
		throw ParseError("Expected closing tag end.", errorLocale);
	}
	++currentLocale;
}

void parseText(FileString& fullCode, size_t& currentLocale,
		size_t& textStart, size_t& textLen)
{
	size_t end = fullCode.find("<", currentLocale);
	textStart = currentLocale;
	if (end < 0 || end == currentLocale)
	{
		textStart = std::string::npos;
		textLen = std::string::npos;
		return;
	}
	textLen = end - textStart;
	currentLocale += textLen;
}

void parseTag(FileString& fullCode, size_t& currentLocale,
		Tag& result, bool& valid);

void parseCommentTag(FileString& fullCode, size_t& currentLocale,
		Tag& commentTag, bool& valid)
{
	size_t originalLocale = currentLocale;
	size_t end;

	if (static_cast<std::string>(*fullCode.substr(currentLocale, 4)) != "<!--")
	{
		valid = false;
		return;
	}

	currentLocale += 4;

	end = fullCode.find("-->", currentLocale);

	if (end == std::string::npos)
	{
		size_t errorLocale = currentLocale;
		currentLocale = originalLocale;
		throw ParseError("Couldn't find comment end.", errorLocale);
	}

	commentTag.name = std::make_shared<StringRef>(&fullCode, currentLocale,
			end - currentLocale);
	commentTag.type = Tag::TAG_COMMENT;
	currentLocale = end + 3;
	valid = true;
}

void parseTagsAndText(FileString& fullCode,
		size_t& currentLocale, std::vector<Tag>& tags,
		size_t& itemsParsed)
{
	size_t textStart = std::string::npos, textEnd = std::string::npos;
	bool valid = false, keep_going = true;

	while (keep_going) {
		valid = false;
		Tag newTag;
		if (fullCode.length() == currentLocale)
		{
			keep_going = false;
			continue;
		}

		parseTag(fullCode, currentLocale, newTag, valid);
		if (!valid)
		{
			parseCommentTag(fullCode, currentLocale, newTag, valid);
			if (!valid)
			{
				parseText(fullCode, currentLocale, textStart, textEnd);
				if (textStart == std::string::npos)
				{
					// Can't parse anything... We're done here...
					keep_going = false;
					continue;
				}
				newTag.name = fullCode.substr(textStart, textEnd);
				newTag.type = Tag::TAG_TEXT;
			}
		}
		tags.push_back(newTag);
		++itemsParsed;
	}
}

void parseTag(FileString& fullCode, size_t& currentLocale, Tag& result,
		bool& valid)
{
	size_t originalLocale = currentLocale;
	size_t tagName, tagNameLen, endTagLen;
	size_t childrenParsed = 0;

	if (currentLocale == fullCode.length())
	{
		return;
	}
	if (readOpenTag(fullCode, currentLocale) == std::string::npos) return;
	eatWhiteSpace(fullCode, currentLocale);
	readIdentifier(fullCode, currentLocale, tagName, tagNameLen);
	if (tagName == std::string::npos)
	{
		currentLocale = originalLocale;
		valid = false;
		return;
	}
	eatWhiteSpace(fullCode, currentLocale);
	parseAttributes(fullCode, currentLocale, result.attributes);
	readEndTag(fullCode, currentLocale, endTagLen);
	if (endTagLen == std::string::npos)
	{
		currentLocale = originalLocale;
		valid = false;
		return;
	}

	if (endTagLen == 1) // tag has children...
	{
		parseTagsAndText(fullCode, currentLocale, result.children,
				childrenParsed);
		parseCloseTag(fullCode, currentLocale, tagName, tagNameLen);
		if (tagNameLen == std::string::npos)
		{
			currentLocale = originalLocale;
			valid = false;
			return;
		}
	}
	result.name = std::make_shared<StringRef>(&fullCode, tagName, tagNameLen);
	result.type = Tag::TAG_NORMAL;
	valid = true;
}

size_t readMetaChar(FileString& fullCode, size_t& currentLocale,
		char& metaChar)
{
	metaChar = '?';
	if (readSingleCharacterToken(fullCode, currentLocale,
			metaChar) == std::string::npos)
	{
		metaChar = '!';
		return readSingleCharacterToken(fullCode, currentLocale, metaChar);
	}
	return currentLocale;
}

void parseMetaTag(FileString& fullCode, size_t& currentLocale,
		Tag& metaTag, bool &valid)
{
	size_t originalLocale = currentLocale;
	size_t tagId, tagIdLen;
	char metaChar = -1;
	size_t endTagLen;

	if (currentLocale == fullCode.length())
	{
		valid = false;
		return;
	}
	readOpenTag(fullCode, currentLocale);
	readMetaChar(fullCode, currentLocale, metaChar);
	// Verify this isn't a comment tag.
	if (fullCode[currentLocale] == '-' && fullCode[currentLocale + 1] == '-')
	{
		// This is a comment tag, we need to bail.
		valid = false;
		currentLocale = originalLocale;
		return;
	}
	readIdentifier(fullCode, currentLocale, tagId, tagIdLen);
	metaTag.name = fullCode.substr(tagId, tagIdLen);
	metaTag.type = Tag::TAG_META;
	eatWhiteSpace(fullCode, currentLocale);
	parseAttributes(fullCode, currentLocale, metaTag.attributes);
	eatWhiteSpace(fullCode, currentLocale);
	if (metaChar == '?')
	{
		readMetaChar(fullCode, currentLocale, metaChar);
		if (metaChar != '?')
		{
			size_t errorLocale = currentLocale;
			currentLocale = originalLocale;
			throw ParseError("Expected '?' metachar.", errorLocale);
		}
	}
	readEndTag(fullCode, currentLocale, endTagLen);
	valid = true;
}

void parseTags(FileString& fullCode, size_t& currentLocale,
		std::vector<Tag>& allTags)
{
	Tag t;
	bool valid;
	
	if (currentLocale == fullCode.length())
	{
		return;
	}

	eatWhiteSpace(fullCode, currentLocale);
	parseTag(fullCode, currentLocale, t, valid);
	if (!valid)
	{
		parseMetaTag(fullCode, currentLocale, t, valid);
		if (!valid)
		{
			parseCommentTag(fullCode, currentLocale, t, valid);
			if (!valid)
			{
				return;
			}
		}
	}
	allTags.push_back(t);
	parseTags(fullCode, currentLocale, allTags);
}

Tag::operator std::string() const
{
	std::string beginString;
	std::string endString;
	std::string attrString;
	std::string childString;
	std::map<TagType, std::string> tagOpening { {TAG_META, "<?"},
			{TAG_COMMENT, "<!--"},
			{TAG_TEXT, ""},
			{TAG_NORMAL, "<"} };
	std::map<TagType, std::string> tagClosing { {TAG_META, "?>"},
			{TAG_COMMENT, "-->"},
			{TAG_NORMAL, ">"} };

	if (type == TAG_TEXT)
	{
		return static_cast<std::string>(*name);
	}
	else
	{
		if (type == TAG_NORMAL || type == TAG_META)
		{
			for (const std::pair<const std::string, std::string>& a : attributes)
			{
				attrString += " " + static_cast<std::string>(a.first) + "=\"" +
						static_cast<std::string>(a.second) + "\"";
			}

			for (const Tag& c : children)
			{
				childString += (std::string)c;
			}
		}
		if (type == TAG_NORMAL && children.size() > 0)
		{
			endString = "</" + static_cast<std::string>(*name) + ">";
		}

		beginString = tagOpening[type] + static_cast<std::string>(*name) +
				attrString +
				(children.size() == 0 && type == TAG_NORMAL ? "/" : "") + 
				tagClosing[type];
	}

	return beginString + childString + endString;
}

void parseDocument(FileString& fullCode, size_t& currentLocale,
		Document& d)
{
	eatWhiteSpace(fullCode, currentLocale);
	parseTags(fullCode, currentLocale, d.tags);
}

std::string readEntireFile(std::string const& path)
{
	std::string retVal;
	std::string line;
	std::ifstream inFile;

	inFile.open(path.c_str());
	
	while (!inFile.eof())
	{
		getline(inFile, line);
		retVal += line;
	}

	inFile.close();

	return retVal;
}

size_t countTagChildren(const Tag& t)
{
	size_t totalCount = 0;
	for (std::vector<Tag>::const_iterator i = t.children.begin();
			i != t.children.end(); ++i)
	{
		if (i->type != Tag::TAG_TEXT)
		{
			totalCount += 1;
		}
		if (i->children.size() > 0)
		{
			totalCount += countTagChildren(*i);
		}
	}
	return totalCount;
}

size_t countDocumentTags(const Document& d)
{
	size_t totalCount = 0;
	for (std::vector<Tag>::const_iterator i = d.tags.begin();
			i != d.tags.end(); ++i)
	{
		totalCount += 1;
		totalCount += countTagChildren(*i);
	}
	return totalCount;
}


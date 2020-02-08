#include "Quizno50XML.hpp"
#include "FileString.hpp"
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

long eatWhiteSpace(FileString& fullCode,
		long& currentLocale)
{
	currentLocale = fullCode.find_first_not_of(" \t\n\r", currentLocale);
	if ((unsigned long)currentLocale == std::string::npos)
	{
		currentLocale = fullCode.length();
	}
	return currentLocale;
}

void readIdentifier(FileString& fullCode, long& currentLocale,
		long& idStart, long& idLen)
{
	idStart = currentLocale;
	currentLocale = fullCode.find_first_not_of(ALPHA_UPPER ALPHA_LOWER
			NUMBERS "_-:", currentLocale);
	idLen = currentLocale - idStart;
	if (idLen <= 0)
	{
		currentLocale = idStart;
		idStart = -1;
		idLen = -1;
	}
}

long readSingleCharacterToken(FileString& fullCode,
		long& currentLocale, char tokenChar)
{
	long originalLocale = currentLocale;

	if (fullCode[currentLocale] == tokenChar)
	{
		++currentLocale;
	}
	else
	{
		return -1;
	}

	return originalLocale;
}

long readOpenTag(FileString& fullCode, long& currentLocale)
{
	return readSingleCharacterToken(fullCode, currentLocale, '<');
}

long readAssignment(FileString& fullCode,
		long& currentLocale)
{
	return readSingleCharacterToken(fullCode, currentLocale, '=');
}

void readValue(FileString& fullCode, long& currentLocale,
		long& valueLocale, long& valueLen)
{
	long originalLocale = currentLocale;
	long endLocale = currentLocale;
	bool firstGo = true;

	if (fullCode[currentLocale] == '"' || fullCode[currentLocale] == '\'')
	{
		++currentLocale;
	}
	valueLocale = currentLocale;
	while ((endLocale > 0 && firstGo)
			|| fullCode.substr(currentLocale - 1, 2) == "\\\"")
	{
		endLocale = fullCode.find_first_of("\"'", currentLocale);
        if ((unsigned long)endLocale == std::string::npos)
        {
            currentLocale = originalLocale;
            valueLocale = -1;
            valueLen = -1;
            return;
		}
		currentLocale = endLocale + 1;
		firstGo = false;
	}
	valueLen = endLocale - valueLocale;
}

void parseAttribute(FileString& fullCode, long& currentLocale,
		long& attrKey, long& attrKeyLen,
		long& attrVal, long& attrValLen)
{
	attrKey = attrKeyLen = attrVal = attrValLen = -1;
	readIdentifier(fullCode, currentLocale, attrKey, attrKeyLen);
	if (attrKey == -1) return;
	eatWhiteSpace(fullCode, currentLocale);
	readAssignment(fullCode, currentLocale);
	eatWhiteSpace(fullCode, currentLocale);
	readValue(fullCode, currentLocale, attrVal, attrValLen);
}

void parseAttributes(FileString& fullCode,
		long& currentLocale, std::vector<Attribute>& attrs)
{
	long attrVal, attrValLen, attrName, attrNameLen;
	parseAttribute(fullCode, currentLocale, attrName, attrNameLen,
			attrVal, attrValLen);
	if (attrName == -1)
	{
		return;
	}
	attrs.push_back(Attribute(attrName, attrNameLen, attrVal, attrValLen));
	eatWhiteSpace(fullCode, currentLocale);
	parseAttributes(fullCode, currentLocale, attrs);
}

void readEndTag(FileString& fullCode, long& currentLocale,
		long& endTagLen)
{
	long originalLocale = currentLocale;
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
		endTagLen = -1;
	}
}

void parseCloseTag(FileString& fullCode, long& currentLocale,
		long& tagName, long& tagNameLen)
{
	long originalLocale = currentLocale;
	long tag, tagLen;
	if (fullCode[currentLocale] != '<')
	{
		long errorLocale = currentLocale;
		currentLocale = originalLocale;
		throw ParseError("Expected closing tag.", errorLocale);
	}
	currentLocale += 1;
	eatWhiteSpace(fullCode, currentLocale);
	if (fullCode[currentLocale] != '/')
	{
		long errorLocale = currentLocale;
		currentLocale = originalLocale;
		throw ParseError("Expected closing tag.", errorLocale);
	}
	++currentLocale;
	readIdentifier(fullCode, currentLocale, tag, tagLen);
	if (tag == -1)
	{
		tagName = -1;
		tagNameLen = -1;
		return;
	}
	eatWhiteSpace(fullCode, currentLocale);
	if (fullCode[currentLocale] != '>')
	{
		long errorLocale = currentLocale;
		currentLocale = originalLocale;
		throw ParseError("Expected closing tag end.", errorLocale);
	}
	++currentLocale;
}

void parseText(FileString& fullCode, long& currentLocale,
		long& textStart, long& textLen)
{
	long end = fullCode.find("<", currentLocale);
	textStart = currentLocale;
	if (end < 0 || end == currentLocale)
	{
		textStart = -1;
		textLen = -1;
		return;
	}
	textLen = end - textStart;
	currentLocale += textLen;
}

void parseTag(FileString& fullCode, long& currentLocale,
		Tag& result);

void parseCommentTag(FileString& fullCode, long& currentLocale,
		Tag& commentTag)
{
	long originalLocale = currentLocale;
	long end;

	if (fullCode.substr(currentLocale, 4) != "<!--")
	{
		return;
	}

	currentLocale += 4;
    commentTag.name = currentLocale;

	end = fullCode.find("-->", currentLocale);

	if (end == -1)
	{
		long errorLocale = currentLocale;
		currentLocale = originalLocale;
		throw ParseError("Couldn't find comment end.", errorLocale);
	}

	currentLocale = end + 3;
    commentTag.nameLen = end - commentTag.name;
	commentTag.type = Tag::TAG_COMMENT;
}

void parseTagsAndText(FileString& fullCode,
		long& currentLocale, std::vector<Tag>& tags,
		long& itemsParsed)
{
	long textStart = -1, textEnd = -1;
	Tag newTag;

	if (fullCode.length() == (unsigned long)currentLocale)
	{
		return;
	}

	parseTag(fullCode, currentLocale, newTag);
	if (newTag.name == -1)
	{
		parseCommentTag(fullCode, currentLocale, newTag);
		if (newTag.name == -1)
		{
			parseText(fullCode, currentLocale, textStart, textEnd);
			newTag.name = textStart;
			newTag.nameLen = textEnd;
			newTag.type = Tag::TAG_TEXT;
			if (newTag.name == -1)
			{
				// Can't parse anything... We're done here...
				return;
			}
		}
	}
	tags.push_back(newTag);
	++itemsParsed;
	parseTagsAndText(fullCode, currentLocale, tags, itemsParsed);
}

void parseTag(FileString& fullCode, long& currentLocale, Tag& result)
{
	long originalLocale = currentLocale;
	long tagName, tagNameLen, endTagLen;
	long childrenParsed = 0;

	result.name = -1;
	result.nameLen = -1;

	if ((unsigned long)currentLocale == fullCode.length())
	{
		return;
	}
	if (readOpenTag(fullCode, currentLocale) == -1) return;
	eatWhiteSpace(fullCode, currentLocale);
	readIdentifier(fullCode, currentLocale, tagName, tagNameLen);
	if (tagName == -1)
	{
		currentLocale = originalLocale;
		return;
	}
	eatWhiteSpace(fullCode, currentLocale);
	parseAttributes(fullCode, currentLocale, result.attributes);
	readEndTag(fullCode, currentLocale, endTagLen);
	if (endTagLen == -1)
	{
		currentLocale = originalLocale;
		return;
	}

	if (endTagLen == 1) // tag has children...
	{
		parseTagsAndText(fullCode, currentLocale, result.children,
				childrenParsed);
		parseCloseTag(fullCode, currentLocale, tagName, tagNameLen);
		if (tagNameLen == -1)
		{
			currentLocale = originalLocale;
			return;
		}
	}
	result.name = tagName;
	result.nameLen = tagNameLen;
	result.type = Tag::TAG_NORMAL;
}

long readMetaChar(FileString& fullCode, long& currentLocale,
		char& metaChar)
{
	metaChar = '?';
	if (readSingleCharacterToken(fullCode, currentLocale, metaChar) == -1)
	{
		metaChar = '!';
		return readSingleCharacterToken(fullCode, currentLocale, metaChar);
	}
	return currentLocale;
}

void parseMetaTag(FileString& fullCode, long& currentLocale,
		Tag& metaTag)
{
	long originalLocale = currentLocale;
	long tagId, tagIdLen;
	char metaChar = -1;
	long endTagLen;

	if ((unsigned long)currentLocale == fullCode.length())
	{
		return;
	}
    readOpenTag(fullCode, currentLocale);
    readMetaChar(fullCode, currentLocale, metaChar);
    // Verify this isn't a comment tag.
    if (fullCode[currentLocale] == '-' && fullCode[currentLocale + 1] == '-')
    {
        // This is a comment tag, we need to bail.
        currentLocale = originalLocale;
        return;
    }
    readIdentifier(fullCode, currentLocale, tagId, tagIdLen);
    metaTag.name = tagId;
    metaTag.nameLen = tagIdLen;
    metaTag.type = Tag::TAG_META;
    eatWhiteSpace(fullCode, currentLocale);
    parseAttributes(fullCode, currentLocale, metaTag.attributes);
    eatWhiteSpace(fullCode, currentLocale);
    if (metaChar == '?')
    {
        readMetaChar(fullCode, currentLocale, metaChar);
        if (metaChar != '?')
        {
            long errorLocale = currentLocale;
            currentLocale = originalLocale;
            throw ParseError("Expected '?' metachar.", errorLocale);
        }
    }
    readEndTag(fullCode, currentLocale, endTagLen);
}

void parseTags(FileString& fullCode, long& currentLocale,
		std::vector<Tag>& allTags)
{
	Tag t;
	
	if ((unsigned long)currentLocale == fullCode.length())
	{
		return;
	}

	eatWhiteSpace(fullCode, currentLocale);
	parseTag(fullCode, currentLocale, t);
	if (t.name == -1)
	{
		parseMetaTag(fullCode, currentLocale, t);
		if (t.name == -1)
		{
			parseCommentTag(fullCode, currentLocale, t);
			if (t.name == -1)
			{
				return;
			}
		}
	}
	allTags.push_back(t);
	parseTags(fullCode, currentLocale, allTags);
}

void parseDocument(FileString& fullCode, long& currentLocale,
		Document& d)
{
	eatWhiteSpace(fullCode, currentLocale);
	parseTags(fullCode, currentLocale, d.tags);
}

std::string readEntireFile(std::string& path)
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

long countTagChildren(const Tag& t)
{
	long totalCount = 0;
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

long countDocumentTags(const Document& d)
{
	long totalCount = 0;
	for (std::vector<Tag>::const_iterator i = d.tags.begin();
			i != d.tags.end(); ++i)
	{
		totalCount += 1;
		totalCount += countTagChildren(*i);
	}
	return totalCount;
}

int main(int argc, char** argv)
{
	std::string testData = "/home/quizno50/Downloads/reddit_rss.xml";
	Document d;
	long currentLocale = 0;

	if (argc > 1)
	{
		testData = argv[1];
	}

	FileString fullCode(testData);
	parseDocument(fullCode, currentLocale, d);

	std::cout << "Parsed a document with " << countDocumentTags(d) << " tags.\n";

	return 0;
}

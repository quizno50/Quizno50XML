#include <iostream>
#include <string>
#include "Quizno50XML.hpp"

#define TEST_FINALIZE(actual, expected) \
do { \
	if (actual != expected) \
	{\
		std::cerr << "Expected: " << expected << "\n"\
				    << "Actual  : " << actual << "\n\n";\
		return false;\
	}\
	return true;\
} while (0)

bool testRenderComment(void)
{
	Tag commentTag;
	std::string renderedTag;

	commentTag.name = "This is a comment.";
	commentTag.type = Tag::TAG_COMMENT;

	renderedTag = (std::string)commentTag;

	TEST_FINALIZE(renderedTag, "<!--This is a comment.-->");
}

bool testRenderMeta(void)
{
	Tag metaTag;
	std::string renderedTag;

	metaTag.name = "This is a meta tag.";
	metaTag.type = Tag::TAG_META;

	renderedTag = (std::string)metaTag;

	TEST_FINALIZE(renderedTag, "<?This is a meta tag.?>");
}

bool testRenderMetaWithAttribute(void)
{
	Tag metaTag;
	std::string renderedTag;

	metaTag.name = "xml";
	metaTag.type = Tag::TAG_META;
	metaTag.attributes.push_back(Attribute("version", "1.0"));

	renderedTag = (std::string)metaTag;

	TEST_FINALIZE(renderedTag, "<?xml version=\"1.0\"?>");
}

bool testRenderMetaWithAttributes(void)
{
	Tag metaTag;
	std::string renderedTag;

	metaTag.name = "xml";
	metaTag.type = Tag::TAG_META;
	metaTag.attributes.push_back(Attribute("version", "1.0"));
	metaTag.attributes.push_back(Attribute("encoding", "UTF-8"));

	renderedTag = (std::string)metaTag;

	TEST_FINALIZE(renderedTag, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
}

bool testRenderText(void)
{
	Tag textTag;
	std::string renderedTag;

	textTag.type = Tag::TAG_TEXT;
	textTag.name = "The quick brown fox jumped over the lazy dogs.";
	
	renderedTag = (std::string)textTag;
	TEST_FINALIZE(renderedTag, "The quick brown fox jumped over the lazy dogs.");
}

bool testRenderNormal(void)
{
	Tag normalTag;
	std::string renderedTag;

	normalTag.name = "foo";
	normalTag.type = Tag::TAG_NORMAL;

	renderedTag = (std::string)normalTag;
	TEST_FINALIZE(renderedTag, "<foo/>");
}

bool testRenderNormalWithAttribute(void)
{
	Tag t;
	std::string renderedTag;

	t.type = Tag::TAG_NORMAL;
	t.name = "foo";
	t.attributes.push_back(Attribute("bar", "baz"));

	renderedTag = (std::string)t;
	TEST_FINALIZE(renderedTag, "<foo bar=\"baz\"/>");
}

bool testRenderNormalWithAttributes(void)
{
	Tag t;
	std::string renderedTag;

	t.type = Tag::TAG_NORMAL;
	t.name = "foo";
	t.attributes.push_back(Attribute("bar", "baz"));
	t.attributes.push_back(Attribute("fizz", "buzz"));

	renderedTag = (std::string)t;
	TEST_FINALIZE(renderedTag, "<foo bar=\"baz\" fizz=\"buzz\"/>");
}

bool testRenderNormalWithChild(void)
{
	Tag t;
	Tag ct;
	std::string renderedTag;

	t.type = Tag::TAG_NORMAL;
	t.name = "foo";
	ct.type = Tag::TAG_TEXT;
	ct.name = "bar";
	t.children.push_back(ct);

	renderedTag = (std::string)t;
	TEST_FINALIZE(renderedTag, "<foo>bar</foo>");
}

bool testRenderNormalWithChildren(void)
{
	Tag t;
	Tag ct;
	Tag ctt;
	std::string renderedTag;

	t.type = Tag::TAG_NORMAL;
	t.name = "foo";
	ct.type = Tag::TAG_TEXT;
	ct.name = "bar";
	ctt.type = Tag::TAG_NORMAL;
	ctt.name = "baz";
	t.children.push_back(ct);
	t.children.push_back(ctt);

	renderedTag = (std::string)t;
	TEST_FINALIZE(renderedTag, "<foo>bar<baz/></foo>");
}

bool testRenderNormalWithAttributesAndChildren(void)
{
	Tag t;
	Tag ct;
	Tag ctt;
	std::string renderedTag;

	t.type = Tag::TAG_NORMAL;
	t.name = "foo";
	t.attributes.push_back(Attribute("one", "1"));
	t.attributes.push_back(Attribute("two", "2"));
	ct.type = Tag::TAG_TEXT;
	ct.name = "bar";
	ctt.type = Tag::TAG_NORMAL;
	ctt.name = "baz";
	ctt.attributes.push_back(Attribute("one", "1"));
	ctt.attributes.push_back(Attribute("two", "2"));
	t.children.push_back(ct);
	t.children.push_back(ctt);

	renderedTag = (std::string)t;
	TEST_FINALIZE(renderedTag, "<foo one=\"1\" two=\"2\">bar"
			"<baz one=\"1\" two=\"2\"/></foo>");
}

int main(void)
{
	bool result = true;
	bool finalResult = true;
	std::vector<bool(*)(void)> tests = { testRenderComment,
		testRenderMeta,
		testRenderMetaWithAttribute,
		testRenderMetaWithAttributes,
		testRenderText,
		testRenderNormal,
		testRenderNormalWithAttribute,
		testRenderNormalWithAttributes,
		testRenderNormalWithChild,
		testRenderNormalWithChildren,
		testRenderNormalWithAttributesAndChildren } ;

	for (auto t : tests)
	{
		result = (*t)();
		std::cout << (result ? "T" : "F");
		finalResult |= result;
	}

	return (finalResult == true ? 0 : 1);
}

#pragma once
#include <string>

class Item 
{
protected:
	std::string title;
	std::string author;

public:

	Item(const std::string& title_, const std::string& author_)
		: title(title_), author(author_) {}

	virtual ~Item();

	virtual std::string info() const = 0;

	std::string GetTitle(Item* item);
	std::string GetAuthor(Item* item);

	static void SetTitle(Item* item, std::string newTitle);
	static void SetAuthor(Item* item, std::string newAuthor);
};

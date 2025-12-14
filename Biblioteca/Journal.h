#pragma once
#include "Item.h"

#include <string>
class Journal :
	public Item {
public:

	Journal(const std::string& title_, const std::string& author_)
		: Item(title_, author_) {}

	std::string info() const override
	{
		return "[Journal] " + title + " [" + author + "]";
	}

};
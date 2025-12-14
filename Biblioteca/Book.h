#pragma once
#include "Item.h"

#include <string>
class Book : 
	public Item {
public:

	Book(const std::string& title_,const std::string author_)
		: Item(title_, author_) {}

	std::string info() const override 
	{
		return "[Book] " + title + " [" + author + "]";
	}

};
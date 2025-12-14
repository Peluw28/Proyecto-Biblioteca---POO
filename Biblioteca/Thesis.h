#pragma once
#include "Item.h"
#include <string>
#include <iostream>

class Thesis :
    public Item
{
private:

    int year;

public:

    Thesis(const std::string& title_, const std::string& author_, int year_)
        : Item(title_, author_), year(year_)
    {
        if (year_ < 1980) {
            //throw std::invalid_argument("Year must be between 1900 and 2024");
			std::cout << "Warning : NOOOOOOOO EL ANYOOOOO ES MENOH QUE 1980s" << std::endl;
		}
    }

    std::string info() const override
    {
        return "[Thesis] " + title + " [" + author + "] / [" + std::to_string(year) + "]";
	}

    static void SetYear(Thesis* thesis, int newYear);

};


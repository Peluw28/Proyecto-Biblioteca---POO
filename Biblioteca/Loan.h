#pragma once
#include "Item.h"
#include "User.h"
#include <chrono>
class Loan
{
private:

	std::chrono::system_clock::time_point loanDate;
	std::chrono::system_clock::time_point dueDate;
	std::chrono::system_clock::time_point returnDate;

public:
	User* user;
	Item* item;

	Loan(User* user_, Item* item_, std::chrono::system_clock::time_point loanStart_, std::chrono::system_clock::time_point loanEnd_, std::chrono::system_clock::time_point loanReturn_)
		: user(user_), item(item_), loanDate(loanStart_), dueDate(loanEnd_), returnDate(loanReturn_) {}

	float DaysLate(const Loan* loan);
	float SanctionPrice(int lateDays, float import);

	std::chrono::system_clock::time_point GetLoanDate();
	std::chrono::system_clock::time_point GetDueDate();
	std::chrono::system_clock::time_point GetReturnDate();
};

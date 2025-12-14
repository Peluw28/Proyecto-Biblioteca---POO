#include "Exporter.h"
#include "Loan.h"
#include "Item.h"
#include "User.h"
#include "Thesis.h"
#include "Book.h"
#include "Journal.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <chrono>
#include <iostream>

extern std::vector<Item*> catalog;
extern std::vector<Item*> filteredCatalog;
extern std::vector<User*> users;
extern std::vector<Loan*> totalLoans;

static std::string TimePointToString(const std::chrono::system_clock::time_point& tp)
{
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm;

#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();

}

static float ComputePenalty(int lateDays)
{
    if (lateDays <= 0) return 0.0f;

    if (lateDays <= 5) return 0.05f * lateDays;

    if (lateDays <= 15) return 0.15f * lateDays;

    return 0.30f * lateDays;
}

void ExportAllCSV(const std::string& dirPath)
{
    // Usuarios
    {
        std::ofstream f(dirPath + "users.csv");
        if (!f) { std::cerr << "No se pudo crear " << dirPath + "users.csv\n"; return; }
        f << "Name,Role,Import\n";

        for (User* u : users)
        {
            std::string name = u->GetName(u);
            std::string role = u->GetRole(u);
            float imp = u->GetImport(u);

            f << '"' << name << '"' << ',' << '"' << role << '"' << ',' << imp << '\n';
        }
    }

    // Los items
    {
        std::ofstream f(dirPath + "items.csv");
        if (!f) { std::cerr << "No se pudo crear " << dirPath + "items.csv\n"; return; }
        f << "Type,Title,Author,Info\n";

        for (Item* it : catalog)
        {
            std::string title = it->GetTitle(it);
            std::string author = it->GetAuthor(it);
            std::string type = "Item";
            if (dynamic_cast<Thesis*>(it)) type = "Thesis";
            else if (dynamic_cast<class Book*>(it)) type = "Book";
            else if (dynamic_cast<class Journal*>(it)) type = "Journal";

            std::string info = it->info();
            f << '"' << type << '"' << ',' << '"' << title << '"' << ',' << '"' << author << '"' << ',' << '"' << info << '"' << '\n';
        }
    }

    // Los prestamos
    {
        std::ofstream f(dirPath + "loans.csv");
        if (!f) { std::cerr << "No se pudo crear " << dirPath + "loans.csv\n"; return; }
        f << "UserName,ItemTitle,LoanDate,DueDate,ReturnDate,DaysLate,EstimatedPenalty\n";
        for (Loan* L : totalLoans)
        {
            std::string userName = L->user->GetName(L->user);
            std::string itemTitle = L->item->GetTitle(L->item);

            auto loanDate = L->GetLoanDate();
            auto dueDate = L->GetDueDate();
            auto returnDate = L->GetReturnDate();

            int lateDays = 0;
            if (returnDate > dueDate) 
            {
                auto dur = std::chrono::duration_cast<std::chrono::hours>(returnDate - dueDate);
                lateDays = static_cast<int>(dur.count() / 24);
                if ((dur.count() % 24) != 0) lateDays++;
            }

            float penalty = ComputePenalty(lateDays);

            f << '"' << userName << '"' << ',' << '"' << itemTitle << '"' << ','
                << '"' << TimePointToString(loanDate) << '"' << ','
                << '"' << TimePointToString(dueDate) << '"' << ','
                << '"' << TimePointToString(returnDate) << '"' << ','
                << lateDays << ',' << penalty << '\n';
        }
    }
}
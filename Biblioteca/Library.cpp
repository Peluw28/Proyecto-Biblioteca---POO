#include "Item.h"
#include "User.h"

#include "Book.h"
#include "Journal.h"
#include "Thesis.h"
#include "Loan.h"

#include "Exporter.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>

#include <fstream>
#include <sstream>
#include <regex>
#include <iomanip>

int userID = 0;

std::vector<Item*> catalog;
std::vector<Item*> filteredCatalog;

std::vector<User*> users;
std::vector<Loan*> totalLoans;

std::string Item::info() const {
	std::cout << "Item: " << title << std::endl;
	return "patata";
}

//A2: 0–5 días: 0,05 €/día; 6–15: 0,15 €/día; >15: 0,30 €/día
//B3: Thesis con director(string) y year >= 1980
//C1: límite por rol(Estudiante = 3, PDI = 6, PAS = 5)
//D :(
//E1: filtro por autor; orden por título ascendente
//F2: resumen por rol de usuario (préstamos activos, retrasos, sanciones)

void MainMenu();

// Funciones de CSV ---------------------------------------------------------------------------------------------------------------------------------------------

// Helper: divide una línea CSV respetando comillas
static std::vector<std::string> SplitCSVLine(const std::string& line)
{
	std::vector<std::string> out;
	std::string cur;
	bool inQuotes = false;
	for (size_t i = 0; i < line.size(); ++i) {
		char c = line[i];
		if (c == '"') {
			// si siguiente es otra comilla → comilla literal
			if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
				cur += '"';
				++i;
			}
			else {
				inQuotes = !inQuotes;
			}
		}
		else if (c == ',' && !inQuotes) {
			out.push_back(cur);
			cur.clear();
		}
		else {
			cur += c;
		}
	}
	out.push_back(cur);
	return out;
}

// Helper: convertir "YYYY-MM-DD HH:MM:SS" -> time_point (usa hora local)
static std::chrono::system_clock::time_point ParseTimePoint(const std::string& s)
{
	std::tm tm = {};
	std::istringstream ss(s);
	ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
	if (ss.fail()) {
		// fallback: ahora
		return std::chrono::system_clock::now();
	}
	std::time_t tt = std::mktime(&tm); // interpreta en hora local
	return std::chrono::system_clock::from_time_t(tt);
}

// Importa users.csv, items.csv y loans.csv desde dirPath (terminar con '/' o '\\')

void ImportAllCSV(const std::string& dirPath)
{
	// Archivo de los users
	{
		std::ifstream f(dirPath + "users.csv");
		if (f) 
		{
			std::string line;
			std::getline(f, line);
			while (std::getline(f, line)) 
			{
				if (line.empty()) continue;
				auto cols = SplitCSVLine(line);

				if (cols.size() < 3) continue;

				std::string name = cols[0];
				std::string role = cols[1];
				float imp = 0.0f;

				try { imp = std::stof(cols[2]); }
				catch (...) { imp = 0.0f; }

				User* newUser = new User(++userID, name, role, imp, false);
				users.push_back(newUser);
			}
		}
	}

	// Archivo de los items
	{
		std::ifstream f(dirPath + "items.csv");
		if (f) 
		{
			std::string line;
			std::getline(f, line); // header
			while (std::getline(f, line)) 
			{
				if (line.empty()) continue;
				auto cols = SplitCSVLine(line);
				// Expect: Type,Title,Author,Info
				if (cols.size() < 3) continue;
				std::string type = cols.size() > 0 ? cols[0] : "";
				std::string title = cols.size() > 1 ? cols[1] : "";
				std::string author = cols.size() > 2 ? cols[2] : "";
				std::string info = cols.size() > 3 ? cols[3] : "";

				if (type.find("Thesis") != std::string::npos || type == "Thesis") 
				{
					// intentar extraer año desde info (último número 4 dígitos)
					int year = 1980;
					std::smatch m;
					std::regex r(R"((\d{4}))");
					auto it = std::sregex_iterator(info.begin(), info.end(), r);
					auto end = std::sregex_iterator();
					if (it != end) {

						for (auto i = it; i != end; ++i) m = *i;
						try { year = std::stoi(m.str(1)); }
						catch (...) { year = 1980; }

					}

					Item* t = new Thesis(title, author, year);
					catalog.push_back(t);

				}

				else if (type.find("Journal") != std::string::npos || type == "Journal") 
				{
					Item* j = new Journal(title, author);
					catalog.push_back(j);

				}
				else 
				{
					Item* b = new Book(title, author);
					catalog.push_back(b);
				}
			}
		}
	}

	// El archivo de loans
	{
		std::ifstream f(dirPath + "loans.csv");
		if (f) {
			std::string line;
			std::getline(f, line);
			while (std::getline(f, line)) 
			{
				if (line.empty()) continue;

				auto cols = SplitCSVLine(line);

				if (cols.size() < 5) continue;

				std::string uname = cols[0];
				std::string ititle = cols[1];
				std::string sLoan = cols[2];
				std::string sDue = cols[3];
				std::string sReturn = cols[4];


				User* foundUser = nullptr;
				Item* foundItem = nullptr;

				for (User* u : users) 
				{
					if (u->GetName(u) == uname) { foundUser = u; break; }
				}

				for (Item* it : catalog) 
				{
					if (it->GetTitle(it) == ititle) { foundItem = it; break; }
				}

				if (!foundUser || !foundItem) continue;

				auto tpLoan = ParseTimePoint(sLoan);
				auto tpDue = ParseTimePoint(sDue);
				auto tpReturn = ParseTimePoint(sReturn);

				Loan* L = new Loan(foundUser, foundItem, tpLoan, tpDue, tpReturn);
				totalLoans.push_back(L);
			}
		}
	}

	std::cout << "Importacion desde CSV completada.\n";
}


// Todas las funciones de las clases  -----------------------------------------------------------------------------------------------------------------------------

std::chrono::system_clock::time_point Loan::GetLoanDate()
{
	return loanDate;
}

std::chrono::system_clock::time_point Loan::GetDueDate()
{
	return dueDate;
}

std::chrono::system_clock::time_point Loan::GetReturnDate()
{
	return returnDate;
}

std::string User::GetRole(User* user)
{
	return user->rol;
}

std::string Item::GetTitle(Item* item)
{
	return item->title;
}

std::string Item::GetAuthor(Item* item)
{
	return item->author;
}

float User::GetImport(User* user)
{
	return user->import;
}

void User::SetImport(User* user, float import)
{
	user->import = import;
}

void Item::SetTitle(Item* item, std::string newTitle)
{
	item->title = newTitle;
}

void Item::SetAuthor(Item* item, std::string newAuthor)
{
	item->author = newAuthor;
}

void Thesis::SetYear(Thesis* thesis, int newYear)
{
	thesis->year = newYear;
}

std::string User::GetName(User* user)
{
	return user->name;
}

void User::ToggleBlock(User* user, bool blockedStatus)
{
	user->blocked = blockedStatus;
}

bool User::GetBlockedStatus(User* user)
{
	return user->blocked;
}

float Loan::SanctionPrice(int lateDays, float import) // Calcula la sancion segun los dias de retraso
{
	if (lateDays <= 5)
	{
import += 0,05 * lateDays;
	}
	else if (lateDays <= 15 && lateDays >= 5)
	{
import += 0,15 * lateDays;
	}
	else
	{
import += 0,30 * lateDays;
	}
	return import;
}

float Loan::DaysLate(const Loan* loan) // Calcula los dias de retraso y actualiza la sancion del usuario
{
	if (loan->returnDate <= loan->dueDate)
	{
		return 0.0f;
	}
	auto lateDuration = loan->returnDate - loan->dueDate;
	int daysLate = std::chrono::duration<float, std::ratio<86400>>(lateDuration).count();

	float updImport = SanctionPrice(daysLate, loan->user->GetImport(user));
	loan->user->SetImport(loan->user, updImport);

	return daysLate;
}

// Funciones del menu principal -----------------------------------------------------------------------------------------------------------------------------

bool CanBorrow(User* user) // Comprueba si un usuario puede pedir prestado otro item segun su rol y prestamos actuales
{
	int curLoans = 0;

	for (Loan* loan : totalLoans)
	{
		if (loan->user == user)
		{
			curLoans++;
		}
	}

	std::string rol = user->GetRole(user);

	if (rol == "Estudiante" && curLoans < 3)
	{
		return true;
	}
	else if (rol == "PDI" && curLoans < 6)
	{
		return true;
	}
	else if (rol == "PAS" && curLoans < 5)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ListItems()
{
	system("cls");
	std::vector<Item*> sortedCatalog = catalog;

	std::sort(sortedCatalog.begin(), sortedCatalog.end(), // Ordena el catalogo por titulo ascendente para asemejarse a una biblioteca real
		[](Item* a, Item* b) {
			return a->GetTitle(a) < b->GetTitle(b);
		}
	);

	for (Item* item : sortedCatalog)
	{
		std::cout << item->info() << std::endl;
	}

	system("pause");
	MainMenu();
}

void Sort(std::string author) // Se encarga de ordenar el catalogo por autor
{
	for (Item* item : catalog)
	{
		std::string authorItem = item->GetAuthor(item);
		if (authorItem == author)
		{
			filteredCatalog.push_back(item);
		}
	}

	std::sort(filteredCatalog.begin(), filteredCatalog.end(),
		[](Item* a, Item* b) {
			return a->GetTitle(a) < b->GetTitle(b);
		}
	);
}

void RoleSummary()
{
	system("cls");
	std::vector<Loan*> studentLoans;
	std::vector<Loan*> pdiLoans;
	std::vector<Loan*> pasLoans;

	int totalLateDays = 0;
	float totalImport = 0.0f;

	for (Loan* loan : totalLoans) // Separa los loans por rol
	{
		std::string role = loan->user->GetRole(loan->user);
		if (role == "Estudiante")
		{
			studentLoans.push_back(loan);
		}
		else if (role == "PDI")
		{
			pdiLoans.push_back(loan);
		}
		else if (role == "PAS")
		{
			pasLoans.push_back(loan);
		}
	}

	for (Loan* loan : studentLoans) // Los loans de estudiantes
	{
		int lateDays = loan->DaysLate(loan);
		float import = loan->user->GetImport(loan->user);

		totalLateDays += lateDays;
		totalImport += import;
	}
	std::cout << "Estudiantes: " << studentLoans.size() << "\nDias de retraso: " << totalLateDays << "\nSanciones: " << totalImport << " eur" << std::endl;
	std::cout << std::endl;

	totalLateDays = 0;
	totalImport = 0.0f;

	for (Loan* loan : pdiLoans) // Los loans de PDI
	{
		int lateDays = loan->DaysLate(loan);
		float import = loan->user->GetImport(loan->user);
		totalLateDays += lateDays;
		totalImport += import;
	}

	std::cout << "PDI: " << pdiLoans.size() << "\nDias de retraso: " << totalLateDays << "\nSanciones: " << totalImport << " eur" << std::endl;
	std::cout << std::endl;

	totalLateDays = 0;
	totalImport = 0.0f;

	for (Loan* loan : pasLoans) // Los loans de PAS
	{
		int lateDays = loan->DaysLate(loan);
		float import = loan->user->GetImport(loan->user);
		totalLateDays += lateDays;
		totalImport += import;
	}

	std::cout << "PAS: " << pasLoans.size() << "\nDias de retraso: " << totalLateDays << "\nSanciones: " << totalImport << " eur" << std::endl;
	std::cout << std::endl;

	system("pause");
	MainMenu();
}

//void CreateBase()
//{
//
//	Item* i1 = new Book("C Programming", "Darkio Gojo");
//	Item* i5 = new Book("W Programming", "Darkio Gojo");
//	Item* i6 = new Book("R Programming", "Darkio Gojo");
//	Item* i7 = new Book("A Programming", "Darkio Gojo");
//
//	Item* i2 = new Journal("C-- Programming", "Hideo Kojima");
//
//	Item* i3 = new Thesis("Skibidi Research", "Dr. Who", 2022);
//	Item* i4 = new Thesis("Le fishe au chocolat: Thesisse", "Sir Jhean Louisse le III", 2006);
//
//	User* u1 = new User(userID + 1, "Alice", "Estudiante", 0, false);
//	userID++;
//	User* u2 = new User(userID + 1, "Bob", "PDI", 67, false);
//	userID++;
//	User* u3 = new User(userID + 1, "Charlie", "PAS", 1, true);
//	userID++;
//
//	Loan* l1 = new Loan(u2, i4, std::chrono::system_clock::now(), std::chrono::system_clock::now() + std::chrono::hours(24 * 14), std::chrono::system_clock::now() + std::chrono::hours(24 * 18));
//	Loan* l2 = new Loan(u1, i1, std::chrono::system_clock::now(), std::chrono::system_clock::now() + std::chrono::hours(24 * 7), std::chrono::system_clock::now() + std::chrono::hours(24 * 50));
//	Loan* l3 = new Loan(u3, i3, std::chrono::system_clock::now(), std::chrono::system_clock::now() + std::chrono::hours(24 * 2), std::chrono::system_clock::now() + std::chrono::hours(24 * 1750));
//
//	users.push_back(u1);
//	users.push_back(u2);
//	users.push_back(u3);
//
//	totalLoans.push_back(l1);
//	totalLoans.push_back(l2);
//	totalLoans.push_back(l3);
//
//	catalog.push_back(i1);
//	catalog.push_back(i2);
//	catalog.push_back(i3);
//	catalog.push_back(i4);
//	catalog.push_back(i5);
//	catalog.push_back(i6);
//	catalog.push_back(i7);
//
//}


//void CreateBase2()
//{
//
//	Item* i1 = new Book("Matar a un ruiseñor", "Harper Lee");
//	Item* i2 = new Book("Ve y pon un centinela", "Harper Lee");
//
//	Item* i3 = new Book("Cumbres borrascosas", "Emily Bronte");
//
//	Item* i4 = new Book("El retrato de Dorian Gray", "Oscar Wilde");
//	Item* i5 = new Book("El crimen de Lord Arthur Savile", "Oscar Wilde");
//	Item* i6 = new Book("El fantasma de Canterville", "Oscar Wilde");
//
//	Item* i7 = new Journal("El oficio de vivir", "Cesare Pavese");
//
//	Item* i8 = new Journal("Diarios de motocicleta", "Ernesto Guevara");
//	Item* i9 = new Journal("El diario del Che en Bolivia", "Ernesto Guevara");
//
//	Item* i10 = new Thesis("Subjects of Desire", "Judith Butler", 1987);
//
//	Item* i11 = new Thesis("La naissance du maquis dans le Sud-Cameroun", "Achille Mbembe", 1996);
//
//	Item* i12 = new Thesis("Longing and Belonging", "Allison Janet Pugh", 2009);
//
//	User* u1 = new User(userID + 1, "Guillermo Patatas", "Estudiante", 12.3, false);
//	userID++;
//
//	User* u2 = new User(userID + 1, "Lisa Simpson", "Estudiante", 0, false);
//	userID++;
//
//	User* u3 = new User(userID + 1, "Marcos Marquito", "Estudiante", 129, true);
//	userID++;
//
//	User* u4= new User(userID + 1, "Juan Pablo", "PDI", 0, false);
//	userID++;
//
//	User* u5 = new User(userID + 1, "Maria Jose", "PAS", 1, true);
//	userID++;
//
//	Loan* l1 = new Loan(u2, i4, std::chrono::system_clock::now(), std::chrono::system_clock::now() + std::chrono::hours(24 * 14), std::chrono::system_clock::now() + std::chrono::hours(24 * 12));
//	Loan* l2 = new Loan(u1, i1, std::chrono::system_clock::now(), std::chrono::system_clock::now() + std::chrono::hours(24 * 30), std::chrono::system_clock::now() + std::chrono::hours(24 * 35));
//	Loan* l3 = new Loan(u4, i3, std::chrono::system_clock::now(), std::chrono::system_clock::now() + std::chrono::hours(24 * 20000), std::chrono::system_clock::now() + std::chrono::hours(24 * 10249));
//
//	users.push_back(u1);
//	users.push_back(u2);
//	users.push_back(u3);
//	users.push_back(u4);
//	users.push_back(u5);
//
//	totalLoans.push_back(l1);
//	totalLoans.push_back(l2);
//	totalLoans.push_back(l3);
//
//	catalog.push_back(i1);
//	catalog.push_back(i2);
//	catalog.push_back(i3);
//	catalog.push_back(i4);
//	catalog.push_back(i5);
//	catalog.push_back(i6);
//	catalog.push_back(i7);
//	catalog.push_back(i8);
//	catalog.push_back(i9);
//	catalog.push_back(i10);
//	catalog.push_back(i11);
//	catalog.push_back(i12);
//}


void CreateUser()
{
	system("cls");

	std::string name;
	std::string role;

	std::cout << "Introduce el nombre del nuevo usuario : ";
	std::getline(std::cin >> std::ws, name);

	std::cout << "Introduce el rol : [Estudiante], [PDI], [PAS] ";
	std::cin >> role;

	if (role != "Estudiante" && role != "PDI" && role != "PAS") // Verifica el rol
	{
		std::cout << "Rol no valido, vuelvelo a intentar\n";

		system("pause");
		MainMenu();
	}

	User* newUser = new User(userID + 1, name, role, 0, false);
	userID++;

	std::cout << "Usuario agregado!\n";

	system("pause");
	MainMenu();
}

void ToggleBlockUser(bool block)
{
	system("cls");

	bool hasFound = false;
	std::string name;

	std::cout << "Introduce el nombre del usuario : \n";
	std::getline(std::cin >> std::ws, name);

	for (User* user : users) // Busca el usuario
	{
		if (user->GetName(user) == name)
		{
			user->ToggleBlock(user, block);

			if (!block) user->SetImport(user, 0.0f);
			std::cout << name << " estado cambiado correctamente\n";
			hasFound = true;
			break;
		}
	}

	if (!hasFound)
	{
		std::cout << "Usuario no encontrado, intentelo de nuevo\n";
	}

	system("pause");
	MainMenu();
}

void CreateItem()
{
	system("cls");
	std::string itemType;

	std::cout << "Introduce el tipo de item : [book], [journal], [thesis] \n";
	std::getline(std::cin >> std::ws, itemType);

	if (itemType != "thesis") // Book o Journal
	{

		std::string title;
		std::string author;

		std::cout << "Introduce el titulo : ";
		std::getline(std::cin >> std::ws, title);

		std::cout << "Introduce el autor : ";
		std::getline(std::cin >> std::ws, author);

		Item* newBook = new Book(title, author);
		catalog.push_back(newBook);

		std::cout << "Item agregado!\n";

	}
	else // Para la tesis, necesita el año ademas
	{

		std::string title;
		std::string author;
		int year;

		std::cout << "Introduce el titulo : ";
		std::getline(std::cin >> std::ws, title);

		std::cout << "Introduce el autor : ";
		std::getline(std::cin >> std::ws, author);

		std::cout << "Introduce el año : ";
		std::cin >> year;

		Item* newThesis = new Thesis(title, author, year);
		catalog.push_back(newThesis);

		std::cout << "Item agregado!\n";

	}
	system("pause");
	MainMenu();
}

void DeleteItem()
{
	system("cls");

	bool hasFound = false;
	std::string title;
	std::cout << "Introduce el nombre del item para eliminar : ";
	std::getline(std::cin >> std::ws, title);

	for (int i = 0; i < catalog.size(); i++) // Busca el item
	{
		if (catalog.at(i)->GetTitle(catalog.at(i)) == title)
		{
			hasFound = true;

			delete catalog.at(i);
			catalog.erase(catalog.begin() + i);
			break;
		}

	}

	if (!hasFound)
	{
		std::cout << "Item does not exist, try again.\n";
	}
	else
	{
		std::cout << "Item edited!\n";
	}

	system("pause");

	MainMenu();
}

void EditItem()
{
	system("cls");

	bool hasFound = false;
	std::string title;
	std::cout << "Introduce el titulo del item que quieres editar : ";
	std::getline(std::cin >> std::ws, title);

	for (int i = 0; i < catalog.size(); i++) // Busca el item
	{
		if (catalog.at(i)->GetTitle(catalog.at(i)) == title)
		{
			std::cout << "- Item encontrado -\n";

			hasFound = true;
			std::string newTitle;
			std::string newAuthor;

			std::cout << "Titulo actual : " << catalog.at(i)->GetTitle(catalog.at(i)) << std::endl;
			std::cout << "Introduce el nuevo titulo : ";
			std::getline(std::cin >> std::ws, newTitle);

			std::cout << "Autor actual : " << catalog.at(i)->GetAuthor(catalog.at(i)) << std::endl;
			std::cout << "Introduce el nuevo autor : ";
			std::getline(std::cin >> std::ws, newAuthor);

			catalog.at(i)->SetTitle(catalog.at(i), newTitle);
			catalog.at(i)->SetAuthor(catalog.at(i), newAuthor);

			break;
		}

	}

	if (!hasFound)
	{
		std::cout << "Item does not exist, try again.\n";
	}
	else
	{
		std::cout << "Item edited!\n";
	}

	system("pause");
	MainMenu();
}

void Search()
{
	filteredCatalog.clear();
	system("cls");

	std::string author;
	std::cout << "Introduzca el autor a buscar: ";

	std::getline(std::cin >> std::ws, author);

	Sort(author);

	std::cout << "Resultados de la busqueda por autor " << author << ":\n";

	for (Item* item : filteredCatalog)
	{
		std::cout << item->info() << std::endl;
	}

	system("pause");
	MainMenu();
}

void TakeLoan()
{
	system("cls");

	std::string userName;
	std::string itemTitle;
	Item* itemFound = nullptr;
	User* userFound = nullptr;
	int maxDate;

	std::cout << "Introduce el nombre del usuario : ";
	std::getline(std::cin >> std::ws, userName);

	std::cout << "Introduce el titulo del item : ";
	std::getline(std::cin >> std::ws, itemTitle);

	for (User* User : users) // Busca usuario
	{
		if (User->GetName(User) == userName)
		{
			if (User->GetBlockedStatus(User))
			{
				std::cout << "El usuario no puede aceptar mas prestamos\n";
				system("pause");
				MainMenu();
			}
			userFound = User;
			break;
		}
	}

	for (Item* itemCheck : catalog) // Busca item
	{
		if (itemCheck->GetTitle(itemCheck) == itemTitle)
		{
			itemFound = itemCheck;
			break;
		}
	}

	if (userFound == nullptr || itemFound == nullptr) // Si no encuentra usuario o item
	{
		std::cout << "Usuario o item no encontrado\n";
		system("pause");
		MainMenu();
		return;
	}

	std::cout << "Introduce el maximo de dias para el prestamo : ";
	std::cin >> maxDate;

	Loan* newLoan = new Loan(userFound, itemFound, std::chrono::system_clock::now(), std::chrono::system_clock::now() + std::chrono::hours(24 * maxDate), std::chrono::system_clock::now());
	totalLoans.push_back(newLoan);

	std::cout << "Prestamo realizado correctamente\n";

	system("pause");
	MainMenu();
}

void ReturnLoan()
{
	system("cls");

	bool hasFound = false;
	std::string userName;
	std::string itemTitle;
	int daysLate = 0;

	std::cout << "Inserta el nombre del usuario : ";
	std::getline(std::cin >> std::ws, userName);

	std::cout << "Inserta el titulo del item : ";
	std::getline(std::cin >> std::ws, itemTitle);

	std::cout << "Inserta los dias de retraso en la devolucion : ";
	std::cin >> daysLate;

	for (Loan* loan : totalLoans)
	{
		if (loan->user->GetName(loan->user) == userName && loan->item->GetTitle(loan->item) == itemTitle)
		{
			hasFound = true;
			if (daysLate > 0)
			{
				std::cout << "Se ha devuelto con  " << daysLate << " dias de retraso y por ello se han aplicado : " << loan->SanctionPrice(daysLate, loan->user->GetImport(loan->user)) << " euros de sancion\n";

				loan->user->SetImport(loan->user, loan->SanctionPrice(daysLate, loan->user->GetImport(loan->user))); // Aplica sancion
			}
			else
			{
				std::cout << "Item devuelto correctamente sin retrasos\n";
			}

			totalLoans.erase(std::remove(totalLoans.begin(), totalLoans.end(), loan), totalLoans.end()); // Elimina el prestamo de la lista y de la memoria
			delete loan;

			break;
		}
	}

	if (!hasFound)
	{
		std::cout << "El prestamo no se ha encontrado, vuelvelo a intentar\n";
	}
	system("pause");
	MainMenu();
}

void MainMenu()
{
	system("cls");

	std::cout << "Bienvenido al menu principal, seleccione una accion :\n";

	std::cout << "I.add-item / remove-item / edit-item\n";
	std::cout << "II.add-user / block-user / unblock-user\n";
	std::cout << "III.loan / return\n";
	std::cout << "IV.search / list\n";
	std::cout << "V.report\n";
	std::cout << "VI.export / import\n";

	std::string command;

	std::cin >> command;
	std::cout << std::endl;

	// Todos los posibles comandos del menu principal ------------------------------------------------------------------------------------------------

	if (command == "add-item")
	{
		CreateItem();
	}

	if (command == "remove-item")
	{
		DeleteItem();
	}

	if (command == "edit-item")
	{
		EditItem();
	}

	if (command == "report")
	{
		RoleSummary();
	}

	if (command == "add-user")
	{
		CreateUser();
	}

	if (command == "block-user")
	{
		ToggleBlockUser(true);
	}
	else if (command == "unblock-user")
	{
		ToggleBlockUser(false);
	}

	if (command == "loan")
	{
		TakeLoan();
	}

	if (command == "return")
	{
		ReturnLoan();
	}

	if (command == "search")
	{
		Search();
	}

	if (command == "list")
	{
		ListItems();
	}

	if (command == "export")
	{
		std::string dirPath = "";

		std::cout << "Introduce el directorio donde exportar los CSV (terminar con '/' o '\\') : ";
		std::getline(std::cin >> std::ws, dirPath);

		ExportAllCSV(dirPath);

		std::cout << "Datos exportados correctamente\n";

		system("pause");
		MainMenu();
	}

	if (command == "import")
	{
		std::string dirPath = "";

		std::cout << "Introduce el directorio desde donde importar los CSV (terminar con '/' o '\\') : ";
		std::getline(std::cin >> std::ws, dirPath);

		ImportAllCSV(dirPath);

		std::cout << "Datos importados correctamente\n";

		system("pause");
		MainMenu();
	}

	std::cout << "Comando no reconocido, intentalo de nuevo\n";

	system("pause");
	MainMenu();
}

int main()
{
	std::cout << "Comenzando :\n";

	//CreateBase();
	//CreateBase2();

	MainMenu();
}
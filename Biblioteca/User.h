#pragma once

#include <string>

class User
{
private:
    std::string rol;
    int ID;
    std::string name;
    float import;
    bool blocked;

public:

    std::string GetName(User* user);
    std::string GetRole(User* user);
    float GetImport(User* user);

    void ToggleBlock(User* user, bool blockedStatus);
    bool GetBlockedStatus(User* user);

    void SetImport(User* user, float import);

    User(const int ID_, std::string name_, std::string rol_, float import_, bool blocked_)
        : ID(ID_), name(name_), rol(rol_), import(import_), blocked(blocked_) {
    }
};

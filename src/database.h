#ifndef DATABASE_H
#define DATABASE_H

#include <mysql.h>
#include <string>
#include "PasswordFunctions.h"

using namespace std;

class Database {
private:
    pass passFobj;
    MYSQL* conn;

public:
    // Constructor
    Database();

    // Getter for the connection
    MYSQL* getConnection() const;

    // Destructor
    ~Database();

    // Method to check user credentials
    string checkUser( string& username,  string& password);
};

#endif // DATABASE_H

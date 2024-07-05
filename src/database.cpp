#include <string>
#include<mysql.h>
#include<iostream>

using namespace std;
#include "database.h"

// Constructor
Database::Database() {
    conn = mysql_init(NULL);
    if (!conn) {
        cerr << "MySQL Initialization failed." << endl;
        exit(1);
    }

    if (!mysql_real_connect(conn, "localhost", "root", "+@4427JDm#;", "testdb", 3306, NULL, 0)) {
        cerr << "Failed to connect to database: Error: " << mysql_error(conn) << endl;
        exit(1);
    }
}
// Get connection
MYSQL* Database::getConnection() const {
    return conn;
}

// Destructor
Database::~Database()
{
    mysql_close(conn);
}

string Database::checkUser(string& username, string& password)
{
    string userType;
    string query = "SELECT * FROM admin WHERE username='" + username + "' AND password='" + password + "'";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in checking admin: " << mysql_error(conn) << endl;
        return "null";
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (mysql_num_rows(res) > 0) {
        userType = "admin";
        mysql_free_result(res);
        return "admin";
    }
    mysql_free_result(res);
    query = "SELECT * FROM staff WHERE username='" + username + "' AND password='" + password + "'";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in checking staff: " << mysql_error(conn) << endl;
        return "null";
    }
    res = mysql_store_result(conn);
    if (mysql_num_rows(res) > 0) {
        userType = "staff";
        mysql_free_result(res);
        return "staff";
    }
    mysql_free_result(res);
    query = "SELECT * FROM student WHERE username='" + username + "' AND password='" + password + "'";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in checking student: " << mysql_error(conn) << endl;
        return "null";
    }
    res = mysql_store_result(conn);
    if (mysql_num_rows(res) > 0) {
        userType = "student";
        mysql_free_result(res);
        return "student";
    }
    mysql_free_result(res);
    return "none";
}


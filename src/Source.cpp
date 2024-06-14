#include <iostream>
#include <string>
#include<mysql.h>
#include <conio.h>

#include "database.h"
#include "admin.h"
#include "staff.h"
#include "student.h"



using namespace std;

int main()
{
    Database db;
    int choice;


    string username, password, userType;
    do {
        cout << "-------ERP MENU-------\n";
        cout << "1. Login" << endl;
        cout << "2. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        // Check if the input operation failed
        if (cin.fail())
        {
            cin.clear(); // Clear the error state
            cin.ignore(50, '\n');
            cout << "Invalid input. Please enter a number." << endl << endl;
            continue; // Skip the switch statement and prompt again
        }

        switch (choice)
        {
        case 1:
            cout << "Enter username: ";
            cin >> username;
            cout << "Enter password: ";
            password = "";

            //Making the password not Visible
            char ch;
            while ((ch = _getch()) != 13) { // 13 is the ASCII code for Enter
                if (ch == 8) { // 8 is the ASCII code for Backspace
                    if (!password.empty()) 
                    {
                        password.pop_back();
                        cout << "\b \b"; // Erase the last character on the console
                    }
                }
                else {
                    password.push_back(ch);
                    cout << '*'; // Print * instead of the actual character
                }
            }
            cout << "\n";

            userType = db.checkUser(username, password);
            if (userType == "none")
            {
                cout << "No user of that username exists!!\n";
                break;
            }
            else if (userType == "null")
            {
                cout << "Error in query!!!??" << endl;
                break;
            }
            else
            {
                cout << "Login successful." << endl;
                if (userType == "admin")
                {
                    Admin admin;
                    admin.adminMenu(db, username);
                }
                else if (userType == "staff")
                {
                    Staff staff;
                    staff.staffMenu(db, username);
                }
                else if (userType == "student")
                {
                    Student student;
                    student.studentMenu(db, username);
                }
            }
            break;
        case 2:
            cout << "Exiting..." << endl;
            break;
        default:
            cout << "Invalid choice. Please enter again." << endl;
            break;
        }
    } while (choice != 2);

    return 0;
}







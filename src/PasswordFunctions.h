#pragma once
#ifndef PASSWORDFUNCTIONS_H
#define PASSWORDFUNCTIONS_H

#include <string>
class Database;

using namespace std;

class pass 
{

public:
	pass();
	string caesarEncrypt(const string& text, int shift);
	string caesarDecrypt(const string& text, int shift);
	string getPass();
	bool checkExistsUser(Database& db, string& username, string userType);
	bool checkUserExists(Database& db, const string& username);
	string getCurrentDateTime();
	string create_uuid();
	string generateOTP();
	string generateSalt();
	void sha256(const string inputStr, stringstream& ss);
	void addPassword(Database& db, const string& username, const string& hash, const string& salt);
	void sendMail(string& email, string& otp,string& username);
	void changePasswordKnown(Database& db,string& username, string& userType);
	void changePasswordOtp(Database& db, string& username, string& userType);
};

#endif
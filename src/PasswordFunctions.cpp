#include "PasswordFunctions.h"

#include <stdio.h>
#include <string>
#include <curl/curl.h>

#include <iostream>
#include <fstream>
#include <conio.h>
#pragma comment(lib, "rpcrt4.lib")
#include <windows.h>

#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <random>
#include <openssl/sha.h>
#include "database.h"
using namespace std;

#pragma warning(disable : 4996)

#define FROM_MAIL     "patbot2045@gmail.com"

pass::pass() {

}

bool pass::is_pdf(const string& file_content) 
{
    return file_content.compare(0, 5, "%PDF-") == 0;
}

bool pass::is_doc(const string& file_content) 
{
    static const unsigned char doc_magic_number[] = { 0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1 };
    return file_content.compare(0, 8, reinterpret_cast<const char*>(doc_magic_number), 8) == 0;
}
bool pass::assignmentAlreadySubmitted(Database& db, int assignNo,int studentId)
{
    MYSQL* conn = db.getConnection();
    // Prepare the SQL query to check if the assignment exists
    string query = "SELECT COUNT(*) FROM assignment_files WHERE assign_no = " + to_string(assignNo) + " AND student_id = " + to_string(studentId);

    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in querying database: " << mysql_error(conn) << endl;
        return false;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    bool exists = atoi(row[0]) > 0;

    // Clean up
    mysql_free_result(res);

    return exists;
}


bool pass::checkDeadlinePass(Database& db, int assignNo)
{
    MYSQL* conn = db.getConnection();
    string query = "SELECT deadline_date from assignment_ques WHERE assign_no = " + to_string(assignNo);
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in retrieving deadline_date " << mysql_error(conn) << endl;
        return false;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return false;
    }

    MYSQL_ROW row;
    string deadline_date;
    // Fetch and store each assign_no and name in the vector
    while ((row = mysql_fetch_row(res)) != nullptr) {
        deadline_date = row[0];
    }
    mysql_free_result(res);

    time_t t = time(0);
    struct tm* now = localtime(&t);
    int current_year = now->tm_year + 1900;
    int current_month = now->tm_mon + 1;
    int current_day = now->tm_mday;

    int year, month, day;
    sscanf(deadline_date.c_str(), "%d-%d-%d", &year, &month, &day);

    // Compare the deadline date with the current date
    if (year < current_year || (year == current_year && (month < current_month || (month == current_month && day < current_day)))) {
        // Deadline has passed, perform some action
        return true;
    }
    else {
        // Deadline has not passed
        return false;
    }

}


string pass::caesarEncrypt(const string& text, int shift) {
    string result = "";

    // Traverse the text
    for (int i = 0; i < text.length(); i++) {
        if (isupper(text[i])) {
            result += char(int(text[i] + shift - 65) % 26 + 65);
        }
        else if (islower(text[i])) {
            result += char(int(text[i] + shift - 97) % 26 + 97);
        }
        else {
            result += text[i]; // Non-alphabetical characters are unchanged
        }
    }
    return result;
}

// Decrypt function
string pass::caesarDecrypt(const string& text, int shift) 
{
    return caesarEncrypt(text, 26 - shift);
}

string pass::getPass()
{
    string filename = "pass_app.txt";
    ifstream file(filename, ios::binary); // Open the file in binary mode
    if (!file) {
        cerr << "Unable to open file: " << filename << endl;
        exit(1);
    }

    // Read the first three bytes to check for BOM
    char bom[3] = { 0 };
    file.read(bom, 3);

    // Check if BOM is present (0xEF, 0xBB, 0xBF for UTF-8)
    if (bom[0] != (char)0xEF && bom[1] != (char)0xBB && bom[2] != (char)0xBF)
    {
        // If no BOM, move the file pointer back to the start
        file.seekg(0, ios::beg);
    }

    // Read the rest of the file
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    return content;
}

bool pass::checkExistsUser(Database& db, string& username, string userType)
{
    MYSQL* conn = db.getConnection();
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return false;
    }
    string query;
    if (userType == "staff") {
        query = "SELECT COUNT(*) FROM staff WHERE username = '" + username + "'";
    }
    else if (userType == "admin") {
        query = "SELECT COUNT(*) FROM admin WHERE username = '" + username + "'";
    }
    else if (userType == "student") {
        query = "SELECT COUNT(*) FROM student WHERE username = '" + username + "'";
    }
    else {
        cerr << "Invalid user type" << endl;
        return false;
    }
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in checking user existence: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    int count = (row && row[0]) ? atoi(row[0]) : 0;
    mysql_free_result(res);
    return count > 0;
}

bool pass::checkUserExists(Database& db, const string& username)
{

    MYSQL* conn = db.getConnection();
    string query = "SELECT username FROM student WHERE username = '" + username + "' UNION "
        "SELECT username FROM staff WHERE username = '" + username + "' UNION "
        "SELECT username FROM admin WHERE username = '" + username + "'";

    if (mysql_query(conn, query.c_str())) {
        cerr << "Query execution error: " << mysql_error(conn) << endl;
        return false;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        cerr << "Error storing result: " << mysql_error(conn) << endl;
        return false;
    }

    bool userExists = mysql_num_rows(res) > 0;
    mysql_free_result(res);

    return userExists;
}

static const char* payload_text;

struct upload_status {
    size_t bytes_read;
};

static size_t payload_source(char* ptr, size_t size, size_t nmemb, void* userp)
{
    struct upload_status* upload_ctx = (struct upload_status*)userp;
    const char* data;
    size_t room = size * nmemb;

    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1)) {
        return 0;
    }

    data = &payload_text[upload_ctx->bytes_read];

    if (data) {
        size_t len = strlen(data);
        if (room < len)
            len = room;
        memcpy(ptr, data, len);
        upload_ctx->bytes_read += len;

        return len;
    }

    return 0;
}

string pass::getCurrentDateTime()
{
    // Get the current time
    auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);

    // Format the time as per RFC 5322
    tm tm = *localtime(&now_time); // Convert to GMT (UTC)

    char buffer[80];

    // Format time
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %z", &tm);

    return string(buffer);
}

string pass::create_uuid()
{
    UUID uuid;
    UuidCreate(&uuid);
    char* str;
    UuidToStringA(&uuid, (RPC_CSTR*)&str);

    string newString(str);

    RpcStringFreeA((RPC_CSTR*)&str);

    return newString;
}

string pass::generateOTP()
{
    // Create a random device and a random number generator
    random_device rd;
    mt19937 gen(rd());

    // Define a distribution for 6-digit numbers
    uniform_int_distribution<> dis(100000, 999999);

    // Generate a random number
    int otp = dis(gen);


    // Convert the number to a string
    return to_string(otp);
}

string pass::generateSalt()
{
    const string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{}|;:,.<>?";
    const int length = 32;
    string salt;

    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<> distribution(0, characters.size() - 1);

    for (int i = 0; i < length; ++i) {
        salt += characters[distribution(generator)];
    }

    return salt;
}

void pass::sha256(const string inputStr, stringstream& ss)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    const unsigned char* data = (const unsigned char*)inputStr.c_str();
    SHA256(data, inputStr.size(), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
}

void pass::addPassword(Database& db, const string& username, const string& hash, const string& salt)
{
    MYSQL* conn = db.getConnection();
    string query = "INSERT INTO holy_salt (username, salt, hash) VALUES ('" + username + "', '" + salt + "', '" + hash + "')";

    if (mysql_query(conn, query.c_str())) {
        cerr << "Error executing query: " << mysql_error(conn) << endl;
    }
}




void pass::sendMail(string& email,string& otp,string& username)
{
    int shift = 6; 
    string password = getPass();
    cout << "Pass : " << password << endl;
    if (password == "0")
    {
        return ;
    }
    // Decrypt the password
    string decrypted = caesarDecrypt(password, shift);
    const char* pass_app = decrypted.c_str();

    const char* mail_char = email.c_str();
    string MesUuid = create_uuid();
    string dateNow = getCurrentDateTime();
    string mail_str =
        "Date: " + dateNow + "\r\n"
        "To: " + email + "\r\n"
        "From: " FROM_MAIL "\r\n"
        "Message-ID: <" + MesUuid + "@"
        "graphic_era_university_erp_cell.org>\r\n"
        "Subject: Erp Password Reset OTP\r\n"
        "MIME-Version: 1.0\r\n"
        "Content-Type: multipart/alternative; boundary=\"boundary42\"\r\n"
        "\r\n"
        "--boundary42\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "Content-Transfer-Encoding: 7bit\r\n"
        "\r\n"
        "Graphic Era University - Erp Password Reset \r\n"
        "UserName: " + username + "\r\n"
        "\r\n"
        "Your Password Reset OTP is: " + otp + "\r\n"
        "\r\n"
        "--boundary42\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Transfer-Encoding: 7bit\r\n"
        "\r\n"
        "<html>\r\n"
        "<body>\r\n"
        "<p>Graphic Era University - Erp Password Reset</p>\r\n"
        "<p>UserName: " + username + "</p>\r\n"
        "<p>Your Password Reset OTP is: <strong style=\"font-size: 24px;\">" + otp + "</strong></p>\r\n"
        "</body>\r\n"
        "</html>\r\n"
        "\r\n"
        "--boundary42--\r\n";

    payload_text = mail_str.c_str();

    CURL* curl;
    CURLcode res = CURLE_OK;
    struct curl_slist* recipients = NULL;
    struct upload_status upload_ctx = { 0 };

    curl = curl_easy_init();
    if (curl) {
        /* Set username and password */
        curl_easy_setopt(curl, CURLOPT_USERNAME, "patbot2045@gmail.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, pass_app);

        /* This is the URL for your mailserver. Note the use of smtps:// rather
         * than smtp:// to request a SSL based connection. */
        curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");

        /* If you want to connect to a site who is not using a certificate that is
         * signed by one of the certs in the CA bundle you have, you can skip the
         * verification of the server's certificate. This makes the connection
         * A LOT LESS SECURE.
         *
         * If you have a CA cert for the server stored someplace else than in the
         * default bundle, then the CURLOPT_CAPATH option might come handy for
         * you. */

        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);


#ifdef SKIP_PEER_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

        /* If the site you are connecting to uses a different host name that what
         * they have mentioned in their server certificate's commonName (or
         * subjectAltName) fields, libcurl refuses to connect. You can skip this
         * check, but it makes the connection insecure. */
#ifdef SKIP_HOSTNAME_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

        /* Note that this option is not strictly required, omitting it results in
         * libcurl sending the MAIL FROM command with empty sender data. All
         * autoresponses should have an empty reverse-path, and should be directed
         * to the address in the reverse-path which triggered them. Otherwise,
         * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
         * details.
         */
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM_MAIL);

        /* Add two recipients, in this particular case they correspond to the
         * To: and Cc: addressees in the header, but they could be any kind of
         * recipient. */
        recipients = curl_slist_append(recipients, mail_char);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        /* We are using a callback function to specify the payload (the headers and
         * body of the message). You could just use the CURLOPT_READDATA option to
         * specify a FILE pointer to read from. */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        /* Since the traffic is encrypted, it is useful to turn on debug
         * information within libcurl to see what is happening during the
         * transfer */
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        /* Send the message */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

        /* Free the list of recipients */
        curl_slist_free_all(recipients);

        /* Always cleanup */
        curl_easy_cleanup(curl);
    }

}


void pass::changePasswordKnown(Database& db,string& username,string& userType)
{
    MYSQL* conn = db.getConnection();
    if (username == "-1")
    {
        username="";
        cout << "Enter the Username: ";
        cin >> username;
        if (!(checkUserExists(db, username)))
        {
            cout << "User doesn't exists!\n";
            return;
        }
        userType = "";
        cout << "Enter the User Type: [ student , staff , admin ]" << endl;
        cin >> userType;
        if (!(checkExistsUser(db, username, userType)))
        {
            cout << "Wrong UserType!!" << endl;
            return;
        }
    }
    
    string old_pass="";
    cout << "Enter the Old Password: ";
  
    //Making the password not Visible
    char ch;
    while ((ch = _getch()) != 13) { // 13 is the ASCII code for Enter
        if (ch == 8) { // 8 is the ASCII code for Backspace
            if (!old_pass.empty())
            {
                old_pass.pop_back();
                cout << "\b \b"; // Erase the last character on the console
            }
        }
        else {
            old_pass.push_back(ch);
            cout << '*'; // Print * instead of the actual character
        }
    }
    cout << "\n";

    string query = "SELECT hash, salt FROM holy_salt WHERE username = '" + username + "'";

    if (mysql_query(conn, query.c_str()))
    {
        cerr << "Error executing query: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error storing result: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_ROW row;
    string salt = "";
    string og_hash = "";
    while ((row = mysql_fetch_row(res)))
    {
        og_hash = row[0] ? row[0] : "";
        salt = row[1] ? row[1] : "";
    }

    mysql_free_result(res);

    old_pass += salt;
    stringstream hash1;
    sha256(old_pass, hash1);
    string new_pass = hash1.str();

    if (og_hash == new_pass)
    {
        string pass="";
        cout << "---Password Matched---\n";
        cout << "Enter the New Password: ";
     
        //Making the password not Visible
        char ch;
        while ((ch = _getch()) != 13) { // 13 is the ASCII code for Enter
            if (ch == 8) { // 8 is the ASCII code for Backspace
                if (!pass.empty())
                {
                    pass.pop_back();
                    cout << "\b \b"; // Erase the last character on the console
                }
            }
            else {
                pass.push_back(ch);
                cout << '*'; // Print * instead of the actual character
            }
        }
        cout << "\n";

        string salt1 = generateSalt();
        stringstream hash1;
        pass += salt1;
        sha256(pass, hash1);
        pass = hash1.str();

        string query = "UPDATE holy_salt SET hash = '" + pass + "', salt = '" + salt1 + "' WHERE username = '" + username + "'";
        string query2 = "UPDATE "+ userType +" SET password = '" + pass + "' WHERE username = '" + username + "'";

        if ((mysql_query(conn, query.c_str())) )
        {
            cerr << "Error executing query: " << mysql_error(conn) << endl;
        }
        else
        {
            if ((mysql_query(conn, query2.c_str())))
            {
                cerr << "Error executing query: " << mysql_error(conn) << endl;
            }
            else
            {
                cout << "---Password Sucessfullly Updated---" << endl;
            }
        }
    }
    else
    {
        cout << "Password Doesn't Match with Old Password!!\n";
        return;
    }
}

void pass::changePasswordOtp(Database& db, string& username, string& userType)
{
    MYSQL* conn = db.getConnection();
    string query = "SELECT mail FROM " + userType + " WHERE username = '" + username + "'";

    if (mysql_query(conn, query.c_str()))
    {
        cerr << "Error executing query: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr)
    {
        cerr << "Error storing result: " << mysql_error(conn) << endl;
        return;
    }

    string email;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)))
    {
        email = row[0] ? row[0] : "";
    }

    mysql_free_result(res);

    string otp = generateOTP();
    sendMail(email, otp, username);

    string user_otp;
    cout << "Enter the OTP: ";
    cin >> user_otp;

    if (user_otp == otp)
    {
        string pass="";
        cout << "---OTP Matched---\n";
        cout << "Enter the New Password: ";

        //Making the password not Visible
        char ch;
        while ((ch = _getch()) != 13) { // 13 is the ASCII code for Enter
            if (ch == 8) { // 8 is the ASCII code for Backspace
                if (!pass.empty())
                {
                    pass.pop_back();
                    cout << "\b \b"; // Erase the last character on the console
                }
            }
            else {
                pass.push_back(ch);
                cout << '*'; // Print * instead of the actual character
            }
        }
        cout << "\n";

        string salt1 = generateSalt();
        stringstream hash1;
        pass += salt1;
        sha256(pass, hash1);
        pass = hash1.str();

        string query = "UPDATE holy_salt SET hash = '" + pass + "', salt = '" + salt1 + "' WHERE username = '" + username + "'";
        string query2 = "UPDATE " + userType + " SET password = '" + pass + "' WHERE username = '" + username + "'";

        if ((mysql_query(conn, query.c_str())))
        {
            cerr << "Error executing query: " << mysql_error(conn) << endl;
        }
        else
        {
            if ((mysql_query(conn, query2.c_str())))
            {
                cerr << "Error executing query: " << mysql_error(conn) << endl;
            }
            else
            {
                cout << "---Password Sucessfullly Updated---" << endl;
            }
        }
    }
    else
    {
        cout << "Wrong OTP!!\n";
        return;
    }

}
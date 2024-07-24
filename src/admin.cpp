#include <iostream>
#include <string>
#include<mysql.h>
#include<vector>
#include <iomanip>
#include<algorithm>
#include<map>

#include <sstream>
#include<random>
#include <conio.h>

#include "database.h"
#include "student.h"
#include "staff.h"
#include "admin.h"

using namespace std;

Admin::Admin::Admin() : username(""), password(""), realName(""), dob(""), address(""), joiningDate(""), salary(0.0) 
{

}


// Getter functions to access private members
Admin::Admin(const string& username, const string& password, const string& realName, const string& dob,
    const string& address, const string& joiningDate, double salary)
    : username(username), password(password), realName(realName), dob(dob), address(address), joiningDate(joiningDate), salary(salary) {}

const string& Admin::getUsername() const { return username; }
const string& Admin::getPassword() const { return password; }
const string& Admin::getRealName() const { return realName; }
const string& Admin::getDob() const { return dob; }
const string& Admin::getAddress() const { return address; }
const string& Admin::getJoiningDate() const { return joiningDate; }
double Admin::getSalary() const { return salary; }


bool Admin::checkUserExists(Database& db, const string& username) 
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




void Admin::resetCurCgpa(Database& db, int studentId, const string& branchCode, int semester) 
{
    MYSQL* conn = db.getConnection();
    string query = "UPDATE student SET cgpa = 0.00 WHERE student_id = " + to_string(studentId)
        + " AND branch_code = '" + branchCode + "' AND semester = " + to_string(semester);

    if (mysql_query(conn, query.c_str())) {
        cerr << "Error executing query: " << mysql_error(conn) << endl;
    }
    else {
        cout << "Successfully reset CGPA for student_id: " << studentId << ", branch_code: " << branchCode
            << ", semester: " << semester << endl;
    }
}


void Admin::updateSemester(Database& db, int student_id, int semester)
{
    MYSQL* conn = db.getConnection();
    // Construct the SQL query to update the semester in the student table
    string updateQuery = "UPDATE student SET semester = " + to_string(semester) + " WHERE student_id = " + to_string(student_id) + ";";

    // Execute the update query
    if (mysql_query(conn, updateQuery.c_str())) {
        cerr << "Query to update student semester failed: " << mysql_error(conn) << endl;
        return;
    }

    cout << "Semester updated successfully for student ID: " << student_id << endl;
}

void updateFees(Database& db, int student_id, const string& branch_code, int semester) 
{
    MYSQL* conn = db.getConnection();
    // Construct the SQL query to retrieve fees from the fees table
    string query = "SELECT fees FROM fees WHERE branch_code = '" + branch_code + "' AND semester = " + to_string(semester) + ";";

    // Execute the query to get fees
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query to retrieve fees failed: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (res == NULL) {
        cerr << "mysql_store_result() failed: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == NULL) {
        cerr << "No matching fees found" << endl;
        mysql_free_result(res);
        return;
    }

    // Get the fees value
    double fees = stod(row[0]);
    mysql_free_result(res);

    // Construct the SQL query to update the fees in the student table
    string updateQuery = "UPDATE student SET fees = fees + " + to_string(fees) + " WHERE student_id = " + to_string(student_id) + ";";

    // Execute the update query
    if (mysql_query(conn, updateQuery.c_str())) {
        cerr << "Query to update student fees failed: " << mysql_error(conn) << endl;
        return;
    }

    cout << "Fees updated successfully for student ID: " << student_id << endl;
}

bool Admin::checkFeesPaid(Database& db, int student_id) 
{
    MYSQL* conn = db.getConnection();
    // Construct the SQL query
    string query = "SELECT fees FROM student WHERE student_id = " + to_string(student_id) + ";";

    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return false;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (res == NULL) {
        cerr << "mysql_store_result() failed: " << mysql_error(conn) << endl;
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == NULL) {
        cerr << "No such student found" << endl;
        mysql_free_result(res);
        return false;
    }

    // Get the fees value
    double fees = stod(row[0]);
    mysql_free_result(res);

    // Check if fees are 0 or less
    return fees <= 0;
}

void Admin::deleteStudent(Database& db, int student_id)
{
    MYSQL* conn = db.getConnection();
    // Construct the SQL query
    string query = "DELETE FROM student WHERE student_id = " + to_string(student_id) + ";";

    // Execute the query
    if (mysql_query(conn, query.c_str())) 
    {
        cerr << "Query failed: " << mysql_error(conn) << endl;
    }
}

string Admin::getStaffSection(Database& db, int staffId, const string& subCode) 
{
    MYSQL* conn = db.getConnection();
    string query = "SELECT section FROM staffid_subcode WHERE staff_id = " + to_string(staffId)
        + " AND sub_code = '" + subCode + "'";

    if (mysql_query(conn, query.c_str())) 
    {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return "";
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        cerr << "Failed to store result: " << mysql_error(conn) << endl;
        return "";
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    string section = "";
    if (row) {
        section = row[0];
    }

    mysql_free_result(res);
    return section;
}


string Admin::getBranchCodeFromSubCode(Database& db, const string& subCode)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    string branchCode;
    MYSQL* conn = db.getConnection();
    // Construct the query
    string query = "SELECT branch_code FROM branchcode_subcode_sem WHERE sub_code = '" + subCode + "'";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return "";
    }
    res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Failed to retrieve result: " << mysql_error(conn) << endl;
        return "";
    }
    // Check if the result set is empty
    if (mysql_num_rows(res) == 0) {
        cout << "No record found with sub_code " << subCode << endl;
        mysql_free_result(res);
        return "";
    }
    // Fetch the branch_code
    row = mysql_fetch_row(res);
    if (row != nullptr) {
        branchCode = row[0];
    }
    // Clean up
    mysql_free_result(res);
    return branchCode;
}
int Admin::getTotalAttendance(Database& db, string& branchCode, int semester, string& section, string subCode)
{
    MYSQL* conn = db.getConnection();
    // Check if the connection is valid
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return -1; // Indicate an error with a negative number
    }
    // Construct the query to get the total value
    string query = "SELECT total FROM total_attendance WHERE branch_code = '" + branchCode
        + "' AND semester = " + to_string(semester)
        + " AND section = '" + section
        + "' AND sub_code = '" + subCode + "'";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in fetching total attendance: " << mysql_error(conn) << endl;
        return -1; // Indicate an error with a negative number
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return -1; // Indicate an error with a negative number
    }
    // Fetch the row
    MYSQL_ROW row = mysql_fetch_row(res);
    int total = (row && row[0]) ? atoi(row[0]) : 0; // Convert the result to an integer
    // Free the result
    mysql_free_result(res);
    return total;
}

int Admin::getStaffIdByUsername(Database& db, string& username)
{
    MYSQL* conn = db.getConnection();
    // Check if the connection is valid
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return -1; // Indicate an error with a negative number
    }
    // Construct the query to get the staff_id based on the username
    string query = "SELECT staff_id FROM staff WHERE username = '" + username + "'";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in fetching staff_id: " << mysql_error(conn) << endl;
        return -1; // Indicate an error with a negative number
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return -1; // Indicate an error with a negative number
    }
    // Fetch the row
    MYSQL_ROW row = mysql_fetch_row(res);
    int staffId = (row && row[0]) ? atoi(row[0]) : -1; // Convert the result to an integer
    // Free the result
    mysql_free_result(res);
    return staffId;
}
int Admin::getSemesterFromSubCode(Database& db, const string subCode)
{
    MYSQL* conn = db.getConnection();
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return -1; // Return -1 to indicate an error
    }
    // Construct the query to get the semester for the given sub_code
    string query = "SELECT semester FROM branchcode_subcode_sem WHERE sub_code = '" + subCode + "'";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in fetching semester: " << mysql_error(conn) << endl;
        return -1;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return -1;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    int semester = (row && row[0]) ? atoi(row[0]) : -1;
    mysql_free_result(res);
    return semester;
}

bool Admin::checkExistsUser(Database& db, string& username, string userType)
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
string Admin::getCourseFromCourseId(Database& db, string& courseId)
{
    MYSQL* conn = db.getConnection();
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return "";
    }
    // Construct the query to get the course name for the given courseId
    string query = "SELECT course FROM course_main WHERE course_id = '" + courseId + "'";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in fetching course name: " << mysql_error(conn) << endl;
        return "";
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return "";
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    string courseName = (row && row[0]) ? row[0] : "";
    mysql_free_result(res);
    return courseName;
}
string Admin::getBranchFromBranchId(Database& db, string& branchId) {
    MYSQL* conn = db.getConnection();
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return "";
    }
    // Construct the query to get the branch name for the given branchId
    string query = "SELECT branch FROM courseid_branchcode WHERE branch_code = '" + branchId + "'";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in fetching branch name: " << mysql_error(conn) << endl;
        return "";
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return "";
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    string branchName = (row && row[0]) ? row[0] : "";
    mysql_free_result(res);
    return branchName;
}
int Admin::getStudentId(Database& db, string& username) {
    MYSQL* conn = db.getConnection();
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return -1;
    }
    // Construct the query to get the student_id for the given username
    string query = "SELECT student_id FROM student WHERE username = '" + username + "'";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in fetching student_id: " << mysql_error(conn) << endl;
        return -1;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return -1;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    int studentId = (row && row[0]) ? atoi(row[0]) : -1;
    mysql_free_result(res);
    return studentId;
}
bool Admin::checkSectionExists(Database& db, string branchCode, int semester, string& section)
{
    MYSQL* conn = db.getConnection();
    // Construct the query to check if the branch_code, section, and semester combination exists
    string checkQuery = "SELECT COUNT(*) FROM student "
        "WHERE branch_code = '" + branchCode + "' AND section = '" + section + "' AND semester = " + to_string(semester);
    if (mysql_query(conn, checkQuery.c_str())) {
        cerr << "Error in checking section existence: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    bool exists = atoi(row[0]) > 0;
    mysql_free_result(res);
    return exists;
}
bool Admin::check_exist(Database& db, string& username, string& userType)
{
    MYSQL* conn = db.getConnection();
    string query = "SELECT * FROM " + userType + " WHERE username = '" + username + "'";
    if (mysql_query(conn, query.c_str()))
    {
        cerr << "Error in executing query: " << mysql_error(conn) << endl;
        exit(1);
    }
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        cerr << "Error in fetching result: " << mysql_error(conn) << endl;
        exit(1);
    }
    int count = int(mysql_num_rows(result));
    mysql_free_result(result);
    if (count > 0)
    {
        cout << "\nUser with username " << username << " exists in " << userType << " table. Use another Username!" << endl;
        return true;
    }
    else
    {
        return false;
    }
}
bool Admin::checkStudentExists(Database& db, int studentId)
{
    MYSQL* conn = db.getConnection();
    // Query to check if studentId exists in student table
    string selectQuery = "SELECT COUNT(*) FROM student WHERE student_id = " + to_string(studentId);
    if (mysql_query(conn, selectQuery.c_str())) {
        cerr << "Error in checking student existence: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    int rowCount = atoi(row[0]);
    mysql_free_result(res);
    return rowCount > 0; // Return true if rowCount is greater than 0, indicating that the student exists
}
bool Admin::checkSubExists(Database& db, string& subCode)
{
    MYSQL* conn = db.getConnection();
    // Query to check if subCode exists in subcode_subname table
    string selectQuery = "SELECT COUNT(*) FROM subcode_subname WHERE sub_code = '" + subCode + "'";
    if (mysql_query(conn, selectQuery.c_str()))
    {
        cerr << "Error in checking subject existence: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    int rowCount = atoi(row[0]);
    mysql_free_result(res);
    return rowCount > 0; // Return true if rowCount is greater than 0, indicating that the subject exists
}
bool Admin::checkSemesterExists(Database& db, string branchCode, int semester)
{
    MYSQL* conn = db.getConnection();
    // Construct the query to check if the branch_code and semester combination exists
    string checkQuery = "SELECT COUNT(*) FROM branchcode_subcode_sem "
        "WHERE branch_code = '" + branchCode + "' AND semester = " + to_string(semester);
    if (mysql_query(conn, checkQuery.c_str())) {
        cerr << "Error in checking semester existence: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    bool exists = atoi(row[0]) > 0;
    mysql_free_result(res);
    return exists;
}

bool Admin::checkCourseExists(Database& db, string& CourseId)
{
    MYSQL* conn = db.getConnection();
    // Query to check if subCode exists in courseid_branchcode table
    string selectQuery = "SELECT COUNT(*) FROM course_main WHERE course_id = '" + CourseId + "'";
    if (mysql_query(conn, selectQuery.c_str()))
    {
        cerr << "Error in checking subject existence: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr)
    {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    int rowCount = atoi(row[0]);
    mysql_free_result(res);
    return rowCount > 0; // Return true if rowCount is greater than 0, indicating that the subject exists
}

bool Admin::checkBranchExists(Database& db, string& BranchCode)
{
    MYSQL* conn = db.getConnection();
    // Query to check if subCode exists in courseid_branchcode table
    string selectQuery = "SELECT COUNT(*) FROM courseid_branchcode WHERE branch_code = '" + BranchCode + "'";
    if (mysql_query(conn, selectQuery.c_str()))
    {
        cerr << "Error in checking subject existence: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr)
    {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    int rowCount = atoi(row[0]);
    mysql_free_result(res);
    return rowCount > 0; // Return true if rowCount is greater than 0, indicating that the subject exists
}
void Admin::viewAdminData(Database& db)
{
    MYSQL* conn = db.getConnection();
    string query = "SELECT * FROM admin";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in viewing admin data: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;

    cout << left << setw(15) << "Username"
        << setw(30) << "Real Name"
        << setw(15) << "DOB"
        << setw(50) << "Address"
        << setw(20) << "Joining Date" << endl;
    cout << string(100, '-') << endl;

    while ((row = mysql_fetch_row(res))) {
        cout << left << setw(15) << row[0]
            << setw(30) << row[2]
            << setw(15) << row[3]
            << setw(50) << row[4]
            << setw(20) << row[5] << endl;
    }

    mysql_free_result(res);
}
void Admin::viewAdminOwnData(string& username, Database& db)
{
    MYSQL* conn = db.getConnection();
    string query = "SELECT * FROM admin WHERE username='" + username + "'";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in selecting admin: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    if (!(row = mysql_fetch_row(res))) {
        cerr << "No admin found with username: " << username << endl;
        mysql_free_result(res);
        return;
    }
    cout << left << setw(15) << "Username"
        << left << setw(30) << "Real Name"
        << left << setw(12) << "DOB"
        << left << setw(50) << "Address"
        << left << setw(15) << "Joining Date"
        << left << setw(10) << "Salary"
        << endl;

    cout << setfill('-') << setw(100) << "-" << setfill(' ') << endl;

    cout << left << setw(15) << row[0]
        << left << setw(30) << row[2]
        << left << setw(12) << row[3]
        << left << setw(50) << row[4]
        << left << setw(15) << row[5]
        << left << setw(10) << row[6]
        << endl;
    
    mysql_free_result(res);
}
void Admin::viewStudentData(Database& db)
{
    MYSQL* conn = db.getConnection();
    string query = "SELECT * FROM student";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in viewing student data: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);

    MYSQL_ROW row;
    cout << left << setw(15) << "Username"
        << left << setw(10) << "Roll No"
        << left << setw(30) << "Real Name"
        << left << setw(40) << "Address"
        << left << setw(25) << "Course"
        << left << setw(30) << "Branch"
        << left << setw(10) << "Section"
        << left << setw(10) << "Student Id"
        << left << setw(10) << "Semester"
        << left << setw(10) << "Fees"
        << left << setw(10) << "Course ID"
        << left << setw(15) << "Branch Code"
        << left << setw(10) << "CGPA" << endl;

    cout << setfill('-') << setw(140) << "-" << setfill(' ') << endl;

    while ((row = mysql_fetch_row(res))) {
        cout << left << setw(15) << row[0]
            << left << setw(10) << row[2]
            << left << setw(30) << row[3]
            << left << setw(40) << row[4]
            << left << setw(25) << row[5]
            << left << setw(30) << row[6]
            << left << setw(10) << row[7]
            << left << setw(10) << row[8]
            << left << setw(10) << row[12]
            << left << setw(10) << row[9]
            << left << setw(10) << row[10]
            << left << setw(15) << row[11]
            << left << setw(10) << row[13] << endl;
    }

    mysql_free_result(res);
}


void Admin::showStaffSubjects(Database& db, string staff_id_str)
{
    MYSQL* conn = db.getConnection();
    string subcode_query = "SELECT * FROM staffid_subcode WHERE staff_id = " + staff_id_str;
    if (mysql_query(conn, subcode_query.c_str())) 
    {
        cerr << "Error in querying subcodes: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* subcode_res = mysql_store_result(conn);
    MYSQL_ROW subcode_row;
    while ((subcode_row = mysql_fetch_row(subcode_res))) 
    {
        // Retrieve subcode
        string sub_code = subcode_row[1];
        int staffId = stoi(staff_id_str);
        string section = subcode_row[2];
        // Query subcode_subname table to get subname
        string subname_query = "SELECT sub_code, sub_name FROM subcode_subname WHERE sub_code = '" + sub_code + "'";
        if (mysql_query(conn, subname_query.c_str())) 
        {
            cerr << "Error in querying subname: " << mysql_error(conn) << endl;
            continue;
        }
        MYSQL_RES* subname_res = mysql_store_result(conn);
        MYSQL_ROW subname_row;
        if ((subname_row = mysql_fetch_row(subname_res))) {
            // Print subcode and subname
            cout << "Subcode: " << subname_row[0] << " , Subname: " << subname_row[1] << " , Section: "<< section << endl;
        }
        mysql_free_result(subname_res);
    }
    mysql_free_result(subcode_res);
}
void Admin::viewStaffData(Database& db)
{
    MYSQL* conn = db.getConnection();
    string query = "SELECT * FROM staff";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in viewing staff data: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(res))) 
    {
        cout << left << setw(15) << "Username"
            << left << setw(30) << "Real Name"
            << left << setw(12) << "DOB"
            << left << setw(25) << "Branch"
            << left << setw(10) << "Salary"
            << left << setw(45) << "Address"
            << left << setw(20) << "Joining Date"
            << left << setw(20) << "Number of Subjects"
            << left << setw(10) << "Staff Id"
            << left << setw(15) << "Branch Code" << endl;

        cout << setfill('-') << setw(150) << "-" << setfill(' ') << endl;

        cout << left << setw(15) << row[0]
            << left << setw(30) << row[2]
            << left << setw(12) << row[3]
            << left << setw(25) << row[4]
            << left << setw(10) << row[5]
            << left << setw(45) << row[6]
            << left << setw(20) << row[7]
            << left << setw(20) << row[8]
            << left << setw(10) << row[9]
            << left << setw(15) << row[10] << endl;

        showStaffSubjects(db, row[9]);
        cout << "\n";
    }

    mysql_free_result(res);
}
void Admin::viewStaffData_Branch(Database& db)
{
    string branch_code;
    cout << "Enter the Branch Code " << endl;
    {
        cin >> branch_code;
    }
    MYSQL* conn = db.getConnection();
    // Formulate the query to find staff with the given branch_code
    string query = "SELECT * FROM staff WHERE branch_code = '" + branch_code + "'";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in querying staff by branch code: " << mysql_error(conn) << endl;
        return;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == NULL) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row;
    // Iterate through the result set and print staff details
 
    // Print data rows
    while ((row = mysql_fetch_row(res))) 
    {
        // Print header row
        cout << left << setw(20) << "Username"
            << left << setw(30) << "Real Name"
            << left << setw(20) << "DOB"
            << left << setw(25) << "Branch"
            << left << setw(20) << "Salary"
            << left << setw(40) << "Address"
            << left << setw(20) << "Joining Date"
            << left << setw(20) << "Number of Subjects"
            << left << setw(15) << "Staff Id"
            << left << setw(15) << "Branch Code" << endl;

        cout << setfill('-') << setw(120) << "-" << setfill(' ') << endl;

        cout << left << setw(20) << row[0]
            << left << setw(30) << row[2]
            << left << setw(20) << row[3]
            << left << setw(25) << row[4]
            << left << setw(20) << row[5]
            << left << setw(40) << row[6]
            << left << setw(20) << row[7]
            << left << setw(20) << row[8]
            << left << setw(15) << row[9]
            << left << setw(15) << row[10] << endl;
        cout << "\n";
        showStaffSubjects(db, row[9]);
        cout << "\n";
    }
    // Free the result
    mysql_free_result(res);
}
void Admin::viewStaffOwnData(Database& db)
{
    string staff_id;
    cout << "Enter the staff_id:  ";
    cin >> staff_id;
    MYSQL* conn = db.getConnection();
    string query = "SELECT * FROM staff WHERE staff_id='" + staff_id + "'";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in selecting staff: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    if (!(row = mysql_fetch_row(res))) {
        cerr << "No staff found with Staff_id: " << staff_id << endl;
        mysql_free_result(res);
        return;
    }

    // Print header row
    cout << left << setw(20) << "Username"
        << left << setw(30) << "Real Name"
        << left << setw(20) << "DOB"
        << left << setw(30) << "Branch"
        << left << setw(20) << "Salary"
        << left << setw(40) << "Address"
        << left << setw(20) << "Joining Date"
        << left << setw(20) << "Number of Subjects"
        << left << setw(15) << "Staff Id"
        << left << setw(15) << "Branch Code" << endl;

    cout << string(210, '-') << endl; // Line separator

    // Print data row
    cout << left << setw(20) << row[0]
        << left << setw(30) << row[2]
        << left << setw(20) << row[3]
        << left << setw(30) << row[4]
        << left << setw(20) << row[5]
        << left << setw(40) << row[6]
        << left << setw(20) << row[7]
        << left << setw(20) << row[8]
        << left << setw(15) << row[9]
        << left << setw(15) << row[10] << endl;
    cout << "\n";
    showStaffSubjects(db, row[9]); // Assuming showStaffSubjects prints the subjects for the given staff ID
    cout << "\n";

    mysql_free_result(res);
}
void Admin::viewStudentDataOn(Database& db, string field)
{
    string value, query;
    if (field == "section")
    {
        string branchcode, section;
        int semester;
        cout << "Enter the branchcode: ";
        cin >> branchcode;
        if (!(checkBranchExists(db, branchcode)))
        {
            cout << "Branch Doesn't Exists!!\n";
            return;
        }
        cout << "Enter the semester: ";
        cin >> semester;
        if (!checkSemesterExists(db, branchcode, semester))
        {
            cout << "Semester of this branch doesn't exists!!" << endl;
            return;
        }
        cout << "Enter the section: ";
        cin >> section;
        if (!checkSectionExists(db, branchcode, semester, section))
        {
            cout << "This Section doesn't Exists!!\n";
            return;
        }
        query = "SELECT * FROM student WHERE branch_code = '" + branchcode + "' and semester = " + to_string(semester) + " and section = '" + section + "'";
    }
    else
    {
        cout << "Enter the " << field << ": ";
        cin >> value;
        query = "SELECT * FROM student WHERE " + field + " = '" + value + "'";
    }
    MYSQL* conn = db.getConnection();
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in querying student data: " << mysql_error(conn) << endl;
        return;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == NULL) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row;
    // Iterate through the result set and print student details
    
    cout << left << setw(20) << "Username"
        << left << setw(10) << "Roll No"
        << left << setw(30) << "Real Name"
        << left << setw(40) << "Address"
        << left << setw(30) << "Course"
        << left << setw(30) << "Branch"
        << left << setw(10) << "Section"
        << left << setw(15) << "Student Id"
        << left << setw(10) << "Fees"
        << left << setw(15) << "Course ID"
        << left << setw(15) << "Branch Code"
        << left << setw(10) << "Semester"
        << left << setw(10) << "CGPA" << endl;

    cout << string(210, '-') << endl; // Line separator

    while ((row = mysql_fetch_row(res))) 
    {
        cout << left << setw(20) << row[0]
            << left << setw(10) << row[2]
            << left << setw(30) << row[3]
            << left << setw(40) << row[4]
            << left << setw(30) << row[5]
            << left << setw(30) << row[6]
            << left << setw(10) << row[7]
            << left << setw(15) << row[8]
            << left << setw(10) << row[9]
            << left << setw(15) << row[10]
            << left << setw(15) << row[11]
            << left << setw(10) << row[12]
            << left << setw(10) << row[13] << endl;
    }

    // Free the result
    mysql_free_result(res);
}
void Admin::UpdateStaffOtherField(Database& db, string& username)
{
    MYSQL* conn = db.getConnection();
    // Retrieve staff_id corresponding to the given username
    string query = "SELECT staff_id FROM staff WHERE username = '" + username + "'";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error retrieving staff_id: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == nullptr) {
        cerr << "No staff found for username: " << username << endl;
        mysql_free_result(res);
        return;
    }
    int staff_id = atoi(row[0]);
    mysql_free_result(res);
    // Ask the user what field they want to change
    cout << "What field do you want to change? (branch/subject)" << endl;
    string field;
    cin >> field;
    // Delete all records from staffid_subcode table where staff_id matches
    query = "DELETE FROM staffid_subcode WHERE staff_id = " + to_string(staff_id);
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error deleting records from staffid_subcode: " << mysql_error(conn) << endl;
        return;
    }
    // Update the necessary field in the staff table
    if (field == "branch")
    {
        cout << "\nEnter new branch_code: ";
        string branch_code;
        cin >> branch_code;
        // Check if branch_code exists in courseid_branchcode table
        query = "SELECT branch FROM courseid_branchcode WHERE branch_code = '" + branch_code + "'";
        if (mysql_query(conn, query.c_str())) {
            cerr << "Error checking branch_code: " << mysql_error(conn) << endl;
            return;
        }
        res = mysql_store_result(conn);
        if (res == nullptr || mysql_num_rows(res) == 0)
        {
            cerr << "Invalid branch_code: " << branch_code << endl;
            mysql_free_result(res);
            return;
        }
        row = mysql_fetch_row(res);
        string branch = row[0];
        mysql_free_result(res);
        // Update branch_code and branch in staff table
        query = "UPDATE staff SET branch_code = '" + branch_code + "', branch = '" + branch + "' WHERE staff_id = " + to_string(staff_id);
        if (mysql_query(conn, query.c_str())) {
            cerr << "Error updating staff table: " << mysql_error(conn) << endl;
            return;
        }
        cout << "Changed sucessfully!\n";
    }
    else if (field != "subject")
    {
        cout << "Invalid field choice" << endl;
        return;
    }
    // Ask for number of subjects
    cout << "Enter number of subjects: ";
    int num_of_subjects;
    cin >> num_of_subjects;
    // Vector to store subject codes
    vector<string> subject_codes(num_of_subjects);
    vector<string>sections(num_of_subjects);
    // Get the subject codes from the user and check if they exist
    for (int i = 0; i < num_of_subjects; ++i) {
        cout << "Enter subject code " << i + 1 << ": ";
        cin >> subject_codes[i];
        if (!(checkSubExists(db, subject_codes[i])))
        {
            cout << "This Subject code Subject Doesn't Exists!!\n";
            return;
        }
        cout << "Enter the section for subcode: " + subject_codes[i] + ": ";
        cin >> sections[i];
        string branchCode = getBranchCodeFromSubCode(db, subject_codes[i]);
        int semester = getSemesterFromSubCode(db, subject_codes[i]);
        if (!(checkSectionExists(db, branchCode, semester, sections[i])))
        {
            cout << "This Section doesn't Exists!!\n";
            return;
        }
    }
    int i = 0;
    // Insert staff_id and subject codes into staffid_subcode table
    for (const string& sub_code : subject_codes)
    {
        query = "INSERT INTO staffid_subcode (staff_id, sub_code, section) VALUES (" + to_string(staff_id) + ", '" + sub_code + "','" + sections[i] + "')";
        i += 1;
        if (mysql_query(conn, query.c_str())) {
            cerr << "Error inserting into staffid_subcode: " << mysql_error(conn) << endl;
            return;
        }
    }
    // Update num_of_subjects in staff table
    query = "UPDATE staff SET num_of_subjects = " + to_string(num_of_subjects) + " WHERE staff_id = " + to_string(staff_id);
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error updating num_of_subjects: " << mysql_error(conn) << endl;
        return;
    }
}
void Admin::UpdateData(Database& db, string& username, string userType)
{
    MYSQL* conn = db.getConnection();
    string query;
    if (userType == "admin")
    {
        // Ask the user how many fields they want to change
        int numFields;
        cout << "How many fields do you want to change? ";
        cin >> numFields;
        cout << "Fields : username | password | real_name | dob | address | joining_date | salary" << endl;
        // Vector to store the field names
        vector<string> fields(numFields);
        vector<string> values(numFields);
        string tempUserName,extra;
        // Get the field names and new values from the user
        for (int i = 0; i < numFields; ++i) 
        {
            cout << "Enter field name: ";
            cin >> fields[i];
            cin.ignore();
            cout << "Enter new value for " << fields[i] << ": ";
            if (fields[i] == "username")
            {
                cin >> tempUserName;
                if (username == tempUserName)
                {
                    values[i] = tempUserName;
                }
                else
                {
                    while (checkExistsUser(db, tempUserName, userType))
                    {
                        extra = username;
                        cout << "\nUser with this UserName already Exists! Enter UserName again: ";
                        cin >> tempUserName;
                        if (tempUserName == username)
                        {
                            break;
                        }
                    }
                    values[i] = tempUserName;
                }
            }
            else
            {
                getline(cin, values[i]);
            }
        }

        // Formulate the update query
        string query = "UPDATE admin SET ";
        for (int i = 0; i < numFields; ++i) {
            query += fields[i] + " = '" + values[i] + "'";
            if (i < numFields - 1) {
                query += ", ";
            }
        }
        query += " WHERE username = '" + username + "'";
        // Execute the update query
        if (mysql_query(conn, query.c_str())) {
            cerr << "Error updating admin data: " << mysql_error(conn) << endl;
            return;
        }
        cout << "Admin data updated successfully." << endl;
    }
    else if (userType == "staff")
    {
        string chi, ch2;
        cout << "Is the update field include branch , or subjects | y or n |" << endl;
        cin >> chi;
        if (chi == "y")
        {
            UpdateStaffOtherField(db, username);
            cout << "Want to change other fields too? |y or n|" << endl;
            {
                cin >> ch2;
            }
            if (ch2 == "n" || ch2!="y")
            {
                return;
            }
        }
        // Ask the user how many fields they want to change
        int numFields;
        cout << "How many fields do you want to change? ";
        cin >> numFields;
        // Vector to store the field names and values
        vector<string> fields(numFields);
        vector<string> values(numFields);
        string tempUserName,extra;
        vector<string> otherfield{ "branch","branch_code","num_of_subjects" };
        cout << "Field Name: username | password | real_name | dob | salary | address " << endl;
        // Get the field names and new values from the user
        for (int i = 0; i < numFields; ++i)
        {
            cout << "Enter field name: ";
            cin >> fields[i];
            cin.ignore();
            cout << "Enter new value for " << fields[i] << ": ";
            if (fields[i] == "username")
            {
                cin >> tempUserName;
                if (username == tempUserName)
                {
                    values[i] = tempUserName;
                }
                else
                {
                    while (checkExistsUser(db, tempUserName, userType))
                    {
                        extra = username;
                        cout << "\nUser with this UserName already Exists! Enter UserName again: ";
                        cin >> tempUserName;
                        if (tempUserName == username)
                        {
                            break;
                        }
                    }
                    values[i] = tempUserName;
                }
            }
            else
            {
                getline(cin, values[i]);
            }
        }
        // Formulate the update query
        string query = "UPDATE staff SET ";
        for (int i = 0; i < numFields; ++i) {
            query += fields[i] + " = '" + values[i] + "'";
            if (i < numFields - 1) {
                query += ", ";
            }
        }
        query += " WHERE username = '" + username + "'";

        // Execute the update query
        if (mysql_query(conn, query.c_str())) {
            cerr << "Error updating staff data: " << mysql_error(conn) << endl;
            return;
        }
        cout << "Staff data updated successfully." << endl;
    }
    else if (userType == "student")
    {
        // Ask the user how many fields they want to change
        cout << "Fields: username | password | roll_no | real_name | address | course | branch | section | student_id | fees | course_id | branch_code | semester | cgpa" << endl;
        int numFields;
        cout << "How many fields do you want to change? ";
        cin >> numFields;
        string extra;
        // Vector to store the field names and values
        vector<string> fields(numFields);
        vector<string> values(numFields);
        string tempUserName;
        // Get the field names and new values from the user
        for (int i = 0; i < numFields; ++i)
        {
            cout << "Enter field name: ";
            cin >> fields[i];
            cin.ignore();
            cout << "Enter new value for " << fields[i] << ": ";
            if (fields[i] == "username")
            {
                cin >> tempUserName;
                if (username == tempUserName)
                {
                    values[i] = tempUserName;
                }
                else
                {
                    while (checkExistsUser(db, tempUserName, userType))
                    {
                        extra = username;
                        cout << "\nUser with this UserName already Exists! Enter UserName again: ";
                        cin >> tempUserName;
                        if (tempUserName == username)
                        {
                            break;
                        }
                    }
                    values[i] = tempUserName;
                }
            }
            else
            {
                getline(cin, values[i]);
            }
        }
        // Formulate the update query
        string query = "UPDATE student SET ";
        for (int i = 0; i < numFields; ++i) {
            query += fields[i] + " = '" + values[i] + "'";
            if (i < numFields - 1) {
                query += ", ";
            }
        }
        query += " WHERE username = '" + username + "'";
        // Execute the update query
        if (mysql_query(conn, query.c_str()))
        {
            cerr << "Error updating student data: " << mysql_error(conn) << endl;
            return;
        }
        cout << "Student data updated successfully." << endl;
    }
    else
    {
        cerr << "Invalid user type." << endl;
    }
}
void Admin::deleteStaff(Database& db, string& username)
{
    int staffId = getStaffIdByUsername(db, username);
    MYSQL* conn = db.getConnection();
    // Delete from staffid_subcode table
    string subcodeQuery = "DELETE FROM staffid_subcode WHERE staff_id = " + to_string(staffId);
    if (mysql_query(conn, subcodeQuery.c_str()))
    {
        cerr << "Error in deleting from staffid_subcode: " << mysql_error(conn) << endl;
        return;
    }
    // Delete from staff table
    string staffQuery = "DELETE FROM staff WHERE username='" + username + "' AND staff_id = " + to_string(staffId);
    if (mysql_query(conn, staffQuery.c_str())) {
        cerr << "Error in deleting from staff: " << mysql_error(conn) << endl;
        return;
    }

    string query = "DELETE FROM holy_salt WHERE username='" + username + "'";
    if (mysql_query(conn, query.c_str()))
    {
        cerr << "Error in deleting password/hash: " << mysql_error(conn) << endl;
        return;
    }
    cout << "Staff " << username << " with staff_id " << staffId << " has been successfully deleted." << endl;
}
void Admin::deleteUser(Database& db, string& username, string& userType)
{
    if (userType == "staff")
    {
    }
    MYSQL* conn = db.getConnection();
    string query = "DELETE FROM " + userType + " WHERE username='" + username + "'";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in deleting user: " << mysql_error(conn) << endl;
        return;
    }
    query = "DELETE FROM holy_salt WHERE username='" + username + "'";
    if (mysql_query(conn, query.c_str())) 
    {
        cerr << "Error in deleting password/hash: " << mysql_error(conn) << endl;
        return;
    }

    cout << "User " << username << " has been successfully deleted." << endl;
}
void Admin::insertAdmin(Database& db, const Admin& admin)
{
    MYSQL* conn = db.getConnection();

    //Password Hashing and storing
    string username = admin.getUsername();
    string salt = passFobj.generateSalt();
    stringstream hash;
    string password = admin.getPassword();
    password += salt;
    passFobj.sha256(password, hash);
    password = hash.str();
    
    string mail;
    cout << "Enter the mail : ";
    cin >> mail;

    string query = "INSERT INTO admin (username, password, real_name, dob, address, joining_date, salary,mail) VALUES ('"
        + admin.getUsername() + "', '" + password + "', '" + admin.getRealName() + "', '"
        + admin.getDob() + "', '" + admin.getAddress() + "', '" + admin.getJoiningDate() + "', "
        + to_string(admin.getSalary()) + ", '" + mail + "')";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in inserting admin: " << mysql_error(conn) << endl;
    }
    else 
    {
        passFobj.addPassword(db, username, password, salt);
        cout << "\nAdmin Successfully Added.\n";
    }
}
vector<string> Admin::getSubCodes(Database& db, string& branchCode, int semester)
{
    vector<string> subCodes;
    MYSQL* conn = db.getConnection();
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return subCodes;
    }
    // Construct the query to get all sub_codes for the given branch_code and semester
    string query = "SELECT sub_code FROM branchcode_subcode_sem WHERE branch_code = '" + branchCode + "' AND semester = " + to_string(semester);
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in fetching sub_codes: " << mysql_error(conn) << endl;
        return subCodes;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return subCodes;
    }
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        subCodes.push_back(row[0]);
    }
    mysql_free_result(res);
    return subCodes;
}
void Admin::insertMarks(Database& db, int studentId, string& branchCode, int semester, string& section) 
{
    MYSQL* conn = db.getConnection();
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return;
    }
    vector<string> subCodes = getSubCodes(db, branchCode, semester);
    if (subCodes.empty()) {
        cerr << "No sub_codes found for the given branch_code and semester" << endl;
        return;
    }
    for (const string& subCode : subCodes) {
        // Construct the query to insert 0 for mid_marks and end_marks
        string insertQuery = "INSERT INTO marks (student_id, branch_code, semester, section, sub_code, mid_marks, end_marks) VALUES ("
            + to_string(studentId) + ", '" + branchCode + "', " + to_string(semester) + ", '"
            + section + "', '" + subCode + "', 0, 0)";
        if (mysql_query(conn, insertQuery.c_str())) {
            cerr << "Error in inserting marks for sub_code " << subCode << ": " << mysql_error(conn) << endl;
        }
    }
}

void Admin::insertCgpa(Database& db, int studentId, string& branchCode, int semester, string& realName, string courseId, int gradYear)
{
    MYSQL* conn = db.getConnection();
    string query = "INSERT INTO cgpa (student_id, real_name, course_id, branch_code, semester, cgpa, grad_year) VALUES ("
        + to_string(studentId) + ", '"
        + realName + "', '"
        + courseId + "', '"
        + branchCode + "', "
        + to_string(semester) + ", 0.00, "
        + to_string(gradYear) + ")";

    if (mysql_query(conn, query.c_str())) 
    {
        cerr << "Error executing query: " << mysql_error(conn) << endl;
    }
}

void Admin::insertStudent(Database& db, const Student& student)
{
    MYSQL* conn = db.getConnection();

    int grad_year = 0;
    cout << "Enter the Graduation Year : ";
    cin >>grad_year;
    cin.ignore();

    //Password Hashing and storing
    string username = student.getUsername();
    string salt = passFobj.generateSalt();
    stringstream hash;
    string password = student.getPassword();
    password += salt;
    passFobj.sha256(password, hash);
    password = hash.str();

    string mail;
    cout << "Enter the mail : ";
    cin >> mail;

    string query = "INSERT INTO student (username, password, real_name, address, course, branch, section, course_id, branch_code, semester,cgpa,grad_year,mail) VALUES ('"
        + student.getUsername() + "', '" + password + "', " + " '" + student.getRealName() + "', '"
        + student.getAddress() + "', '" + student.getCourse() + "', '" + student.getBranch() + "', '" + student.getSection() + "', '"
        + student.getCourseId() + "', '" + student.getBranchCode() + "', "
        + to_string(student.getSemester()) + ", " + "0.0 ," + to_string(grad_year) + ",'"  + mail + "')";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in inserting student: " << mysql_error(conn) << endl;
    }
    passFobj.addPassword(db, username, password, salt);

    string selectQuery = "SELECT branch_code, semester,section FROM student WHERE username = '" + username + "'";
    if (mysql_query(conn, selectQuery.c_str()))
    {
        cerr << "Error in retrieving branch_code and semester: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr)
    {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == nullptr)
    {
        cerr << "No student found with the given username." << std::endl;
        mysql_free_result(res);
        return;
    }
    string branchCode = row[0];
    int semester = atoi(row[1]);
    string section = row[2];
    mysql_free_result(res);
    // Retrieve fees from fees table using branch_code and semester
    selectQuery = "SELECT fees FROM fees WHERE branch_code = '" + branchCode + "' AND semester = " + to_string(semester);
    if (mysql_query(conn, selectQuery.c_str()))
    {
        cerr << "Error in retrieving fees: " << mysql_error(conn) << endl;
        return;
    }
    res = mysql_store_result(conn);
    if (res == nullptr)
    {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    row = mysql_fetch_row(res);
    if (row == nullptr) {
        cerr << "No fees found for the given branch_code and semester." << endl;
        mysql_free_result(res);
        return;
    }
    double fees = atof(row[0]);
    mysql_free_result(res);
    // Update the student table with the retrieved fees
    string updateQuery = "UPDATE student SET fees = " + to_string(fees) + " WHERE username = '" + username + "'";
    if (mysql_query(conn, updateQuery.c_str()))
    {
        cerr << "Error in updating fees: " << mysql_error(conn) << endl;
        return;
    }
    int studentid = getStudentId(db, username);
    insertMarks(db, studentid, branchCode, semester, section);
    string real_name = student.getRealName();
    string courseId= student.getCourseId();
    insertCgpa(db, studentid, branchCode, semester, real_name, courseId, grad_year);
    // Retrieve all students in the same branch, semester, and section, sorted by real_name
    selectQuery = "SELECT student_id, real_name FROM student WHERE branch_code = '" + branchCode +
        "' AND semester = " + to_string(semester) + " AND section = '" + section + "' ORDER BY real_name";
    if (mysql_query(conn, selectQuery.c_str())) {
        cerr << "Error in retrieving students: " << mysql_error(conn) << endl;
        return;
    }
    res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    // Update roll numbers based on alphabetical order of real_name
    int roll_no = 1;
    while ((row = mysql_fetch_row(res))) {
        int student_id = stoi(row[0]);
        string updateQuery = "UPDATE student SET roll_no = " + to_string(roll_no) + " WHERE student_id = " + to_string(student_id);
        if (mysql_query(conn, updateQuery.c_str()))
        {
            cerr << "Error in updating roll number: " << mysql_error(conn) << endl;
            mysql_free_result(res);
            return;
        }
        roll_no++;
    }
    cout << "Student Sucessfully Added.\n";
    mysql_free_result(res);
}
void Admin::insertStaff(Database& db, const Staff& staff)
{
    MYSQL* conn = db.getConnection();

    //Password Hashing and storing
    string username = staff.getUsername();
    string salt = passFobj.generateSalt();
    stringstream hash;
    string password = staff.getPassword();
    password += salt;
    passFobj.sha256(password, hash);
    password = hash.str();
    string mail;
    cout << "Enter the mail : ";
    cin >> mail;

    string query = "INSERT INTO staff (username, password, real_name, dob, branch, salary, address, joining_date, num_of_subjects, branch_code,mail) VALUES ('"
        + staff.getUsername() + "', '" + password + "', '" + staff.getRealName() + "', '" + staff.getDob() + "', '"
        + staff.getBranch() + "', " + to_string(staff.getSalary()) + ", '" + staff.getAddress() + "', '" + staff.getJoiningDate() + "', "
        + to_string(staff.getNumOfSubjects()) + ", '" + staff.getBranchCode() + "', '"+ mail +"')";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in inserting staff: " << mysql_error(conn) << endl;
    }
    else 
    {
        passFobj.addPassword(db, username, password, salt);
        cout << "\nStaff Successfully Added.\n";
        cout << "Want to Enter the subjects Now ( y or n )\n";
        string ans;
        cin >> ans;
        if (ans == "n" && ans != "y")
        {
            return;
        }
        string username=staff.getUsername();
        int staffId = getStaffIdByUsername(db, username);
        vector<string> subjectCodes;
        cout << "Enter " << staff.getNumOfSubjects() << " subject codes:\n";
        for (int i = 0; i < staff.getNumOfSubjects(); ++i) 
        {
            string subCode;
            cout << "Enter subject code " << (i + 1) << ": ";
            cin >> subCode;
            subjectCodes.push_back(subCode);
        }
        string section;
        int semester;
        string branchCode = staff.getBranchCode();
        // Check if each subject exists and insert into staffid_subcode table
        for (string subCode : subjectCodes)
        {
            if (checkSubExists(db, subCode))
            {
                semester = getSemesterFromSubCode(db, subCode);
                cout << "Enter the Section of SubCode (" + subCode + ") : ";
                cin >> section;
                if (!(checkSectionExists(db, branchCode, semester, section)))
                {
                    cout << "Section Does'nt Exists or no Students in this Section for this Branch: " + staff.getBranch() << endl;
                    return;
                }
                string insertQuery = "INSERT INTO staffid_subcode (staff_id, sub_code, section) VALUES ("
                    + to_string(staffId) + ", '" + subCode + "', '" + section + "')";
                if (mysql_query(conn, insertQuery.c_str())) {
                    cerr << "Error in inserting staffid_subcode: " << mysql_error(conn) << endl;
                }
                else
                {
                    cout << "Subject code " << subCode << " successfully added for staff ID " << staffId << ".\n";
                }
            }
            else
            {
                cerr << "Subject code " << subCode << " does not exist and was not added.\n";
            }
        }
    }
}
void Admin::addNewUser(Database& db)
{
    MYSQL* conn = db.getConnection();
    string username, password, userType;
    cout << "Choose user type (admin, staff, student): ";
    cin >> userType;
    if (userType != "admin" && userType != "staff" && userType != "student")
    {
        cout << "Enter Valid UserType\n";
        return;
    }
    cout << "Enter username: ";
    cin >> username;
    if (check_exist(db, username, userType))
    {
        cout << "Username already exists.\n";
        return;
    }
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
        else 
        {
            password.push_back(ch);
            cout << '*'; // Print * instead of the actual character
        }
    }
    cout << "\n";
    cin.ignore();
    if (userType == "admin") {
        string realName, address, joiningDate;
        string dob;
        double salary;
        cout << "Enter real name: ";
        getline(cin, realName);
        cout << "Enter dob: (YYYY-MM-DD) : ";
        cin >> dob;
        cin.ignore();
        cout << "Enter address: ";
        getline(cin, address);
        cout << "Enter joining date (YYYY-MM-DD): ";
        cin >> joiningDate;
        cout << "Enter salary: ";
        cin >> salary;
        Admin admin(username, password, realName, dob, address, joiningDate, salary);
        insertAdmin(db, admin);
        
    }
    else if (userType == "staff")
    {
        string realName, branch, address, joiningDate, branch_code, dob;
        int  numOfSubjects;
        double salary;
        cout << "Enter real name: ";
        getline(cin, realName);
        cout << "Enter DOB: (YYYY-MM-DD): ";
        cin >> dob;
        cin.ignore();
        cout << "Enter salary: ";
        cin >> salary;
        cin.ignore();
        cout << "Enter address: ";
        getline(cin, address);
        cout << "Enter joining date (YYYY-MM-DD): ";
        cin >> joiningDate;
        cout << "Enter number of subjects: ";
        cin >> numOfSubjects;
        cin.ignore();
        cout << "Enter branch code: ";
        cin >> branch_code;
        if (!(checkBranchExists(db, branch_code)))
        {
            cout << "Invalid Branch Code!!\n";
            return;
        }
        branch = getBranchFromBranchId(db, branch_code);
        Staff staff(username, password, realName, dob, branch, salary, address, joiningDate, numOfSubjects, branch_code);
        insertStaff(db, staff);
    }
    else if (userType == "student")
    {
        int semester;
        string realName, address, course, branch, section, courseId, branchCode;
        cout << "Enter real name: ";
        getline(cin, realName);
        cout << "Enter address: ";
        getline(cin, address);
        cout << "Enter courseId: ";
        cin >> courseId;
        if (!(checkCourseExists(db, courseId)))
        {
            cout << "Invalid CourseId!!\n";
            return;
        }
        cout << "Enter branch Code: ";
        cin >> branchCode;
        if (!(checkBranchExists(db, branchCode)))
        {
            cout << "Invalid Branch Code!!\n";
            return;
        }
        cout << "Enter section: ";
        cin >> section;
        cout << "Enter semester: ";
        cin >> semester;
        if (!(checkSemesterExists(db, branchCode, semester)))
        {
            cout << "Invalid Semester for this BranchCode: " + branchCode<<"\n";
            return;
        }
        cin.ignore();
        course = getCourseFromCourseId(db, courseId);
        branch = getBranchFromBranchId(db, branchCode);
        Student student(username, password, realName, address, course, branch, section, courseId, branchCode, semester);
        insertStudent(db, student);
    }
    else
    {
        cout << "Invalid user type." << endl;
    }
}
void Admin::change_sem(Database& db)
{
    MYSQL* conn = db.getConnection();

    // Query to truncate the StaffID_Subcode table
    string truncateQuery = "TRUNCATE TABLE staffid_subcode";

    // Execute the query
    if (mysql_query(conn, truncateQuery.c_str())) {
        cerr << "Failed to truncate table. Error: " << mysql_error(conn) << endl;
        return;
    }

    // Query to truncate the Attendance table
    truncateQuery = "TRUNCATE TABLE attendance";

    // Execute the query
    if (mysql_query(conn, truncateQuery.c_str())) {
        cerr << "Failed to truncate table. Error: " << mysql_error(conn) << endl;
        return;
    }

    // Query to truncate the Attendance table
    truncateQuery = "TRUNCATE TABLE total_attendance";

    // Execute the query
    if (mysql_query(conn, truncateQuery.c_str())) {
        cerr << "Failed to truncate table. Error: " << mysql_error(conn) << endl;
        return;
    }

    // Construct the SQL query to update num_of_subjects to 0 for all staff
    string updateQuery = "UPDATE staff SET num_of_subjects = 0";
    // Execute the update query
    if (mysql_query(conn, updateQuery.c_str())) {
        cerr << "Query to update staff num_of_subjects failed: " << mysql_error(conn) << endl;
        return;
    }

    string query = "SELECT *  FROM student";

    if (mysql_query(conn, query.c_str())) 
    {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) 
    {
        cerr << "Failed to store result: " << mysql_error(conn) << endl;
        return;
    }
    int total_student_sem_success = 0;
    int total_grad = 0;
    int total_grad_not_fees_paid = 0;
    int total_student_not_fees_paid = 0;
    int total_sem_error = 0;
    MYSQL_ROW row;
    vector<int> StudentSemError;
    vector<int> PendingFeesStudent;
    vector<int> PendingFeesGradStudent;
    while ((row = mysql_fetch_row(res)) != nullptr) 
    {
        int studentId = atoi(row[8]);
        string branchCode = row[11];
        int semester = atoi(row[12]);
        string section = row[7];
        double pendingFeesGrad = 0;
        string courseId = row[10];
        int gradYear = atoi(row[14]);
        string real_name = row[3];
        if (!(checkSemesterExists(db, branchCode, semester + 1)))
        {
            if (!(checkFeesPaid(db, studentId)))
            {
                total_grad_not_fees_paid += 1;
                PendingFeesGradStudent.push_back(studentId);
                pendingFeesGrad = atof(row[9]);
            }
            string address = row[4];
            string course = row[5];
            string branch = row[6];
            //Stuedent Got graduated
            float cgpa = stof(row[13]);

            string query = "INSERT INTO grad_student (student_id, real_name, address, course, branch, section, course_id, branch_code, cgpa, grad_year,pending_fees) VALUES ("
                + to_string(studentId) + ", '"
                + real_name + "', '"
                + address + "', '"
                + course + "', '"
                + branch + "', '"
                + section + "', '"
                + courseId + "', '"
                + branchCode + "', "
                + to_string(cgpa) + ", "
                + to_string(gradYear) + ","
                + to_string(pendingFeesGrad)+")";

            // Execute the query
            if (mysql_query(conn, query.c_str())) 
            {
                total_sem_error += 1;
                StudentSemError.push_back(studentId);
                continue;
            }
            else
            {
                total_grad += 1;
                deleteStudent(db, studentId);
            }
        }
        else
        {
            //We need to update marks of all student to their new semester to 0
            insertMarks(db, studentId, branchCode, semester + 1, section);
            insertCgpa(db, studentId, branchCode, semester + 1, real_name, courseId, gradYear);
            //Now we Need to update fees of students to their new semester fees
            if (!(checkFeesPaid(db, studentId)))
            {
                PendingFeesStudent.push_back(studentId);
                total_student_not_fees_paid += 1;
            }
            else
            {
                total_student_sem_success += 1;
                updateFees(db, studentId, branchCode, semester + 1);
                resetCurCgpa(db, studentId, branchCode, semester);
                updateSemester(db, studentId, semester + 1);
            }
        }
    }
    cout<<"Total Student Graduate : " << total_grad<<endl;
    cout << "Total Student Successfull Semester Change : " << total_student_sem_success << endl;
    
    if (total_student_not_fees_paid > 0)
    {
        cout << "\nTotal Student Fees Not Paid : " << total_student_not_fees_paid << endl;
        cout << "Their Student Ids: ";
        for (int i = 0; i < PendingFeesStudent.size(); i++)
        {
            cout << PendingFeesStudent[i]<<" , ";
        }
    }
    if (total_grad_not_fees_paid > 0)
    {
        cout << "\nTotal Graduate Student Fees Not Paid : " << total_grad_not_fees_paid << endl;
        cout << "Their Student Ids: ";
        for (int i = 0; i < PendingFeesGradStudent.size(); i++)
        {
            cout << PendingFeesGradStudent[i] << " , ";
        }
    }
    if (total_sem_error > 0)
    {
        cout << "\nTotal Student Update of Semester Failed: " << total_sem_error << endl;
        cout << "Their StudentIds : ";
        for (int i = 0; i < StudentSemError.size(); i++)
        {
            cout << StudentSemError[i]<<" , ";
        }
    }
    mysql_free_result(res);

}
void Admin::addStudentFees(Database& db)
{
    MYSQL* conn = db.getConnection();
    int studentId;
    cout << "Enter the StudentId: ";
    cin >> studentId;
    cin.ignore();
    if (!(checkStudentExists(db, studentId)))
    {
        cout << "Student with StudentId :" + studentId << " doesn't Exists!!\n";
        return;
    }
    double paymentAmount;
    
    cout << "\nEnter the payment amount: ";
    cin >> paymentAmount;
    string selectQuery = "SELECT fees FROM student WHERE student_id = " + to_string(studentId);
    if (mysql_query(conn, selectQuery.c_str())) {
        cerr << "Error in retrieving current fees: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == nullptr) {
        cerr << "No student found with the given student_id." << endl;
        mysql_free_result(res);
        return;
    }
    double currentFees = atof(row[0]);
    mysql_free_result(res);
    // Calculate the updated fees
    double updatedFees = currentFees - paymentAmount;
    // Update the student table with the new fees
    string updateQuery = "UPDATE student SET fees = " + to_string(updatedFees) + " WHERE student_id = " + to_string(studentId);
    if (mysql_query(conn, updateQuery.c_str())) {
        cerr << "Error in updating fees: " << mysql_error(conn) << endl;
        return;
    }
    cout << "Fees updated successfully for student_id: " << studentId << ". Pending fees: " << updatedFees << endl;
}
void Admin::addStudentExtraFees(Database& db)
{
    MYSQL* conn = db.getConnection();
    cout << "Enter the StudentId: ";
    int studentId;
    cin >> studentId;
    if (!(checkStudentExists(db, studentId)))
    {
        cout << "Student with StudentId :" + studentId << " doesn't Exists!!\n";
        return;
    }
    double extraFees;
    cout << "\nEnter the payment amount: ";
    cin >> extraFees;
    // Retrieve current fees for the given studentId from student table
    string selectQuery = "SELECT fees FROM student WHERE student_id = " + to_string(studentId);
    if (mysql_query(conn, selectQuery.c_str())) {
        cerr << "Error in retrieving current fees: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == nullptr) {
        cerr << "No student found with the given student_id." << endl;
        mysql_free_result(res);
        return;
    }
    double currentFees = atof(row[0]);
    mysql_free_result(res);
    // Calculate the updated fees
    double updatedFees = currentFees + extraFees;
    // Update the student table with the new fees
    string updateQuery = "UPDATE student SET fees = " + to_string(updatedFees) + " WHERE student_id = " + to_string(studentId);
    if (mysql_query(conn, updateQuery.c_str())) {
        cerr << "Error in updating fees: " << mysql_error(conn) << endl;
        return;
    }
    cout << "Fees updated successfully for student_id: " << studentId << ". Updated fees: " << updatedFees << endl;
}
void Admin::viewStudentPendingFeesBranchSem(Database& db)
{
    int semester;
    string branchCode;
    cout << "Enter the BranchCode: ";
    cin >> branchCode;
    cout << "\nEnter the Semester: ";
    cin >> semester;
    MYSQL* conn = db.getConnection();
    // Query to select students with pending fees in the specified branch and semester
    string selectQuery = "SELECT student_id, real_name, branch, section, fees,username FROM student WHERE branch_code = '" + branchCode + "' AND semester = " + to_string(semester) + " AND fees > 0";
    if (mysql_query(conn, selectQuery.c_str())) 
    {
        cerr << "Error in retrieving pending fees: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row;

    // Print header row
    cout << "List of students with pending fees:" << endl;
    cout << left << setw(12) << "Student ID"
        << left << setw(20) << "Username"
        << left << setw(35) << "Real Name"
        << left << setw(40) << "Branch"
        << left << setw(10) << "Section"
        << left << setw(15) << "Pending Fees" << endl;

    cout << string(92, '-') << endl; // Line separator

    while ((row = mysql_fetch_row(res)) != nullptr) 
    {
        int studentId = atoi(row[0]);
        string username = row[5];
        string realName = row[1];
        string branch = row[2];
        string section = row[3];
        double pendingFees = atof(row[4]);

        cout << left << setw(12) << studentId
            << left << setw(20) << username
            << left << setw(35) << realName
            << left << setw(40) << branch
            << left << setw(10) << section
            << left << setw(15) << pendingFees << endl;
    }

    mysql_free_result(res);
}
void Admin::viewStudentFeesBranchSem(Database& db)
{
    MYSQL* conn = db.getConnection();
    int semester;
    string branchCode;
    cout << "Enter the BranchCode: ";
    cin >> branchCode;
    cout << "\nEnter the Semester: ";
    cin >> semester;
    // Query to select students in the specified branch and semester
    string selectQuery = "SELECT student_id, real_name, section, branch, semester, fees FROM student WHERE branch_code = '" + branchCode + "' AND semester = " + to_string(semester);
    if (mysql_query(conn, selectQuery.c_str())) {
        cerr << "Error in retrieving student details: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row;
    // Print header row

    cout << "Student Details for Branch Code: " << branchCode << " and Semester: " << semester << endl;
    cout << left << setw(12) << "Student ID"
        << left << setw(35) << "Real Name"
        << left << setw(10) << "Section"
        << left << setw(40) << "Branch"
        << left << setw(10) << "Semester"
        << left << setw(10) << "Fees" << endl;

    cout << string(110, '-') << endl; // Line separator

    while ((row = mysql_fetch_row(res)) != nullptr) {
        int studentId = atoi(row[0]);
        string realName = row[1];
        string section = row[2];
        string branch = row[3];
        int sem = atoi(row[4]);
        double fees = atof(row[5]);

        cout << left << setw(12) << studentId
            << left << setw(35) << realName
            << left << setw(10) << section
            << left << setw(40) << branch
            << left << setw(10) << sem
            << left << setw(10) << fees << endl;
    }

    mysql_free_result(res);
}
void Admin::viewAdminSalary(Database& db)
{
    MYSQL* conn = db.getConnection();
    // Query to select admin details
    string selectQuery = "SELECT username, real_name, salary FROM admin";
    if (mysql_query(conn, selectQuery.c_str())) {
        cerr << "Error in retrieving admin details: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr)
    {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row;
    // Print header row

    cout << "Admin Salary Details:" << endl;
    cout << left << setw(15) << "Username"
        << left << setw(30) << "Real Name"
        << left << setw(10) << "Salary" << endl;

    cout << string(60, '-') << endl; // Line separator

    while ((row = mysql_fetch_row(res)) != nullptr) {
        string username = row[0];
        string realName = row[1];
        double salary = atof(row[2]);

        cout << left << setw(15) << username
            << left << setw(30) << realName
            << left << setw(10) << salary << endl;
    }
    mysql_free_result(res);
}
void Admin::viewStaffSalary(Database& db)
{
    MYSQL* conn = db.getConnection();
    // Query to select staff details
    string selectQuery = "SELECT staff_id,real_name, branch, salary,username FROM staff";
    if (mysql_query(conn, selectQuery.c_str())) {
        cerr << "Error in retrieving staff details: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row;

    // Print header row
    cout << "Staff Salary Details:" << endl;
    cout << left << setw(10) << "Staff ID"
        << left << setw(20) << "Username"
        << left << setw(30) << "Real Name"
        << left << setw(40) << "Branch"
        << left << setw(10) << "Salary" << endl;

    cout << string(60, '-') << endl; // Line separator

    while ((row = mysql_fetch_row(res)) != nullptr) {
        int staffId = atoi(row[0]);
        string realName = row[1];
        string branch = row[2];
        double salary = atof(row[3]);
        string username = row[4];
        cout << left << setw(10) << staffId
            << left << setw(20) << username
            << left << setw(30) << realName
            << left << setw(40) << branch
            << left << setw(10) << salary << endl;
    }
    mysql_free_result(res);
}
void Admin::viewSpecificSalary(Database& db)
{
    MYSQL* conn = db.getConnection();
    string userType;
    string userId;
    cout << "Enter the useType, [ admin , staff ] : ";
    cin >> userType;
    if (userType != "admin" && userType != "staff")
    {
        cout << "\nWrong userType! Try Again. \n";
        return;
    }
    if (userType == "admin")
    {
        cout << "Enter the Admin username: ";
        cin >> userId;
    }
    else
    {
        cout << "Enter the StaffId: ";
        cin >> userId;
    }
    string selectQuery;
    if (userType == "admin")
    {
        selectQuery = "SELECT username, real_name, salary FROM admin WHERE username = '" + userId + "'";
    }
    else if (userType == "staff")
    {
        selectQuery = "SELECT staff_id, real_name, branch, salary FROM staff WHERE staff_id = " + userId;
    }
    else {
        cerr << "Invalid user type." << endl;
        return;
    }
    if (mysql_query(conn, selectQuery.c_str())) {
        cerr << "Error in retrieving user salary: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == nullptr)
    {
        cerr << "No user found with the given ID." << endl;
        mysql_free_result(res);
        return;
    }
    cout << "User Salary Details:" << endl;
    if (userType == "staff")
    {
        // Print header row
        cout << left << setw(10) << "Staff ID"
            << left << setw(35) << "Real Name"
            << left << setw(40) << "Branch"
            << left << setw(10) << "Salary" << endl;

        cout << string(80, '-') << endl; // Line separator

        cout << left << setw(10) << row[0]
            << left << setw(35) << row[1]
            << left << setw(40) << row[2]
            << left << setw(10) << row[3] << endl;
    }
    else
    {
        cout << left << setw(15) << "Username"
            << left << setw(30) << "Real Name"
            << left << setw(10) << "Salary" << endl;

        cout << string(50, '-') << endl; // Line separator

        cout << left << setw(15) << row[0]
            << left << setw(30) << row[1]
            << left << setw(10) << row[2] << endl;
    }
    mysql_free_result(res);
}
void Admin::updateSalary(Database& db)
{
    MYSQL* conn = db.getConnection();
    string userType;
    string userId;
    cout << "Enter the useType, [ admin , staff ] : ";
    cin >> userType;
    if (userType != "admin" && userType != "staff")
    {
        cout << "\nWrong userType! Try Again. \n";
        return;
    }
    if (userType == "admin")
    {
        cout << "Enter the Admin username: ";
        cin >> userId;
        if (!(checkExistsUser(db, userId, userType)))
        {
            cout << "This Username Admin doesn't exists!!\n";
            return;
        }
    }
    else
    {
        cout << "Enter the Staff Username: ";
        cin >> userId;
        if (!(checkExistsUser(db, userId, userType)))
        {
            cout << "This Username Staff doesn't exists!!\n";
            return;
        }
    }
    cout << "Enter the Updated Salary: ";
    double UpdatedSalary;
    cin >> UpdatedSalary;
    string updateQuery;
    cout << "\n";
    if (userType == "admin")
    {
        updateQuery = "UPDATE admin SET salary = " + to_string(UpdatedSalary) + " WHERE username = '" + userId + "'";
    }
    else if (userType == "staff")
    {
        updateQuery = "UPDATE staff SET salary = " + to_string(UpdatedSalary) + " WHERE username = '" + userId + "'";
    }
    else
    {
        cerr << "Invalid user type." << endl;
        return;
    }
    if (mysql_query(conn, updateQuery.c_str()))
    {
        cerr << "Error in updating salary: " << mysql_error(conn) << endl;
        return;
    }
    cout << "Salary updated successfully for user type: " << userType << ", ID: " << userId << ". New salary: " << UpdatedSalary << endl;
}
void Admin::viewStudentAttendenceSec(Database& db)
{
    string branchCode, sec, subCode;
    int semester;
    cout << "Enter the branchCode ";
    {
        cin >> branchCode;
    }
    if (!(checkBranchExists(db, branchCode)))
    {
        cout << "Branch Does't Exist! Enter correct branch code.\n";
        return;
    }
    cout << "Enter the semester: ";
    cin >> semester;
    cin.ignore();
    cout << "\nEnter the Sec: ";
    cin >> sec;
    cout << "Enter the SubCode: ";
    cin >> subCode;
    if (!(checkSubExists(db, subCode)))
    {
        cout << "Subject Doesn't Exist!! Enter correct subcode. \n";
        return;
    }
    MYSQL* conn = db.getConnection();

    // Query to select student attendance details with matching fields
    string selectQuery = "SELECT a.student_id, c.branch, a.semester, a.section, s.sub_name, a.present, a.roll_no "
        "FROM attendance AS a "
        "JOIN subcode_subname AS s ON a.sub_code = s.sub_code "
        "JOIN courseid_branchcode AS c ON a.branch_code = c.branch_code "
        "WHERE a.branch_code = '" + branchCode + "' AND a.semester = " + to_string(semester) + " AND a.section = '" + sec + "' AND a.sub_code = '" + subCode + "'";
    if (mysql_query(conn, selectQuery.c_str())) {
        cerr << "Error in retrieving student attendance: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }

    int total_attendance = getTotalAttendance(db, branchCode, semester, sec, subCode);
    MYSQL_ROW row;
    cout << "Student Attendance Details for Branch: " << branchCode << ", Semester: " << semester << ", Section: " << sec << ", Subject Code: " << subCode << endl;
    cout << left << setw(12) << "Student ID" << setw(10) << "Roll No" << setw(40) << "Branch" << setw(10) 
        << "Semester" << setw(10) << "Section" << setw(40) << "Subject" << setw(10) << "Present" << setw(10) << "Total" << endl;
    cout << "------------------------------------------------------------------------------------------------------" << endl;

    while ((row = mysql_fetch_row(res)) != nullptr) 
    {
        int studentId = atoi(row[0]);
        int roll_no = atoi(row[6]);
        string branch = row[1];
        int sem = atoi(row[2]);
        string section = row[3];
        string subName = row[4];
        int present = atoi(row[5]);

        cout << left << setw(12) << studentId
            << setw(10) << roll_no
            << setw(40) << branch
            << setw(10) << sem
            << setw(10) << section
            << setw(40) << subName
            << setw(10) << present
            << setw(10) << total_attendance
            << endl;
    }

    mysql_free_result(res);
}
void Admin::updateStudentAttendance(Database& db)
{
    MYSQL* conn = db.getConnection();
    int num_student, num_subject;
    cout << "Enter the number of students to update Attendance: ";
    cin >> num_student;
    map<int, vector<string>> mp;
    int studentId;
    cin.ignore();
    for (int i = 0; i < num_student; i++)
    {
        cout << "\nEnter the studentId: ";
        cin >> studentId;
        if (!(checkStudentExists(db, studentId)))
        {
            cout << "\nNo Student of that StudentId Exists. Enter Valid Student Id!\n";
            return;
        }
        cin.ignore();
        cout << "Enter num of subjects to Update of StudentId: " << studentId << ": ";
        cin >> num_subject;
        cin.ignore();
        string sub;
        cout << "Enter the subcodes: \n";
        for (int j = 0; j < num_subject; j++)
        {
            cout << j+1<<" : ";
            cin >> sub;
            if (!(checkSubExists(db, sub)))
            {
                cout << "\nSubject Doesn't Exist!! Enter correct subcode. \n";
                return;
            }
            mp[studentId].push_back(sub);
        }
    }
    int present;
    for (const auto& entry : mp)
    {
        int studentid = entry.first;
        const vector<string>& subCodes = entry.second;
        for (const string& subCode : subCodes)
        {
            // Query to check if the student already exists in the attendance table
            string attendanceCheckQuery = "SELECT COUNT(*) FROM attendance WHERE student_id = " + to_string(studentid)
                + " AND sub_code = '" + subCode + "'";
            // Execute the attendance check query
            if (mysql_query(conn, attendanceCheckQuery.c_str())) {
                cerr << "Error in checking attendance: " << mysql_error(conn) << endl;
                return;
            }
            MYSQL_RES* attendanceCheckRes = mysql_store_result(conn);
            if (attendanceCheckRes == nullptr) {
                cerr << "Error in storing attendance check result: " << mysql_error(conn) << endl;
                return;
            }
            MYSQL_ROW attendanceCheckRow = mysql_fetch_row(attendanceCheckRes);
            int count = attendanceCheckRow ? atoi(attendanceCheckRow[0]) : 0;
            mysql_free_result(attendanceCheckRes);
            MYSQL_RES* res;
            MYSQL_ROW row;
            string query = "SELECT section, branch_code, semester, roll_no "
                "FROM student WHERE student_id = " + to_string(studentid);
            // Execute the query
            if (mysql_query(conn, query.c_str())) {
                cerr << "Query failed: " << mysql_error(conn) << endl;
                continue;
            }
            res = mysql_store_result(conn);
            if (res == nullptr) {
                cerr << "Failed to retrieve result: " << mysql_error(conn) << endl;
                continue;
            }
            // Check if the result set is empty
            if (mysql_num_rows(res) == 0)
            {
                cout << "No student found with ID " << studentid << endl;
                mysql_free_result(res);
                continue;
            }
            string branchCode, section;
            int semester = 0, rollNo = 0;
            // Fetch and print the student details
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                section = row[0];
                branchCode = row[1];
                semester = atoi(row[2]);
                rollNo = atoi(row[3]);
            }
            int total_attendance = getTotalAttendance(db, branchCode, semester, section, subCode);

            cout << "\nStudentId : "<<studentid << "\n";
            cout << "Total Attendance in " + subCode + " : " << total_attendance << "\n";
            present = 0;
            cout << "Enter the Present Value: ";
            cin >> present;
            while (present > total_attendance || present < 0)
            {
                cout << "\nPresent Can't be greater than Total Attendance or lower than 0 " + total_attendance << " \nEnter Again : ";
                cin >> present;
            }
            if (count == 0)
            {
                // Insert the student into the attendance table if they do not exist
                string insertQuery = "INSERT INTO attendance (student_id, branch_code, semester, section, sub_code, present, roll_no) VALUES ("
                    + to_string(studentid) + ", '" + branchCode + "', " + to_string(semester)
                    + ", '" + section + "', '" + subCode + "', " + to_string(present) + "," + to_string(rollNo) + ")";
                if (mysql_query(conn, insertQuery.c_str())) {
                    cerr << "Error in inserting attendance: " << mysql_error(conn) << endl;
                    return;
                }
            }
            else
            {
                // Construct the update query
                string updateQuery = "UPDATE attendance SET present = " + to_string(present) +
                    " WHERE student_id = " + to_string(studentid) + " AND sub_code = '" + subCode + "'";
                if (mysql_query(conn, updateQuery.c_str()))
                {
                    cerr << "Error in updating attendance: " << mysql_error(conn) << endl;
                    return;
                }
            }
            cout << "\nAttendance updated successfully for Student ID: " << studentid << ", Subject: " << subCode << endl;
            mysql_free_result(res);
        }
    }
}





void Admin::adminMenu(Database& db, string& user)
{
    int choice;
    string username;
    string userType,user2,userT;
    int ch, ch2;
    do
    {
        cout << "\nAdmin Menu:" << endl;
        cout << "1. View Data" << endl;
        cout << "2. Update Data" << endl;
        cout << "3. Delete User" << endl;
        cout << "4. Add New User" << endl;
        cout << "5. Sem Change" << endl;
        cout << "6. Fees" << endl;
        cout << "7. Salary" << endl;
        cout << "8. Attendence" << endl;
        cout << "9. Change Password" << endl;
        cout << "10. Exit" << endl;
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
            do
            {
                cout << "\nAdmin View Menu:" << endl;
                cout << "1. View Admin Data" << endl;
                cout << "2. View Own Data" << endl;
                cout << "3. View Staff Data" << endl;
                cout << "4. View Students Data" << endl;
                cout << "5. View Specific Data" << endl;
                cout << "6. Exit" << endl;
                cout << "Enter your choice: ";
                cin >> ch;
                // Check if the input operation failed
                if (cin.fail())
                {
                    cin.clear(); // Clear the error state
                    cin.ignore(50, '\n');
                    cout << "Invalid input. Please enter a number." << endl << endl;
                    continue; // Skip the switch statement and prompt again
                }
                switch (ch)
                {
                case 1:
                    cout << "\nAdmin Data:" << endl;
                    viewAdminData(db);
                    break;
                case 2:
                    cout << "\nOwn Data:" << endl;
                    viewAdminOwnData(user, db);
                    break;
                case 3:
                    do
                    {
                        cout << "\nStaff Data Menu:" << endl;
                        cout << "1. All Staff Data" << endl;
                        cout << "2. Based On Branch" << endl;
                        cout << "3. Specific Staff" << endl;
                        cout << "4. Exit" << endl;
                        cout << "Enter your choice: ";
                        cin >> ch2;
                        // Check if the input operation failed
                        if (cin.fail())
                        {
                            cin.clear(); // Clear the error state
                            cin.ignore(50, '\n');
                            cout << "Invalid input. Please enter a number." << endl << endl;
                            continue; // Skip the switch statement and prompt again
                        }
                        switch (ch2)
                        {
                        case 1:
                            viewStaffData(db);
                            //All Staff Data funciton
                            break;
                        case 2:
                            viewStaffData_Branch(db);
                            //Based On Branch
                            break;
                        case 3:
                            viewStaffOwnData(db);
                            //Specific Staff
                            break;
                        case 4:
                            cout << "Exiting Staff Data Menu.." << endl;
                            break;
                        default:
                            break;
                        }
                    } while (ch2 != 4);
                    break;
                case 4:
                    do
                    {
                        cout << "\nStudent Data Menu:" << endl;
                        cout << "1. All Student Data" << endl;
                        cout << "2. Based On Course" << endl;
                        cout << "3. Based On Branch" << endl;
                        cout << "4. Based On Sec" << endl;
                        cout << "5. Specific Student" << endl;
                        cout << "6. Exit" << endl;
                        cout << "Enter your choice: ";
                        cin >> ch2;
                        // Check if the input operation failed
                        if (cin.fail())
                        {
                            cin.clear(); // Clear the error state
                            cin.ignore(50, '\n');
                            cout << "Invalid input. Please enter a number." << endl << endl;
                            continue; // Skip the switch statement and prompt again
                        }
                        switch (ch2)
                        {
                        case 1:
                            viewStudentData(db);
                            //All Staff Data funciton
                            break;
                        case 2:
                            viewStudentDataOn(db, "course_id");
                            //Based on Course
                            break;
                        case 3:
                            viewStudentDataOn(db, "branch_code");
                            //Based On Branch
                            break;
                        case 4:
                            viewStudentDataOn(db, "section");
                            //Specific Sec
                            break;
                        case 5:
                            viewStudentDataOn(db, "student_id");
                            //Specific Student
                            break;
                        case 6:
                            cout << "Exiting Student Data Menu.." << endl;
                            break;
                        default:
                            break;
                        }
                    } while (ch2 != 6);
                    break;
                case 5:
                    cout << "Enter userType: admin , student, staff  : ";
                    cin >> userType;
                    if (userType == "admin")
                    {
                        cout << "Enter the username of admin: ";
                        cin >> username;
                        viewAdminOwnData(username, db);
                    }
                    else if (userType == "staff")
                    {
                        viewStaffOwnData(db);
                    }
                    else if (userType == "student")
                    {
                        viewStudentDataOn(db, "student_id");
                    }
                    else
                    {
                        cout << "Wrong Type inserted!! Try again" << endl;
                    }
                    break;
                case 6:
                    cout << "\nExiting View Menu..." << endl;
                    break;
                }
            } while (ch != 6);
            break;
        case 2:
            do
            {
                cout << "\nAdmin Update:" << endl;
                cout << "1. Update Admin Data" << endl;
                cout << "2. Update Staff Data" << endl;
                cout << "3. Update Student Data" << endl;
                cout << "4. Exit" << endl;
                cout << "Enter your choice: ";
                cin >> ch2;
                // Check if the input operation failed
                if (cin.fail())
                {
                    cin.clear(); // Clear the error state
                    cin.ignore(50, '\n');
                    cout << "Invalid input. Please enter a number." << endl << endl;
                    continue; // Skip the switch statement and prompt again
                }
                string user1;
                switch (ch2)
                {
                case 1:
                    cout << "\nEnter the username to Update data: ";
                    cin >> user1;

                    if (!(checkExistsUser(db, user1, "admin")))
                    {
                        cout << "Invalid Username! User not Exists!!\n";
                        break;
                    }
                    UpdateData(db, user1, "admin");
                    break;
                case 2:
                    cout << "\nEnter the username to Update data: ";
                    cin >> user1;

                    if (!(checkExistsUser(db, user1, "staff")))
                    {
                        cout << "Invalid Username! User not Exists!!\n";
                        break;
                    }
                    UpdateData(db, user1, "staff");
                    break;
                case 3:
                    cout << "\nEnter the username to Update data: ";
                    cin >> user1;

                    if (!(checkExistsUser(db, user1, "student")))
                    {
                        cout << "Invalid Username! User not Exists!!\n";
                        break;
                    }
                    UpdateData(db, user1, "student");
                    break;
                case 4:
                    cout << "\nExiting Modify Menu...\n";
                    break;
                }
            } while (ch2 != 4);
            break;
        case 3:
            cout << "Enter username of User to delete: ";
            cin >> username;
            cout << "Enter the type of user [ admin | staff | student ]" << endl;
            cin >> userType;
            if (userType != "admin" && userType != "staff" && userType != "student")
            {
                cout << "Wrong Type !" << endl;
                break;
            }
            if (!(checkExistsUser(db, username, userType)))
            {
                cout << "User with this username doesn't exists!!\n";
                break;
            }
            if (userType == "staff")
            {
                deleteStaff(db, username);
            }
            else
            {
                deleteUser(db, username, userType);
            }
            break;
        case 4:
            addNewUser(db);
            break;
        case 5:
            //Semester Change Happen Here
            cout << "Change the Semester" << endl;
            cout << "1 . Are You Sure!!" << endl;
            cout << "2 . No " << endl;
            cin >> ch2;
            if (ch2 == 1)
            {
                change_sem(db);
                //Change Here
                cout << "\nDone Semester is now Changed for everyone" << endl;
            }
            else if (ch2 == 2)
            {
                cout << "Exiting ..." << endl;
            }
            else
            {
                cout << "Wrong choice !! " << endl;
            }
            break;
        case 6:
            do
            {
                cout << "\nFees Menu" << endl;
                cout << "1. Add Student Fees" << endl;
                cout << "2. Add Extra Fees" << endl;
                cout << "3. View Pending Fees Chart Based on Branch and Semester" << endl;
                cout << "4. View Fees Chart Based on Branch and Semester" << endl;
                cout << "5. Exit" << endl;
                cout << "Enter your choice: ";
                cin >> ch2;
                // Check if the input operation failed
                if (cin.fail())
                {
                    cin.clear(); // Clear the error state
                    cin.ignore(50, '\n');
                    cout << "Invalid input. Please enter a number." << endl << endl;
                    continue; // Skip the switch statement and prompt again
                }
                switch (ch2)
                {
                case 1:
                    addStudentFees(db);
                    //Add Student Fees Here
                    break;
                case 2:
                    addStudentExtraFees(db);
                    //Add extra fees
                    break;
                case 3:
                    viewStudentPendingFeesBranchSem(db);
                    //View Pending Fees Chart Based on Branch
                    break;
                case 4:
                    viewStudentFeesBranchSem(db);
                    //View Fees Chart Based on Branch and semester
                    break;
                case 5:
                    cout << "Exiting Fees Menu... " << endl;
                    break;
                default:
                    cout << "Invalid Choice !!" << endl;
                    break;
                }
            } while (ch2 != 5);
            break;
        case 7:
            do
            {
                cout << "\nSalary Menu" << endl;
                cout << "1. View Admin Salary Data" << endl;
                cout << "2. View Staff Salary Data" << endl;
                cout << "3. Specific User Salary" << endl;
                cout << "4. Update Salary" << endl;
                cout << "5. Exit" << endl;
                cout << "Enter your choice: ";
                cin >> ch2;
                // Check if the input operation failed
                if (cin.fail())
                {
                    cin.clear(); // Clear the error state
                    cin.ignore(50, '\n');
                    cout << "Invalid input. Please enter a number." << endl << endl;
                    continue; // Skip the switch statement and prompt again
                }
                switch (ch2)
                {
                case 1:
                    viewAdminSalary(db);
                    //View Admin Salary
                    break;
                case 2:
                    viewStaffSalary(db);
                    //View Staff Salary
                    break;
                case 3:
                    viewSpecificSalary(db);
                    //Update Salary
                    break;
                case 4:
                    updateSalary(db);
                    //View Fees Chart Based on Branch
                    break;
                case 5:
                    cout << "Exiting Fees Menu... " << endl;
                    break;
                default:
                    cout << "Invalid Choice !!" << endl;
                    break;
                }
            } while (ch2 != 5);
            break;
        case 8:
            do
            {
                cout << "\nAttendance Menu" << endl;
                cout << "1. Modify Student Attendance" << endl;
                cout << "2. View Student Attendance " << endl;
                cout << "3. Exit" << endl;
                cout << "Enter your choice: ";
                cin >> ch2;
                // Check if the input operation failed
                if (cin.fail())
                {
                    cin.clear(); // Clear the error state
                    cin.ignore(50, '\n');
                    cout << "Invalid input. Please enter a number." << endl << endl;
                    continue; // Skip the switch statement and prompt again
                }
                switch (ch2)
                {
                case 1:
                    updateStudentAttendance(db);
                    //Modify Student Attendance
                    break;
                case 2:
                    viewStudentAttendenceSec(db);
                    //View Student Attendance on branch,sem, sec
                    break;
                case 3:
                    cout << "Exiting Attendance Menu..." << endl;
                    break;
                default:
                    cout << "Invalid Choice !!" << endl;
                    break;
                }
            } while (ch2 != 3);
            break;
        case 9:
            do {
                cout << "-----------------------"<<endl;
                cout << "1. By old Password\n";
                cout << "2. By Email OTP\n";
                cout << "3. Exit\n";
                cout << "Enter your choice: ";
                cin >> ch2;
                // Check if the input operation failed
                if (cin.fail())
                {
                    cin.clear(); // Clear the error state
                    cin.ignore(50, '\n');
                    cout << "Invalid input. Please enter a number." << endl << endl;
                    continue; // Skip the switch statement and prompt again
                }
                switch (ch2)
                {
                case 1:
                    user2 = "-1";
                    userT = "";
                    passFobj.changePasswordKnown(db,user2,userT);
                    break;
                case 2:
                    cout << "Enter username : ";
                    cin >> username;
                    cout << "Enter the type of user [ admin | staff | student ]" << endl;
                    cin >> userType;
                    if (userType != "admin" && userType != "staff" && userType != "student")
                    {
                        cout << "Wrong Type !" << endl;
                        break;
                    }
                    if (!(checkExistsUser(db, username, userType)))
                    {
                        cout << "User with this username doesn't exists!!\n";
                        break;
                    }
                    passFobj.changePasswordOtp(db, username,userType);
                    break;
                case 3:
                    cout << "Exiting Password Change Menu!!\n";
                    break;
                default:
                    cout << "Invalid Choice!";
                    break;
                }
            } while (ch2 != 3);
            break;

        case 10:
            cout << "Exiting Admin Menu..." << endl;
            break;
        default:
            cout << "Invalid choice. Please enter again." << endl;
            break;
        }
    } while (choice != 10);
}

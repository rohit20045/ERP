#include <iostream>
#include <string>
#include<mysql.h>
#include<vector>
#include <iomanip>
#include<algorithm>

#include "database.h"
#include "staff.h"

using namespace std;

// Constructors
Staff::Staff() : username(""), password(""), realName(""), dob(""), branch(""), salary(0.0), address(""), joiningDate(""), numOfSubjects(0), branchCode("") {}

Staff::Staff(const string& username, const string& password, const string& realName, const string& dob,
    const string& branch, double salary, const string& address, const string& joiningDate,
    int numOfSubjects, const string& branchCode)
    : username(username), password(password), realName(realName), dob(dob), branch(branch), salary(salary), address(address), joiningDate(joiningDate), numOfSubjects(numOfSubjects), branchCode(branchCode) {}

// Getter functions
const string& Staff::getUsername() const { return username; }
const string& Staff::getPassword() const { return password; }
const string& Staff::getRealName() const { return realName; }
const string& Staff::getDob() const { return dob; }
const string& Staff::getBranch() const { return branch; }
double Staff::getSalary() const { return salary; }
const string& Staff::getAddress() const { return address; }
const string& Staff::getJoiningDate() const { return joiningDate; }
int Staff::getNumOfSubjects() const { return numOfSubjects; }
const string& Staff::getBranchCode() const { return branchCode; }

int Staff::getNumSubjects(Database& db, const string& branchCode, int semester)
{
    MYSQL* conn = db.getConnection();
    // Build the query
    string query = "SELECT COUNT(sub_code) FROM branchcode_subcode_sem WHERE branch_code = '" + branchCode +
        "' AND semester = " + to_string(semester);

    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "MySQL query error: " << mysql_error(conn) << endl;
        return -1; // Return -1 on error
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        cerr << "MySQL store result error: " << mysql_error(conn) << endl;
        return -1; // Return -1 on error
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    int numSubjects = (row && row[0]) ? stoi(row[0]) : 0; // Get the count from the result

    mysql_free_result(result);
    return numSubjects;
}


void Staff::updateCgpa(Database& db, const vector<int>& studentIds, const string& branchCode, int semester, const string& section, const string& subCode, int num_subjects)
{
    MYSQL* conn = db.getConnection();
    vector<int>cgpaUpdateFailedStu;
    for (int studentId : studentIds) 
    {
        // Fetch the mid_marks and end_marks for the student for the given subject
        string query = "SELECT mid_marks, end_marks FROM marks WHERE student_id = " + to_string(studentId) +
            " AND branch_code = '" + branchCode + "'" +
            " AND semester = " + to_string(semester) +
            " AND section = '" + section + "'" +
            " AND sub_code = '" + subCode + "'";

        if (mysql_query(conn, query.c_str())) 
        {
            cerr << "MySQL query error: " << mysql_error(conn) << endl;
            cgpaUpdateFailedStu.push_back(studentId);
            continue;
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if (res == nullptr) 
        {
            cerr << "MySQL store result error: " << mysql_error(conn) << endl;
            cgpaUpdateFailedStu.push_back(studentId);
            continue;
        }

        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row) 
        {
            cgpaUpdateFailedStu.push_back(studentId);
            mysql_free_result(res);
            continue;
        }

        int mid_marks = atoi(row[0]);
        int end_marks = atoi(row[1]);

        mysql_free_result(res);
        // Calculate the new CGPA component for this subject
        float new_cgpa_sub = (((static_cast<float>(mid_marks) + static_cast<float>(end_marks)) / 125.00f) * 10.0f) / num_subjects;

        // Fetch the current CGPA
        query = "SELECT cgpa FROM student WHERE student_id = " + to_string(studentId);
        if (mysql_query(conn, query.c_str())) {
            cerr << "MySQL query error: " << mysql_error(conn) << endl;
            cgpaUpdateFailedStu.push_back(studentId);
            continue;
        }

        res = mysql_store_result(conn);
        if (res == nullptr) {
            cerr << "MySQL store result error: " << mysql_error(conn) << endl;
            cgpaUpdateFailedStu.push_back(studentId);
            continue;
        }

        row = mysql_fetch_row(res);
        if (!row) 
        {
            mysql_free_result(res);
            cgpaUpdateFailedStu.push_back(studentId);
            continue;
        }

        float current_cgpa = stof(row[0]);

        mysql_free_result(res);

        // Update the CGPA
        float updated_cgpa = current_cgpa + new_cgpa_sub;

        query = "UPDATE student SET cgpa = " + to_string(updated_cgpa) +
            " WHERE student_id = " + to_string(studentId);

        if (mysql_query(conn, query.c_str()))
        {
            cerr << "MySQL update error: " << mysql_error(conn) << endl;
            cgpaUpdateFailedStu.push_back(studentId);
            continue;
        }

        // Query to select the CGPA
        query = "SELECT cgpa FROM student WHERE student_id = " + to_string(studentId) + " AND semester = " + to_string(semester);
        if (mysql_query(conn, query.c_str()))
        {
            cerr << "Error executing query: " << mysql_error(conn) << endl;
            return;
        }
        res = mysql_store_result(conn);
        if (!res)
        {
            cerr << "Error storing result: " << mysql_error(conn) << endl;
            return;
        }

        MYSQL_ROW row1 = mysql_fetch_row(res);
        if (!row1) {
            cerr << "No CGPA found for student_id: " << studentId << " in semester: " << semester << endl;
            mysql_free_result(res);
            return;
        }

        // Store the CGPA in a float variable
        float cgpa = atof(row1[0]);

        mysql_free_result(res);

        // Query to update the CGPA in the marks table
        string updateQuery = "UPDATE cgpa SET cgpa = " + to_string(cgpa) + 
            " WHERE student_id = " + to_string(studentId) +" AND semester = "+ to_string(semester);
        if (mysql_query(conn, updateQuery.c_str())) {
            cerr << "Error updating CGPA in marks table: " << mysql_error(conn) << endl;
            return;
        }

    }
    if (cgpaUpdateFailedStu.size() > 0)
    {
        cout << "Number of Student Cgpa Coudn't be Updated: " << cgpaUpdateFailedStu.size();
        cout << "\nStudent Ids : ";
        for (int j = 0; j < cgpaUpdateFailedStu.size(); j++)
        {
            cout << cgpaUpdateFailedStu[j] << " , ";
        }
    }
    cout << "\n";
}

int Staff::getPresentFromAttendance(Database& db, int studentId, const string& branchCode, int semester, const string& section, const string& subCode) 
{
    MYSQL* conn = db.getConnection();
    MYSQL_RES* res;
    MYSQL_ROW row;

    string query = "SELECT present FROM attendance WHERE student_id = " + to_string(studentId) +
        " AND branch_code = '" + branchCode + "'" +
        " AND semester = " + to_string(semester) +
        " AND section = '" + section + "'" +
        " AND sub_code = '" + subCode + "'";

    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return -1;
    }

    res = mysql_store_result(conn);
    if (!res) {
        cerr << "Store result failed: " << mysql_error(conn) << endl;
        return -1;
    }

    row = mysql_fetch_row(res);
    if (!row) {
        mysql_free_result(res);
        return -1; // No record found
    }

    int present = atoi(row[0]);
    mysql_free_result(res);

    return present;
}

string Staff::getStaffSection(Database& db, int staffId, const string& subCode)
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

// Get branch code from staff ID
string Staff::getBranchCodeFromStaffId(Database& db,int staffId) 
{
    MYSQL* conn = db.getConnection();
    string query = "SELECT branch_code FROM staff WHERE staff_id = " + to_string(staffId);
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return "";
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        cerr << "Failed to store result: " << mysql_error(conn) << endl;
        return "";
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    string branchCode = "";
    if (row) {
        branchCode = row[0];
    }

    mysql_free_result(res);
    return branchCode;
}

vector<string> Staff::getSubCodes(Database& db, string& branchCode, int semester)
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

bool Staff::staffSemesterSubTeach(Database& db, int staffId, int semester, string& branchCode)
{
    MYSQL* conn = db.getConnection();
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return false;
    }
    vector<string> subCodes = getSubCodes(db, branchCode, semester);
    if (subCodes.empty())
    {
        cerr << "No sub_codes found for the given branch_code and semester" << endl;
        return false;
    }
    for (const string& subCode : subCodes) {
        // Construct the query to check if the staff member teaches this sub_code
        string checkQuery = "SELECT COUNT(*) FROM staffid_subcode WHERE staff_id = " + to_string(staffId)
            + " AND sub_code = '" + subCode + "'";
        if (mysql_query(conn, checkQuery.c_str())) {
            cerr << "Error in checking staff sub_code: " << mysql_error(conn) << endl;
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
        if (count > 0) {
            return true;
        }
    }
    return false;
}
vector<int> Staff::getRollNo(Database& db, string& branch_code, int semester, string& section)
{
    MYSQL* conn = db.getConnection();
    vector<int> roll_numbers;
    // Construct the SQL query to select roll numbers
    std::string query = "SELECT roll_no FROM student WHERE branch_code = '" + branch_code +
        "' AND semester = " + to_string(semester) +
        " AND section = '" + section + "'";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in retrieving roll numbers: " << mysql_error(conn) << endl;
        return roll_numbers;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return roll_numbers;
    }
    // Fetch the rows and add roll numbers to the vector
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        roll_numbers.push_back(stoi(row[0]));
    }
    // Free the result
    mysql_free_result(res);
    return roll_numbers;
}
vector<int> Staff::getStudentId(Database& db, string& branch_code, int semester, string& section)
{
    MYSQL* conn = db.getConnection();
    vector<int> student_ids;
    // Construct the SQL query to select student IDs
    string query = "SELECT student_id FROM student WHERE branch_code = '" + branch_code +
        "' AND semester = " + to_string(semester) +
        " AND section = '" + section + "' ORDER BY roll_no";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in retrieving student IDs: " << mysql_error(conn) << endl;
        return student_ids;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return student_ids;
    }
    // Fetch the rows and add student IDs to the vector
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)))
    {
        student_ids.push_back(stoi(row[0]));
    }
    // Free the result
    mysql_free_result(res);
    return student_ids;
}
bool Staff::checkStudentExists(Database& db, int studentId)
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
bool Staff::checkSubExists(Database& db, string& subCode)
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
bool Staff::checkSemesterExists(Database& db, string& branchCode, int semester)
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
bool Staff::checkBranchExists(Database& db, string& BranchCode)
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
bool Staff::checkSectionExists(Database& db, string& branchCode, int semester, string& section)
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

void Staff::showStaffSubjects(Database& db, string staff_id_str)
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
            cout << "Subcode: " << subname_row[0] << " , Subname: " << subname_row[1] << " , Section: " << section << endl;
        }
        mysql_free_result(subname_res);
    }
    mysql_free_result(subcode_res);
}

void Staff::viewStaffData(Database& db)
{
    MYSQL* conn = db.getConnection();
    // Query to select required fields from staff table
    string selectQuery = "SELECT username, real_name, dob, branch, joining_date, num_of_subjects, staff_id, branch_code FROM staff";
    if (mysql_query(conn, selectQuery.c_str())) {
        cerr << "Error in retrieving staff data: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row;
    cout << left;
    cout << setw(15) << "Username"
        << setw(20) << "Real Name"
        << setw(12) << "DOB"
        << setw(35) << "Branch"
        << setw(15) << "Joining Date"
        << setw(17) << "Num of Subjects"
        << setw(10) << "Staff ID"
        << setw(10) << "Branch Code"
        << endl;
    cout << "------------------------------------------------------------------------------------------------------------" << endl;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        string username = row[0];
        string realName = row[1];
        string dob = row[2];
        string branch = row[3];
        string joiningDate = row[4];
        int numOfSubjects = atoi(row[5]);
        int staffId = atoi(row[6]);
        string branchCode = row[7];
        cout << setw(15) << username
            << setw(20) << realName
            << setw(12) << dob
            << setw(35) << branch
            << setw(15) << joiningDate
            << setw(17) << numOfSubjects
            << setw(10) << staffId
            << setw(10) << branchCode
            << endl;
    }
    mysql_free_result(res);
}
void Staff::viewStudentDataOnBranchSem(Database& db,string &username)
{
    string branchCode;
    int semester;
    int staffId = getStaffIdByUsername(db, username);
    branchCode = getBranchCodeFromStaffId(db, staffId);
    cout << "Enter the Semester: ";
    cin >> semester;
    cin.ignore();
    if (!(checkSemesterExists(db, branchCode, semester)))
    {
        cout << "This Branch Doesn't Have this semester!" << endl;
        return;
    }
    MYSQL* conn = db.getConnection();
    // Query to select required fields from student table and join with courseid_branchcode table to get branch name
    string selectQuery = "SELECT s.roll_no, s.real_name, s.address, s.course, cb.branch, s.section, s.student_id ,s.cgpa "
        "FROM student s "
        "JOIN courseid_branchcode cb ON s.branch_code = cb.branch_code "
        "WHERE s.branch_code = '" + branchCode + "'";
    if (mysql_query(conn, selectQuery.c_str()))
    {
        cerr << "Error in retrieving student data: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row;
    // Print table headers
    cout << "Student Data\n";
    cout << left << setw(10) << "Roll No"
        << left << setw(30) << "Real Name"
        << left << setw(50) << "Address"
        << left << setw(30) << "Course"
        << left << setw(40) << "Branch"
        << left << setw(10) << "Semester"
        << left << setw(10) << "Section"
        << left << setw(10) << "Student ID"
        << left << setw(10) << "CGPA" << endl;
    // Print a separator
    cout << string(130, '-') << endl;
    // Print table rows
    while ((row = mysql_fetch_row(res)) != nullptr) {
        int rollNo = atoi(row[0]);
        string realName = row[1];
        string address = row[2];
        string course = row[3];
        string branch = row[4];
        string section = row[5];
        int studentId = atoi(row[6]);
        cout << left << setw(10) << rollNo
            << left << setw(30) << realName
            << left << setw(50) << address
            << left << setw(30) << course
            << left << setw(40) << branch
            << left << setw(10) << semester
            << left << setw(10) << section
            << left << setw(10) << studentId
            << left << setw(10) << row[7] << endl;
    }
    mysql_free_result(res);
}
void Staff::viewStudentDataOnSec(Database& db, string& username)
{
    MYSQL* conn = db.getConnection();
    int staffId = getStaffIdByUsername(db, username);
    
    string branchCode, section;
    int semester;
    branchCode = getBranchCodeFromStaffId(db, staffId);
    cout << "Enter the Semester: ";
    cin >> semester;
    cin.ignore();
    if (!(checkSemesterExists(db, branchCode, semester)))
    {
        cout << "This Branch Doesn't Have this semester!" << endl;
        return;
    }
    cout << "Enter the Section: ";
    cin >> section;
    if (!(checkSectionExists(db, branchCode, semester, section)))
    {
        cout << "No Section of that Exists or That Section that no Students!!" << endl;
        return;
    }
    string selectQuery = "SELECT real_name, address, branch, section, student_id, semester, cgpa , roll_no "
        "FROM student "
        "WHERE branch_code = '" + branchCode + "' AND semester = " + to_string(semester) + " AND section = '" + section + "'";
    // Execute the query
    if (mysql_query(conn, selectQuery.c_str())) {
        cerr << "Error in fetching student data: " << mysql_error(conn) << endl;
        return;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    // Print the table header
    cout << left << setw(30) << "Real Name"
        << setw(50) << "Address"
        << setw(30) << "Branch"
        << setw(10) << "Section"
        << setw(10) << "Roll No"
        << setw(15) << "Student ID"
        << setw(10) << "Semester"
        << setw(5) << "CGPA" << endl;
    cout << string(110, '-') << endl; // Print a separator line
    // Fetch and print the rows
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        // Print the student data
        cout << left << setw(30) << row[0]
            << setw(50) << row[1]
            << setw(30) << row[2]
            << setw(10) << row[3]
            << setw(10) << row[7]
            << setw(15) << row[4]
            << setw(10) << row[5]
            << setw(5) << row[6] << endl;
    }
    // Free the result set
    mysql_free_result(res);
}
void Staff::viewSpecificStudentData(Database& db)
{
    MYSQL* conn = db.getConnection();
    int studentId;
    cout << "Enter the Student ID: ";
    cin >> studentId;
    cin.ignore();
    if (!(checkStudentExists(db, studentId)))
    {
        cout << "Wrong StudentId ! Enter a Valid StudentId.." << endl;
        return;
    }
    // Construct the SQL query
    string selectQuery = "SELECT real_name, address, branch, section, student_id, semester, cgpa ,roll_no "
        "FROM student "
        "WHERE student_id = " + to_string(studentId);
    // Execute the query
    if (mysql_query(conn, selectQuery.c_str())) {
        cerr << "Error in fetching student data: " << mysql_error(conn) << endl;
        return;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    // Print the table header
    cout << left << setw(30) << "Real Name"
        << setw(50) << "Address"
        << setw(30) << "Branch"
        << setw(10) << "Section"
        << setw(10) << "Roll No"
        << setw(15) << "Student ID"
        << setw(10) << "Semester"
        << setw(5) << "CGPA" << endl;
    cout << string(110, '-') << endl; // Print a separator line
    // Fetch and print the row
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        // Print the student data
        cout << left << setw(30) << row[0]
            << setw(50) << row[1]
            << setw(30) << row[2]
            << setw(10) << row[3]
            << setw(10) << row[7]
            << setw(15) << row[4]
            << setw(10) << row[5]
            << setw(5) << row[6] << endl;
    }
    // Free the result set
    mysql_free_result(res);
}



void Staff::viewStaffOwnData(Database& db, string& username)
{
    MYSQL* conn = db.getConnection();
    string query = "SELECT * FROM staff WHERE username='" + username + "'";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in selecting staff: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    if (!(row = mysql_fetch_row(res))) {
        cerr << "No staff found with username: " << username << endl;
        mysql_free_result(res);
        return;
    }
    cout << left;
    cout << setw(15) << "Username"
        << setw(30) << "Real Name"
        << setw(12) << "DOB"
        << setw(30) << "Branch"
        << setw(12) << "Salary"
        << setw(50) << "Address"
        << setw(15) << "Joining Date"
        << setw(16) << "Num of Subjects"
        << setw(10) << "Staff ID"
        << setw(12) << "Branch Code"
        << endl;
    cout << string(110, '-') << endl;
    cout << setw(15) << row[0]
        << setw(30) << row[2]  // Real Name
        << setw(12) << row[3]  // DOB
        << setw(30) << row[4]  // Branch
        << setw(12) << row[5]  // Salary
        << setw(50) << row[6]  // Address
        << setw(15) << row[7]  // Joining Date
        << setw(16) << row[8]  // Number of Subjects
        << setw(10) << row[9]  // Staff ID
        << setw(12) << row[10]  // Branch Code
        << endl;
    cout << "----Subejects Teaching----\n";
    showStaffSubjects(db, row[9]);
    cout << "\n";
    mysql_free_result(res);
}

bool Staff::checkStaffBranch(Database& db, string& branchCode, string& username)
{
    MYSQL* conn = db.getConnection();
    // Check if the connection is valid
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return false;
    }
    // Construct the query to check if the username and branch_code combination exists in the staff table
    string checkQuery = "SELECT COUNT(*) FROM staff WHERE username = '" + username + "' AND branch_code = '" + branchCode + "'";
    // Execute the query
    if (mysql_query(conn, checkQuery.c_str())) {
        cerr << "Error in checking staff branch: " << mysql_error(conn) << endl;
        return false;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return false;
    }
    // Fetch the row
    MYSQL_ROW row = mysql_fetch_row(res);
    bool exists = row && atoi(row[0]) > 0;
    // Free the result
    mysql_free_result(res);
    return exists;
}
int Staff::total_students(Database& db, string& branchCode, int semester, string& section)
{
    MYSQL* conn = db.getConnection();
    // Check if the connection is valid
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return -1; // Indicate an error with a negative number
    }
    // Construct the query to count the number of students
    string countQuery = "SELECT COUNT(*) FROM student WHERE branch_code = '" + branchCode
        + "' AND section = '" + section + "' AND semester = " + to_string(semester);
    // Execute the query
    if (mysql_query(conn, countQuery.c_str())) {
        cerr << "Error in counting students: " << mysql_error(conn) << endl;
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
    int totalStudents = row ? atoi(row[0]) : 0;
    // Free the result
    mysql_free_result(res);
    return totalStudents;
}

vector<int> Staff::getRollNoFromAttendance(Database& db, const string& branchCode, int semester, const string& section, const string& subCode) 
{
    MYSQL* conn = db.getConnection();
    vector<int> rollNos;
    string query = "SELECT roll_no FROM total_attendance WHERE branch_code = '" + branchCode
        + "' AND semester = " + to_string(semester)
        + " AND section = '" + section
        + "' AND sub_code = '" + subCode + "'";

    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return rollNos;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        cerr << "Failed to store result: " << mysql_error(conn) << endl;
        return rollNos;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        rollNos.push_back(atoi(row[0]));
    }

    mysql_free_result(res);
    return rollNos;
}

int Staff::getStaffIdByUsername(Database& db, string& username)
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
void Staff::totalAttendanceUpdate(Database& db, string& branchCode, int semester, string& section, string& subCode)
{
    MYSQL* conn = db.getConnection();
    // Check if the connection is valid
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return;
    }

    // Query to check if the student already exists in the attendance table
    string attendanceCheckQuery = "SELECT COUNT(*) FROM total_attendance WHERE branch_code = '" + branchCode + "' AND section = '" + section
        + "' AND semester = " + to_string(semester) +" AND sub_code = '"+ subCode+"'";

    // Execute the attendance check query
    if (mysql_query(conn, attendanceCheckQuery.c_str())) {
        cerr << "Error in checking attendance: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* attendanceCheckRes = mysql_store_result(conn);
    if (attendanceCheckRes == nullptr) 
    {
        cerr << "Error in storing attendance check result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW attendanceCheckRow = mysql_fetch_row(attendanceCheckRes);
    int count = attendanceCheckRow ? atoi(attendanceCheckRow[0]) : 0;
    
    mysql_free_result(attendanceCheckRes);
    if (count == 0)
    {
        string insertQuery = "INSERT INTO total_attendance (branch_code, semester, section, sub_code, total) VALUES ('"
            + branchCode + "', " + to_string(semester) + ", '" + section + "', '" + subCode + "', " + to_string(1) + ")";
        if (mysql_query(conn, insertQuery.c_str()))
        {
            cerr << "Error in inserting total attendance: " << mysql_error(conn) << endl;
            return;
        }
    }
    else
    {
        string updateQuery = "UPDATE total_attendance SET total = total + 1 WHERE branch_code = '" + branchCode
            + "' AND semester = " + to_string(semester)
            + " AND section = '" + section
            + "' AND sub_code = '" + subCode + "'";
        if (mysql_query(conn, updateQuery.c_str())) {
            cerr << "Error in updating total attendance: " << mysql_error(conn) << endl;
            return;
        }
    }

}

int Staff::getTotalAttendance(Database& db, string& branchCode, int semester, string& section, string& subCode)
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

bool Staff::checkStaffSubTeach(Database& db, string& subCode, string& section, int staffId)
{
    MYSQL* conn = db.getConnection();
    // Check if the connection is valid
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return false;
    }
    // Construct the query to check if the combination of sub_code, section, and staff_id exists
    string query = "SELECT COUNT(*) FROM staffid_subcode WHERE sub_code = '" + subCode + "' AND section = '" + section
        + "' AND staff_id = " + to_string(staffId);
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in checking staff subcode: " << mysql_error(conn) << endl;
        return false;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return false;
    }
    // Fetch the row
    MYSQL_ROW row = mysql_fetch_row(res);
    bool exists = row && atoi(row[0]) > 0;
    // Free the result
    mysql_free_result(res);
    return exists;
}
bool Staff::checkStudentRollExist(Database& db, string& branchCode, int semester, string& section, int rollNo)
{
    MYSQL* conn = db.getConnection();
    // Check if the connection is valid
    if (conn == nullptr) {
        cerr << "Database connection error" << endl;
        return false;
    }
    // Construct the query to check if the student exists
    string query = "SELECT COUNT(*) FROM student WHERE branch_code = '" + branchCode
        + "' AND semester = " + to_string(semester)
        + " AND section = '" + section
        + "' AND roll_no = " + to_string(rollNo);
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in checking student roll: " << mysql_error(conn) << endl;
        return false;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return false;
    }
    // Fetch the row
    MYSQL_ROW row = mysql_fetch_row(res);
    bool exists = row && atoi(row[0]) > 0;
    // Free the result
    mysql_free_result(res);
    return exists;
}
void Staff::addAttendance(Database& db, string& username)
{
    MYSQL* conn = db.getConnection();
    string subcode, section, branchCode;
    int semester;
    int staffId = getStaffIdByUsername(db, username);
    branchCode = getBranchCodeFromStaffId(db,staffId);
    cout << "Enter the Semester: ";
    cin >> semester;
    if (!(checkSemesterExists(db, branchCode, semester)))
    {
        cout << "This semester doesn't exists" << endl;
        return;
    }
    if (!(staffSemesterSubTeach(db, staffId, semester, branchCode)))
    {
        cout << "You don't Teach any subjects to this semester!!\n";
        return;
    }
    cout << "\nEnter the section: ";
    cin >> section;
    if (!(checkSectionExists(db, branchCode, semester, section)))
    {
        cout << "Section Does't Exists!!!" << endl;
        return;
    }
    cout << "\nEner the subcode of the subject: ";
    cin >> subcode;
    if (!(checkStaffSubTeach(db, subcode, section, staffId)))
    {
        cout << "Sorry but you do not teach this subject to this Sectioin.!" << endl;
        return;
    }
    int totalStudents = total_students(db, branchCode, semester, section);
    int present;
    cout << "\nTotal Number of Students in Sec " + section << " = " << totalStudents << endl;
    cout << "Enter the number of present Students: ";
    cin >> present;
    vector<int> presentStu;
    if (present <= totalStudents)
    {
        int temproll;
        cout << "Enter the rollno of " + present << " students: " << endl;
        for (int i = 0; i < present; i++)
        {
            cin >> temproll;
            if (!(checkStudentRollExist(db, branchCode, semester, section, temproll)))
            {
                cout << "Roll No: " << temproll<< " is not registered or not exists! " << endl;
                continue;
            }
            presentStu.push_back(temproll);
        }

        // Check if the connection is valid
        if (conn == nullptr) {
            cerr << "Database connection error" << endl;
            return;
        }

        // Query to get all students with the given criteria
        // Query to get all students with the given criteria
        string studentQuery = "SELECT student_id,roll_no FROM student WHERE branch_code = '" + branchCode
            + "' AND section = '" + section + "' AND semester = " + to_string(semester)
            + " AND roll_no IN (";

        for (size_t i = 0; i < presentStu.size(); ++i) {
            studentQuery += to_string(presentStu[i]);
            if (i < presentStu.size() - 1) {
                studentQuery += ", ";
            }
        }
        studentQuery += ")";

        vector<int>presentEnd;

        // Execute the student query
        if (mysql_query(conn, studentQuery.c_str())) {
            cerr << "Error in retrieving students: " << mysql_error(conn) << endl;
            return;
        }
        MYSQL_RES* studentRes = mysql_store_result(conn);
        if (studentRes == nullptr) {
            cerr << "Error in storing student result: " << mysql_error(conn) << endl;
            return;
        }
        totalAttendanceUpdate(db, branchCode, semester, section, subcode);
        int total_attendance = getTotalAttendance(db, branchCode, semester, section, subcode);
        MYSQL_ROW studentRow;

        while ((studentRow = mysql_fetch_row(studentRes)))
        {
            int studentId = atoi(studentRow[0]);
            int rollNo = atoi(studentRow[1]);


            // Query to check if the student already exists in the attendance table
            string attendanceCheckQuery = "SELECT COUNT(*) FROM attendance WHERE student_id = " + to_string(studentId)
                + " AND branch_code = '" + branchCode + "' AND section = '" + section
                + "' AND semester = " + to_string(semester);
            // Execute the attendance check query
            if (mysql_query(conn, attendanceCheckQuery.c_str())) {
                cerr << "Error in checking attendance: " << mysql_error(conn) << endl;
                mysql_free_result(studentRes);
                return;
            }
            MYSQL_RES* attendanceCheckRes = mysql_store_result(conn);
            if (attendanceCheckRes == nullptr) {
                cerr << "Error in storing attendance check result: " << mysql_error(conn) << endl;
                mysql_free_result(studentRes);
                return;
            }
            MYSQL_ROW attendanceCheckRow = mysql_fetch_row(attendanceCheckRes);
            int count = attendanceCheckRow ? atoi(attendanceCheckRow[0]) : 0;
            mysql_free_result(attendanceCheckRes);
            if (count == 0)
            {
                // Insert the student into the attendance table if they do not exist
                string insertQuery = "INSERT INTO attendance (staff_id,student_id, branch_code, semester, section, sub_code, present, roll_no) VALUES ("+ to_string(staffId) +","
                    + to_string(studentId) + ", '" + branchCode + "', " + to_string(semester)
                    + ", '" + section + "', '" + subcode + "', 1, "  + to_string(rollNo) + ")";
                if (mysql_query(conn, insertQuery.c_str())) {
                    cerr << "Error in inserting attendance: " << mysql_error(conn) << endl;
                    mysql_free_result(studentRes);
                    return;
                }
            }
            else
            {
                string updateQuery = "UPDATE attendance SET present = present + 1 WHERE student_id = " + to_string(studentId)
                    + " AND branch_code = '" + branchCode + "' AND section = '" + section
                    + "' AND semester = " + to_string(semester) + " AND sub_code = '" + subcode + "'";
                if (mysql_query(conn, updateQuery.c_str()))
                {
                    cerr << "Error in updating attendance: " << mysql_error(conn) << endl;
                    mysql_free_result(studentRes);
                    return;
                }
            }
            presentEnd.push_back(rollNo);
        }
        cout << "Present List: ";
        for (int x : presentEnd)
        {
            cout << x << "  ";
        }
        // Free the student result
        mysql_free_result(studentRes);
    }
    else
    {
        cout << "Present student can't be greater than total:" << endl;
    }
}
void Staff::incrementCustomAttendance(Database& db, string& username)
{
    MYSQL* conn = db.getConnection();
    string subcode, section, branchCode;
    int semester;
    int staffId = getStaffIdByUsername(db, username);
    branchCode = getBranchCodeFromStaffId(db, staffId);
    cout << "Enter the Semester: ";
    cin >> semester;
    if (!(checkSemesterExists(db, branchCode, semester)))
    {
        cout << "This semester doesn't exists" << endl;
        return;
    }
    cout << "\nEnter the section: ";
    cin >> section;
    if (!(checkSectionExists(db, branchCode, semester, section)))
    {
        cout << "Section Does't Exists!!!" << endl;
        return;
    }
    cout << "\nEner the subcode of the subject: ";
    cin >> subcode;
    
    if (!(checkStaffSubTeach(db, subcode, section, staffId)))
    {
        cout << "Sorry but you do not teach this subject to this Sectioin.!" << endl;
        return;
    }
    int totalStudents = total_students(db, branchCode, semester, section);
    int present;
    cout << "\nEnter Number of Students to Increment of Sec:  " + section + " : ";
    cin >> present;
    cin.ignore();
    vector<int> presentStu;
    if (present <= totalStudents)
    {
        int temproll;
        cout << "\nEnter the rollno of " + present << " students: " << endl;
        for (int i = 0; i < present; i++)
        {
            cout << i + 1 << ": ";
            cin >> temproll;
            if (!(checkStudentRollExist(db, branchCode, semester, section, temproll)))
            {
                cout << "Roll No: " << temproll << " is not registered or not exists! " << endl;
                continue;
            }
            presentStu.push_back(temproll);
        }

        // Check if the connection is valid
        if (conn == nullptr) {
            cerr << "Database connection error" << endl;
            return;
        }

        // Query to get all students with the given criteria
        // Query to get all students with the given criteria
        string studentQuery = "SELECT student_id,roll_no FROM student WHERE branch_code = '" + branchCode
            + "' AND section = '" + section + "' AND semester = " + to_string(semester)
            + " AND roll_no IN (";

        for (size_t i = 0; i < presentStu.size(); ++i) {
            studentQuery += to_string(presentStu[i]);
            if (i < presentStu.size() - 1) {
                studentQuery += ", ";
            }
        }
        studentQuery += ")";

        // Execute the student query
        if (mysql_query(conn, studentQuery.c_str())) {
            cerr << "Error in retrieving students: " << mysql_error(conn) << endl;
            return;
        }
        MYSQL_RES* studentRes = mysql_store_result(conn);
        if (studentRes == nullptr) 
        {
            cerr << "Error in storing student result: " << mysql_error(conn) << endl;
            return;
        }
        int total_attendance = getTotalAttendance(db, branchCode, semester, section, subcode);
        MYSQL_ROW studentRow;

        while ((studentRow = mysql_fetch_row(studentRes)))
        {
            int studentId = atoi(studentRow[0]);
            int rollNo = atoi(studentRow[1]);


            // Query to check if the student already exists in the attendance table
            string attendanceCheckQuery = "SELECT COUNT(*) FROM attendance WHERE student_id = " + to_string(studentId)
                + " AND branch_code = '" + branchCode + "' AND section = '" + section
                + "' AND semester = " + to_string(semester);
            // Execute the attendance check query
            if (mysql_query(conn, attendanceCheckQuery.c_str())) {
                cerr << "Error in checking attendance: " << mysql_error(conn) << endl;
                mysql_free_result(studentRes);
                return;
            }
            MYSQL_RES* attendanceCheckRes = mysql_store_result(conn);
            if (attendanceCheckRes == nullptr) {
                cerr << "Error in storing attendance check result: " << mysql_error(conn) << endl;
                mysql_free_result(studentRes);
                return;
            }
            MYSQL_ROW attendanceCheckRow = mysql_fetch_row(attendanceCheckRes);
            int count = attendanceCheckRow ? atoi(attendanceCheckRow[0]) : 0;
            mysql_free_result(attendanceCheckRes);

            int total_attendance = getTotalAttendance(db, branchCode, semester, section, subcode);
            int presen =0;
            if (count == 0)
            {
                if (total_attendance == 0)
                {
                    presen = 0;
                }
                else
                {
                    presen = 1;
                }
                // Insert the student into the attendance table if they do not exist
                string insertQuery = "INSERT INTO attendance (staff_id,student_id, branch_code, semester, section, sub_code, present, roll_no) VALUES (" + to_string(staffId) + ","
                    + to_string(studentId) + ", '" + branchCode + "', " + to_string(semester)
                    + ", '" + section + "', '" + subcode + "',"+ to_string(presen) +", " + to_string(rollNo) + ")";
                if (mysql_query(conn, insertQuery.c_str())) 
                {
                    cerr << "Error in inserting attendance: " << mysql_error(conn) << endl;
                    mysql_free_result(studentRes);
                    return;
                }
            }
            else
            {
                presen = getPresentFromAttendance(db, studentId, branchCode, semester, section, subcode);
                if ((presen + 1) > total_attendance)
                {
                    cout << "StudentId : " << studentId << " RollNo :"<< rollNo << " already has Total Attendance of this Subject.Can't increment more!!\n";  
                }
                else
                {
                    string updateQuery = "UPDATE attendance SET present = present + 1 WHERE student_id = " + to_string(studentId)
                        + " AND branch_code = '" + branchCode + "' AND section = '" + section
                        + "' AND semester = " + to_string(semester) + " AND sub_code = '" + subcode + "'";
                    if (mysql_query(conn, updateQuery.c_str()))
                    {
                        cerr << "Error in updating attendance: " << mysql_error(conn) << endl;
                        mysql_free_result(studentRes);
                        return;
                    }
                } 
            }
        }
        // Free the student result
        mysql_free_result(studentRes);
    }
    else
    {
        cout << "Present student can't be greater than total:" << endl;
    }
}
int Staff::getNoOfTotalStudentInSec(Database& db, string& branch_code, int semester, string& section)
{
    MYSQL* conn = db.getConnection();
    string query = "SELECT COUNT(*) FROM student WHERE branch_code = '" + branch_code +
        "' AND semester = " + to_string(semester) + " AND section = '" + section + "'";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in retrieving the number of students: " << mysql_error(conn) << endl;
        return 0;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return 0;
    }
    // Fetch the row and display the count
    MYSQL_ROW row = mysql_fetch_row(res);
    int student_count = 0;
    if (row)
    {
        student_count = stoi(row[0]);
    }
    else
    {
        cerr << "Error in fetching the student count." << endl;
    }
    // Free the result
    mysql_free_result(res);
    return student_count;
}


void Staff::showStudentMarks(Database& db, string& branch_code, int semester, string& section, string& sub_code, vector<int>& student_ids, vector<int>& rollNo)
{
    MYSQL* conn = db.getConnection();
    if (student_ids.empty())
    {
        cerr << "No student IDs provided." << std::endl;
        return;
    }
    vector<int> mid_marks_vec(student_ids.size());
    vector<int> end_marks_vec(student_ids.size());
    for (size_t i = 0; i < student_ids.size(); ++i)
    {
        int student_id = student_ids[i];
        // Construct the SQL query for each student ID
        string query = "SELECT mid_marks, end_marks FROM marks WHERE student_id = " + to_string(student_id) +
            " AND branch_code = '" + branch_code + "' AND semester = " + to_string(semester) +
            " AND section = '" + section + "' AND sub_code = '" + sub_code + "'";
        // Execute the query
        if (mysql_query(conn, query.c_str())) {
            cerr << "Error in retrieving marks for student ID " << student_id << ": " << mysql_error(conn) << endl;
            continue;
        }
        // Store the result
        MYSQL_RES* res = mysql_store_result(conn);
        if (res == nullptr) {
            cerr << "Error in storing result for student ID " << student_id << ": " << mysql_error(conn) << endl;
            continue;
        }
        // Fetch the row and store the marks
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row)
        {
            mid_marks_vec[i] = stoi(row[0]);
            end_marks_vec[i] = stoi(row[1]);
        }
        // Free the result
        mysql_free_result(res);
    }

    //Display Marks
    cout << left << setw(15) << "Student ID"
        << left << setw(12) << "Branch Code"  
        << left << setw(15) << "Semester" 
        << left << setw(15) << "Section"  
        << left << setw(15) << "RollNo" 
        << left << setw(15) << "Sub Code" 
        << left << setw(12) << "Mid Marks" 
        << left << setw(12) << "End Marks" << endl;
    cout << string(112, '-') << endl;

    for (size_t i = 0; i < student_ids.size(); ++i)
    {
        cout << left << setw(15) << student_ids[i] 
            << left << setw(12) << branch_code 
            << left << setw(15) << semester 
            << left << setw(15) << section 
            << left << setw(15) << rollNo[i] 
            << left << setw(15) << sub_code 
            << left << setw(12) << mid_marks_vec[i] 
            << left << setw(12) << end_marks_vec[i] << endl;
    }
}
void Staff::viewStudentMarks(Database& db, string& username)
{
    MYSQL* conn = db.getConnection();
    int staffId = getStaffIdByUsername(db, username);
    string branchCode, section, subCode;
    int semester;
    branchCode = getBranchCodeFromStaffId(db, staffId);
    cout << "Enter the Semester: ";
    cin >> semester;
    if (!(checkSemesterExists(db, branchCode, semester)))
    {
        cout << "Semester of this Branch doesn't Exists!" << endl;
        return;
    }
    cout << "Enter the Section: ";
    cin >> section;
    if (!(checkSectionExists(db, branchCode, semester, section)))
    {
        cout << "This Section of this Branch and Semester doesn't exists!!" << endl;
        return;
    }
    cout << "Enter the SubCode: ";
    cin >> subCode;
    if (!(checkSubExists(db, subCode)))
    {
        cout << "This Subject doesn't Exists! Enter a Valid Subject!.." << endl;
        return;
    }
    
    if (!(checkStaffSubTeach(db, subCode, section, staffId)))
    {
        cout << "You do not teach this subject to this section!!" << endl;
        return;
    }
    vector<int>rollNo = getRollNo(db, branchCode, semester, section);
    sort(rollNo.begin(), rollNo.end());
    vector<int>StudentIds = getStudentId(db, branchCode, semester, section);
    showStudentMarks(db, branchCode, semester, section, subCode, StudentIds, rollNo);
}
int Staff::getStudentIdsByRollNo(Database& db, string& branch_code, int semester, string& section, int rollNo)
{
    MYSQL* conn = db.getConnection();
    // Construct the SQL query to retrieve the student_id
    string query = "SELECT student_id FROM student WHERE branch_code = '" + branch_code +
        "' AND semester = " + to_string(semester) + " AND section = '" + section +
        "' AND roll_no = " + to_string(rollNo);
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in retrieving student ID: " << mysql_error(conn) << endl;
        return -1;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return -1;
    }
    // Fetch the row and retrieve the student_id
    MYSQL_ROW row = mysql_fetch_row(res);
    int student_id = -1;
    if (row) {
        student_id = stoi(row[0]);
    }
    else {
        cerr << "No student found with the given roll number." << endl;
    }
    // Free the result
    mysql_free_result(res);
    return student_id;
}
void Staff::viewSpecificStudentMarks(Database& db, string& username)
{
    string branchCode, section, subCode;
    int semester;
    int staffId = getStaffIdByUsername(db, username);
    MYSQL* conn = db.getConnection();
    branchCode = getBranchCodeFromStaffId(db, staffId);
    cout << "Enter the Semester: ";
    cin >> semester;
    if (!(checkSemesterExists(db, branchCode, semester)))
    {
        cout << "Semester of this Branch doesn't Exists!" << endl;
        return;
    }
    cout << "Enter the Section: ";
    cin >> section;
    if (!(checkSectionExists(db, branchCode, semester, section)))
    {
        cout << "This Section of this Branch and Semester doesn't exists!!" << endl;
        return;
    }
    cout << "Enter the SubCode: ";
    cin >> subCode;
    if (!(checkSubExists(db, subCode)))
    {
        cout << "This Subject doesn't Exists! Enter a Valid Subject!.." << endl;
        return;
    }
    
    if (!(checkStaffSubTeach(db, subCode, section, staffId)))
    {
        cout << "You do not teach this subject to this section!!" << endl;
        return;
    }
    int num_stu = 0;
    cout << "Enter no of students marks show: " << endl;
    cin >> num_stu;
    int totalStu = getNoOfTotalStudentInSec(db, branchCode, semester, section);
    if (num_stu > totalStu)
    {
        cout << "Total Number of Student in this section are only: " << totalStu << " : Enter Valid Student Number." << endl;
        return;
    }
    vector<int>rollNo;
    vector<int>studentId;
    int rollTemp;
    cout << "Enter their RollNo: " << endl;
    for (int i = 0; i < num_stu; i++)
    {
        cout << i + 1 << ": ";
        cin >> rollTemp;
        if (!(checkStudentRollExist(db, branchCode, semester, section, rollTemp)))
        {
            cout << "This Roll No: " << rollTemp << " doesn't exists in this section. So it will be Ignored." << endl;
            continue;
        }
        rollNo.push_back(rollTemp);
        int stu_id = getStudentIdsByRollNo(db, branchCode, semester, section, rollTemp);
        studentId.push_back(stu_id);
    }
    showStudentMarks(db, branchCode, semester, section, subCode, studentId, rollNo);
}
void Staff::updateMarks(Database& db, string& branch_code, int semester, string& section, string& sub_code, vector<int>& student_ids, vector<int>& rollNo, string mark_type)
{
    MYSQL* conn = db.getConnection();
    vector<int>sucessUpdatemarks;
    vector<int> UpdateMarksFailedStudent;
    vector<int> UpdateCgpaFailedStudent;
    if (student_ids.size() != rollNo.size())
    {
        cerr << "Mismatch between number of student IDs and roll numbers." << endl;
        return;
    }
    for (size_t i = 0; i < student_ids.size(); ++i)
    {
        int student_id = student_ids[i];
        int roll_no = rollNo[i];
        cout << "Enter " + mark_type+" for RollNo : "<<roll_no<<" : ";
        int temp_marks;
        int new_marks;
        cin >> temp_marks;
        if (mark_type == "end_marks")
        {
            while (temp_marks < 0 || temp_marks > 100)
            {
                cout << "\nMarks Cannot be greater than 100 or less 0!!\n";
                cout << "For Roll No :" << roll_no << " .Enter Marks again : ";
                cin >> temp_marks;
            }
            new_marks = temp_marks;
        }
        else
        {
            while (temp_marks < 0 || temp_marks > 25)
            {
                cout << "\nMarks Cannot be greater than 25 or less 0!!\n";
                cout << "For Roll No :" << roll_no << " .Enter Marks again : ";
                cin >> temp_marks;
            }
            new_marks = temp_marks;
        }

        //Here we Get the Curernt mid_marks and end_marks
        string query = "SELECT mid_marks, end_marks FROM marks WHERE branch_code = '"
            + branch_code + "' AND semester = " + to_string(semester)
            + " AND section = '" + section
            + "' AND student_id = " + to_string(student_id)
            + " AND sub_code = '" + sub_code + "'";

        if (mysql_query(conn, query.c_str())) 
        {
            UpdateMarksFailedStudent.push_back(roll_no);
            cerr << "Query failed: " << mysql_error(conn) << endl;
            continue;
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) 
        {
            UpdateMarksFailedStudent.push_back(roll_no);
            cerr << "Failed to store result: " << mysql_error(conn) << endl;
            continue;
        }
        int old_marks = 0, other_marks=0;
        MYSQL_ROW row;
        int midMarks = 0;
        int endMarks = 0;
        if ((row = mysql_fetch_row(res))) 
        {
            midMarks = atoi(row[0]);
            endMarks = atoi(row[1]);
        }
        else 
        {
            UpdateMarksFailedStudent.push_back(roll_no);
            cout << "No records found for the given criteria." << endl;
            continue;
        }
        mysql_free_result(res);

        if (mark_type == "mid_marks")
        {
            old_marks = midMarks;
            other_marks = endMarks;

        }
        else
        {
            old_marks = endMarks;
            other_marks = midMarks;
        }

        query = "UPDATE marks SET "+ mark_type + " = " + to_string(new_marks) +
            " WHERE student_id = " + to_string(student_id) +
            " AND branch_code = '" + branch_code +
            "' AND semester = " + to_string(semester) +
            " AND section = '" + section +
            "' AND sub_code = '" + sub_code + "'";
        // Execute the query

        if (mysql_query(conn, query.c_str()))
        {
            UpdateMarksFailedStudent.push_back(roll_no);
            cerr << "Error in updating marks for Roll No " << roll_no << ": " << mysql_error(conn) << endl;
            continue;
        }
        else
        {
            cout << "End marks updated for Roll No " << roll_no << " (Student ID: " << student_id << ")" << endl;
        }

        int num_subjects = getNumSubjects(db, branch_code, semester);

        float old_cgpa_sub = (((static_cast<float>(old_marks) + static_cast<float>(other_marks)) / 125.00f) * 10.0f) / num_subjects;

        query = "UPDATE student SET cgpa = cgpa - " + to_string(old_cgpa_sub) + " WHERE student_id = " + to_string(student_id);

        if (mysql_query(conn, query.c_str())) 
        {
            UpdateCgpaFailedStudent.push_back(roll_no);
            cerr << "Error updating CGPA: " << mysql_error(conn) << endl;
            continue;
        }

        float new_cgpa_sub = (((static_cast<float>(new_marks) + static_cast<float>(other_marks)) / 125.00f) * 10.0f) / num_subjects;

        query = "UPDATE student SET cgpa = cgpa + " + to_string(new_cgpa_sub) + " WHERE student_id = " + to_string(student_id);


        if (mysql_query(conn, query.c_str()))
        {
            UpdateCgpaFailedStudent.push_back(roll_no);
            cerr << "Error updating CGPA: " << mysql_error(conn) << endl;
            continue;
        }
        else
        {
            sucessUpdatemarks.push_back(roll_no);
        }

        // Query to select the CGPA
        query = "SELECT cgpa FROM student WHERE student_id = " + to_string(student_id) + " AND semester = " + to_string(semester);
        if (mysql_query(conn, query.c_str())) 
        {
            cerr << "Error executing query: " << mysql_error(conn) << endl;
            return;
        }

        res = mysql_store_result(conn);
        if (!res) 
        {
            cerr << "Error storing result: " << mysql_error(conn) << endl;
            return;
        }

        MYSQL_ROW row1 = mysql_fetch_row(res);
        if (!row1) {
            cerr << "No CGPA found for student_id: " << student_id << " in semester: " << semester << endl;
            mysql_free_result(res);
            return;
        }

        // Store the CGPA in a float variable
        float cgpa = atof(row1[0]);

        mysql_free_result(res);

        // Query to update the CGPA in the marks table
        string updateQuery = "UPDATE cgpa SET cgpa = " + to_string(cgpa) + " WHERE student_id = " + to_string(student_id) + " AND semester = " + to_string(semester);
        if (mysql_query(conn, updateQuery.c_str())) {
            cerr << "Error updating CGPA in marks table: " << mysql_error(conn) << endl;
            return;
        }
    }
    if (UpdateCgpaFailedStudent.size() > 0)
    {
        cout << UpdateCgpaFailedStudent.size() << " Students Cgpa Updation Failed\n";
        cout << "Roll No : ";
        for (int i = 0; i < UpdateCgpaFailedStudent.size(); i++)
        {
            cout << UpdateCgpaFailedStudent[i] << " , ";
        }
        cout << "\n";
    }
    if (UpdateMarksFailedStudent.size() > 0)
    {
        cout << UpdateMarksFailedStudent.size() << " Students Marks Updation Failed\n";
        cout << "Roll No : ";
        for (int i = 0; i < UpdateMarksFailedStudent.size(); i++)
        {
            cout << UpdateMarksFailedStudent[i] << " , ";
        }
        cout << "\n";
    }
    if (sucessUpdatemarks.size() > 0)
    {
        cout << sucessUpdatemarks.size() << " Students Marks Updation Sucessfull\n";
        cout << "Roll No : ";
        for (int i = 0; i < sucessUpdatemarks.size(); i++)
        {
            cout << sucessUpdatemarks[i] << " , ";
        }
        cout << "\n";
    }

}
void Staff::addEndMarks(Database& db, string& branch_code, int semester, string& section, string& sub_code, vector<int>& student_ids, vector<int>& rollNo)
{
    vector<int>sucessUpdatemarks;
    MYSQL* conn = db.getConnection();
    if (student_ids.size() != rollNo.size())
    {
        cerr << "Mismatch between number of student IDs and roll numbers." << endl;
        return;
    }
    for (size_t i = 0; i < student_ids.size(); ++i)
    {
        int student_id = student_ids[i];
        int roll_no = rollNo[i];
        // Ask for mid marks
        int end_marks;
        cout << "Enter End marks for Roll No " << roll_no << ": ";
        // Check if the marks are within the allowed range
        int temp_marks;
        cin >> temp_marks;
        // Check if the marks are within the allowed range
        while (temp_marks < 0 || temp_marks > 100)
        {
            cout << "\nMarks Cannot be greater than 100 or less 0!!\n";
            cout << "For Roll No :" << roll_no << " .Enter Marks again : ";
            cin >> temp_marks;
        }
        end_marks = temp_marks;
        // Construct the SQL query to update the mid marks
        string query = "UPDATE marks SET end_marks = " + to_string(end_marks) +
            " WHERE student_id = " + to_string(student_id) +
            " AND branch_code = '" + branch_code +
            "' AND semester = " + to_string(semester) +
            " AND section = '" + section +
            "' AND sub_code = '" + sub_code + "'";
        // Execute the query
        if (mysql_query(conn, query.c_str()))
        {
            cerr << "Error in updating marks for Student ID " << student_id << ": " << mysql_error(conn) << endl;
        }
        else
        {
            sucessUpdatemarks.push_back(student_id);
            cout << "End marks updated for Roll No " << roll_no << " (Student ID: " << student_id << ")" << endl;
        }
    }
    int num_subjects = getNumSubjects(db, branch_code, semester);
    updateCgpa(db, sucessUpdatemarks,branch_code,semester,section,sub_code,num_subjects);
}

void Staff::addMidMarks(Database& db, string& branch_code, int semester, string& section, string& sub_code, vector<int>& student_ids, vector<int>& rollNo)
{
    MYSQL* conn = db.getConnection();
    if (student_ids.size() != rollNo.size())
    {
        cerr << "Mismatch between number of student IDs and roll numbers." << endl;
        return;
    }
    for (size_t i = 0; i < student_ids.size(); ++i)
    {
        int student_id = student_ids[i];
        int roll_no = rollNo[i];
        // Ask for mid marks
        int mid_marks;
        cout << "Enter mid marks for Roll No " << roll_no << ": ";
        int temp_marks;
        cin >> temp_marks;
        // Check if the marks are within the allowed range
        while(temp_marks < 0 || temp_marks > 25)
        {
            cout << "\nMarks Cannot be greater than 25 or less 0!!\n";
            cout << "For Roll No :"<<roll_no<<" .Enter Marks again : ";
            cin >> temp_marks;
        }
        mid_marks = temp_marks;
        // Construct the SQL query to update the mid marks
        string query = "UPDATE marks SET mid_marks = " + to_string(mid_marks) +
            " WHERE student_id = " + to_string(student_id) +
            " AND branch_code = '" + branch_code +
            "' AND semester = " + to_string(semester) +
            " AND section = '" + section +
            "' AND sub_code = '" + sub_code + "'";
        // Execute the query
        if (mysql_query(conn, query.c_str())) {
            cerr << "Error in updating marks for Student ID " << student_id << ": " << mysql_error(conn) << endl;
        }
        else {
            cout << "Mid marks updated for Roll No " << roll_no << " (Student ID: " << student_id << ")" << endl;
        }
    }
}
void Staff::addMarks(Database& db, string& username, string type)
{
    MYSQL* conn = db.getConnection();
    int staffId = getStaffIdByUsername(db, username);
    string branchCode, section, subCode;
    int semester;
    branchCode = getBranchCodeFromStaffId(db, staffId);
    cout << "Enter the Semester: ";
    cin >> semester;
    if (!(checkSemesterExists(db, branchCode, semester)))
    {
        cout << "Semester of this Branch doesn't Exists!" << endl;
        return;
    }
    cout << "Enter the Section: ";
    cin >> section;
    if (!(checkSectionExists(db, branchCode, semester, section)))
    {
        cout << "This Section of this Branch and Semester doesn't exists!!" << endl;
        return;
    }
    cout << "Enter the SubCode: ";
    cin >> subCode;
    if (!(checkSubExists(db, subCode)))
    {
        cout << "This Subject doesn't Exists! Enter a Valid Subject!.." << endl;
        return;
    }
    
    if (!(checkStaffSubTeach(db, subCode, section, staffId)))
    {
        cout << "You do not teach this subject to this section!!" << endl;
        return;
    }
    vector<int>rollNo = getRollNo(db, branchCode, semester, section);
    sort(rollNo.begin(), rollNo.end());
    vector<int>StudentIds = getStudentId(db, branchCode, semester, section);
    if (type == "mid")
    {
        addMidMarks(db, branchCode, semester, section, subCode, StudentIds, rollNo);
    }
    else if (type == "end")
    {
        addEndMarks(db, branchCode, semester, section, subCode, StudentIds, rollNo);
    }
    else
    {
        cout << "Invalid!!!!!!???" << endl;
        return;
    }
}

void Staff::addSpecificMarks(Database& db, string& username, string type)
{
    MYSQL* conn = db.getConnection();
    string branchCode, section, subCode;
    int staffId = getStaffIdByUsername(db, username);
    int semester;
    branchCode = getBranchCodeFromStaffId(db, staffId);
    cout << "Enter the Semester: ";
    cin >> semester;
    if (!(checkSemesterExists(db, branchCode, semester)))
    {
        cout << "Semester of this Branch doesn't Exists!" << endl;
        return;
    }
    cout << "Enter the Section: ";
    cin >> section;
    if (!(checkSectionExists(db, branchCode, semester, section)))
    {
        cout << "This Section of this Branch and Semester doesn't exists!!" << endl;
        return;
    }
    cout << "Enter the SubCode: ";
    cin >> subCode;
    if (!(checkSubExists(db, subCode)))
    {
        cout << "This Subject doesn't Exists! Enter a Valid Subject!.." << endl;
        return;
    }
    
    if (!(checkStaffSubTeach(db, subCode, section, staffId)))
    {
        cout << "You do not teach this subject to this section!!" << endl;
        return;
    }
    int num_stu = 0;
    cout << "Enter no of students marks to Update/Add" << endl;
    cin >> num_stu;
    int totalStu = getNoOfTotalStudentInSec(db, branchCode, semester, section);
    if (num_stu > totalStu)
    {
        cout << "Total Number of Student in this section are only: " << totalStu << " : Enter Valid Student Number." << endl;
        return;
    }
    vector<int>rollNo;
    vector<int>studentId;
    int rollTemp;
    cout << "Enter their RollNo: " << endl;
    for (int i = 0; i < num_stu; i++)
    {
        cout << i + 1 << ": ";
        cin >> rollTemp;
        if (!(checkStudentRollExist(db, branchCode, semester, section, rollTemp)))
        {
            cout << "This Roll No: " << rollTemp << " doesn't exists in this section. So it will be Ignored." << endl;
            continue;
        }
        rollNo.push_back(rollTemp);
        int stu_id = getStudentIdsByRollNo(db, branchCode, semester, section, rollTemp);
        studentId.push_back(stu_id);
        cout << "\n";
    }
    if (type == "mid")
    {
        updateMarks(db, branchCode, semester, section, subCode, studentId, rollNo,"mid_marks");
    }
    else if (type == "end")
    {
        updateMarks(db, branchCode, semester, section, subCode, studentId, rollNo,"end_marks");
    }
    else
    {
        cout << "Invalid!!!!!!???" << endl;
        return;
    }
}
void Staff::viewStudentAttendenceSec(Database& db, string& username)
{
    MYSQL* conn = db.getConnection();
    string branchCode, section, subCode;
    int semester;
    int staffId = getStaffIdByUsername(db, username);
    branchCode = getBranchCodeFromStaffId(db, staffId);

    cout << "Enter the semester: ";
    cin >> semester;
    cin.ignore();
    cout << "\nEnter the Sec: ";
    cin >> section;
    cout << "Enter the SubCode: ";
    cin >> subCode;
    if (!(checkStaffSubTeach(db, subCode, section, staffId)))
    {
        cout << "Can only view Attendance of Class you Teach!\n";
        return;
    }
    if (!(checkSubExists(db, subCode)))
    {
        cout << "Subject Doesn't Exist!! Enter correct subcode. \n";
        return;
    }

    int total_attendance = getTotalAttendance(db, branchCode, semester, section, subCode);

    // Query to select student attendance details with matching fields
    string selectQuery = "SELECT a.student_id, c.branch, a.semester, a.section, s.sub_name, a.present, a.roll_no "
        "FROM attendance AS a "
        "JOIN subcode_subname AS s ON a.sub_code = s.sub_code "
        "JOIN courseid_branchcode AS c ON a.branch_code = c.branch_code "
        "WHERE a.branch_code = '" + branchCode + "' AND a.semester = " + to_string(semester) + " AND a.section = '" + section + "' AND a.sub_code = '" + subCode + "'";
    if (mysql_query(conn, selectQuery.c_str())) {
        cerr << "Error in retrieving student attendance: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row;
    
    cout << "Student Attendance Details for Branch: " << branchCode << ", Semester: " << semester << ", Section: " << section << ", Subject Code: " << subCode << endl;
    cout << left << setw(12) << "Student ID"
        << setw(10) << "Roll No"
        << setw(40) << "Branch"
        << setw(10) << "Semester"
        << setw(10) << "Section"
        << setw(40) << "Subject"
        << setw(10) << "Present"
        << setw(10) << "Total" << endl;
    cout << "----------------------------------------------------------------------------" << endl;
    // Fetch and print each row directly
    while ((row = mysql_fetch_row(res)) != nullptr) 
    {
        cout << left << setw(12) << row[0]  // Student ID
            << setw(10) << row[6]           //Roll No
            << setw(40) << row[1]          // Branch
            << setw(10) << row[2]          // Semester
            << setw(10) << row[3]          // Section
            << setw(40) << row[4]          // Subject
            << setw(10) << row[5]          // Present
            << setw(10) << total_attendance      // Total
            << endl;
    }
    mysql_free_result(res);
}

void Staff::staffMenu(Database& db, string& user)
{
    int choice, ch1, ch2;
    string username;
    do
    {
        cout << "\nStaff Menu:" << endl;
        cout << "1. View Data" << endl;
        cout << "2. Attendance" << endl;
        cout << "3. Marks" << endl;
        cout << "4. Exit" << endl;
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
                cout << "\nStaff View Data Menu:" << endl;
                cout << "1. View Staff Data" << endl;
                cout << "2. View Student Data Based on Section" << endl;
                cout << "3. View Student Data Based on Branch and semester" << endl;
                cout << "4. View Specific Student Data" << endl;
                cout << "5. View Own Data" << endl;
                cout << "6. Exit" << endl;
                cout << "Enter your choice: ";
                cin >> ch1;
                // Check if the input operation failed
                if (cin.fail())
                {
                    cin.clear(); // Clear the error state
                    cin.ignore(50, '\n');
                    cout << "Invalid input. Please enter a number." << endl << endl;
                    continue; // Skip the switch statement and prompt again
                }
                switch (ch1)
                {
                case 1:
                    viewStaffData(db);
                    break;
                    //Shows all Staff Data excluding their salary
                case 2:
                    viewStudentDataOnSec(db,user);
                    //Shows Student Data Based on Section
                    break;
                case 3:
                    viewStudentDataOnBranchSem(db,user);
                    //View Student Data Based on Branch and semester
                    break;
                case 4:
                    viewSpecificStudentData(db);
                    //View Specific Student Data
                    break;
                case 5:
                    viewStaffOwnData(db, user);
                    //View Own Data
                    break;
                case 6:
                    cout << "Exiting View Data Menu..." << endl;
                    break;
                default:
                    cout << "Wrong Choice! Try Again." << endl;
                    break;
                }
            } while (ch1 != 6);
            break;
        case 2:
            do
            {
                cout << "\nAttendance Menu:" << endl;
                cout << "1. Add Attendance" << endl;
                cout << "2. Increment Attendance By 1" << endl;
                cout << "3. View Attendance" << endl;
                cout << "4. Exit" << endl;
                cout << "Enter your choice: ";
                cin >> ch1;
                // Check if the input operation failed
                if (cin.fail())
                {
                    cin.clear(); // Clear the error state
                    cin.ignore(50, '\n');
                    cout << "Invalid input. Please enter a number." << endl << endl;
                    continue; // Skip the switch statement and prompt again
                }
                switch (ch1)
                {
                case 1:
                    addAttendance(db, user);
                    break;
                case 2:
                    incrementCustomAttendance(db, user);
                    break;
                case 3:
                    viewStudentAttendenceSec(db, user);
                    break;
                case 4:
                    cout << "Exiting Attendance Menu." << endl;
                    break;
                default:
                    cout << "Wrong Input Choice!" << endl;
                }
            } while (ch1 != 4);
            break;
        case 3:
            do
            {
                cout << "\nMarks Menu:" << endl;
                cout << "1. View Marks " << endl;
                cout << "2. Add Marks " << endl;
                cout << "3. Exit" << endl;
                cout << "Enter your choice: ";
                cin >> ch1;
                // Check if the input operation failed
                if (cin.fail())
                {
                    cin.clear(); // Clear the error state
                    cin.ignore(50, '\n');
                    cout << "Invalid input. Please enter a number." << endl << endl;
                    continue; // Skip the switch statement and prompt again
                }
                switch (ch1)
                {
                case 1:
                    
                    do {
                        cout << "\nMarks View Menu:" << endl;
                        cout << "1. View Marks By Section" << endl;
                        cout << "2. View Specific Students Marks.." << endl;
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
                            viewStudentMarks(db, user);
                            //View Marks by section
                            break;
                        case 2:
                            viewSpecificStudentMarks(db, user);
                            //View Specific student Marks
                        case 3:
                            cout << "Exiting View Marks Menu!!" << endl;
                            break;
                        default:
                            cout << "Invalid Choice!" << endl;
                            break;
                        }
                    } while (ch2 != 3);
                    break;
                case 2:
                    //Add Marks Menu
                    do {
                        cout << "\nAdd Marks Menu:" << endl;
                        cout << "1. Add  Mid Marks of Section" << endl;
                        cout << "2. Add End Marks of Section" << endl;
                        cout << "3. Update Mid Marks Specific Students" << endl;
                        cout << "4. Update End Marks Specific Students" << endl;
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
                            addMarks(db, user, "mid");
                            //Add mid marks
                            break;
                        case 2:
                            addMarks(db, user, "end");
                            //Add end marks
                            break;
                        case 3:
                            addSpecificMarks(db, user, "mid");
                            //Add mid marks specific students
                            break;
                        case 4:
                            addSpecificMarks(db, user, "end");
                            //Add end marks specific students
                            break;
                        case 5:
                            cout << "Exiting Add Marks Menu!!" << endl;
                            break;
                        default:
                            cout << "Invalid Choice!" << endl;
                            break;
                        }
                    } while (ch2 != 5);
                    break;
                case 3:
                    cout << "Exiting Marks Menu." << endl;
                    break;
                default:
                    cout << "Wrong Input Choice!" << endl;
                    break;
                }
            } while (ch1 != 3);
            break;
        case 4:
            cout << "Exiting Staff Menu..." << endl;
            break;
        default:
            cout << "Invalid choice. Please enter again." << endl;
            break;
        }
    } while (choice != 4);
}


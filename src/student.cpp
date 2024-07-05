#include <iostream>
#include <string>
#include<mysql.h>
#include<vector>
#include <iomanip>
#include<map>

#include "database.h"
#include "student.h"

using namespace std;

// Constructors
Student::Student() : username(""), password(""), realName(""), address(""), course(""), branch(""), section(""), courseId(""), branchCode(""), semester(0), fees(0.0), cgpa(0.0) {}

Student::Student(const string& username, const string& password, const string& realName, const string& address,
    const string& course, const string& branch, const string& section, const string& courseId,
    const string& branchCode, int semester)
    : username(username), password(password), realName(realName), address(address), course(course), branch(branch), section(section), courseId(courseId), branchCode(branchCode), semester(semester), fees(0.0), cgpa(0.0) {}

// Getter functions
const string& Student::getUsername() const { return username; }
const string& Student::getPassword() const { return password; }
const string& Student::getRealName() const { return realName; }
const string& Student::getAddress() const { return address; }
const string& Student::getCourse() const { return course; }
const string& Student::getBranch() const { return branch; }
const string& Student::getSection() const { return section; }
int Student::getSemester() const { return semester; }
const string& Student::getCourseId() const { return courseId; }
const string& Student::getBranchCode() const { return branchCode; }

bool Student::checkSemesterExists(Database& db, string branchCode, int semester)
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

string Student::getBranchCodeFromStudentId(Database& db, int studentId)
{
    string branchCode;
    MYSQL* conn = db.getConnection();  // Use conn for the connection
    string query = "SELECT branch_code FROM student WHERE student_id = " + to_string(studentId);

    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return branchCode;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        cerr << "Failed to store result: " << mysql_error(conn) << endl;
        return branchCode;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        branchCode = row[0];
    }

    mysql_free_result(res);
    return branchCode;
}



int Student::getSemesterFromStudentId(Database& db, int studentId) 
{
    int semester = -1;
    MYSQL* conn = db.getConnection();  // Use conn for the connection
    string query = "SELECT semester FROM student WHERE student_id = " + to_string(studentId);

    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return semester;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        cerr << "Failed to store result: " << mysql_error(conn) << endl;
        return semester;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        semester = atoi(row[0]);
    }

    mysql_free_result(res);
    return semester;
}


string Student::getStudentSection(Database& db, int studentId) 
{
    string section;
    MYSQL* conn = db.getConnection();  // Use conn for the connection
    string query = "SELECT section FROM student WHERE student_id = " + to_string(studentId);

    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return section;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        cerr << "Failed to store result: " << mysql_error(conn) << endl;
        return section;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        section = row[0];
    }

    mysql_free_result(res);
    return section;
}

vector<string> Student::getSubCodes(Database& db, string& branchCode, int semester)
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


int Student::getTotalAttendance(Database& db, string& branchCode, int semester, string& section, string subCode)
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


string Student::getSubNameFromSubCode(Database& db, const string& subCode) {
    MYSQL* conn = db.getConnection();  // Use conn for the connection
    string query = "SELECT sub_name FROM subcode_subname WHERE sub_code = '" + subCode + "'";

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
    string subName = "";
    if (row) {
        subName = row[0];
    }

    mysql_free_result(res);
    return subName;
}


void Student::viewStudentAttendance(Database& db,string& username) 
{
    int studentId = getStudentId(db, username);
    int semester = getSemesterFromStudentId(db, studentId);
    string branchCode = getBranchCodeFromStudentId(db, studentId);
    vector<string>subcodes = getSubCodes(db, branchCode, semester);

    MYSQL* conn = db.getConnection();

    // Query to get the attendance details
    string query = "SELECT sub_code, present FROM attendance WHERE student_id = " + to_string(studentId);
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        cerr << "Failed to store result: " << mysql_error(conn) << endl;
        return;
    }

    map<string, int> attendanceMap;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        string sub_code = row[0];
        int present = atoi(row[1]);
        attendanceMap[sub_code] = present;
    }

    mysql_free_result(res);

    // Print the attendance details
    cout << left << setw(10) << "Sub Code"
        << setw(40) << "Subject"
        << setw(10) << "Present"
        << setw(10) << "Total" << endl;
    cout << string(65, '-') << endl;

    for (const string& subCode : subcodes)
    {
        string sub_name = getSubNameFromSubCode(db, subCode);
        string section = getStudentSection(db, studentId);

        int total_attendance = getTotalAttendance(db, branchCode, semester, section, subCode);

        if (attendanceMap.find(subCode) != attendanceMap.end())
        {
            cout << left << setw(10) << subCode
                << setw(40) << sub_name
                << setw(10) << attendanceMap[subCode]
                << setw(10) << total_attendance << endl;
        }
        else
        {
            cout << left << setw(10) << subCode
                << setw(40) << sub_name
                << setw(10) << 0
                << setw(10) << total_attendance << endl;
        }
    }
}



void Student::showStaffData(Database& db, vector<int>& staffIds)
{
    MYSQL* conn = db.getConnection();
    if (staffIds.empty())
    {
        cerr << "No staff IDs provided." << endl;
        return;
    }
    // Construct the SQL query to retrieve staff data
    string query = "SELECT real_name, branch, joining_date FROM staff WHERE staff_id IN (";
    for (size_t i = 0; i < staffIds.size(); ++i) {
        query += to_string(staffIds[i]);
        if (i < staffIds.size() - 1) {
            query += ", ";
        }
    }
    query += ")";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in retrieving staff data: " << mysql_error(conn) << endl;
        return;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    // Print the header
    cout << left << setw(20) << "Real Name"
        << left << setw(30) << "Branch"
        << left << setw(15) << "Joining Date" << endl;
    cout << string(65, '-') << endl;
    // Fetch and print all rows
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        cout << left << setw(20) << row[0]
            << left << setw(30) << row[1]
            << left << setw(15) << row[2] << endl;
    }
    // Free the result
    mysql_free_result(res);
}
vector<int> Student::getStaffIds(Database& db, vector<string>& subcodes, string& section)
{
    MYSQL* conn = db.getConnection();
    vector<int> staff_ids;
    if (subcodes.empty()) {
        cerr << "No subcodes provided." << endl;
        return staff_ids;
    }
    // Construct the SQL query to retrieve staff_ids
    string query = "SELECT DISTINCT staff_id FROM staffid_subcode WHERE section = '" + section + "' AND (";
    for (size_t i = 0; i < subcodes.size(); ++i) {
        query += "sub_code = '" + subcodes[i] + "'";
        if (i < subcodes.size() - 1) {
            query += " OR ";
        }
    }
    query += ")";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in retrieving staff IDs: " << mysql_error(conn) << endl;
        return staff_ids;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return staff_ids;
    }
    // Fetch all rows and add staff_id to the vector
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        if (row[0]) {
            staff_ids.push_back(stoi(row[0]));
        }
    }
    // Free the result
    mysql_free_result(res);
    return staff_ids;
}
vector<string> Student::getSubCode(Database& db, string& branch_code, int semester)
{
    MYSQL* conn = db.getConnection();
    vector<string> sub_codes;
    // Construct the SQL query to retrieve sub_codes
    string query = "SELECT sub_code FROM branchcode_subcode_sem WHERE branch_code = '" + branch_code +
        "' AND semester = " + to_string(semester);
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in retrieving sub codes: " << mysql_error(conn) << endl;
        return sub_codes;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return sub_codes;
    }
    // Fetch all rows and add sub_code to the vector
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        if (row[0]) {
            sub_codes.push_back(row[0]);
        }
    }
    // Free the result
    mysql_free_result(res);
    return sub_codes;
}
int Student::getStudentId(Database& db, string& username) {
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
void Student::viewStudentOwnData(Database& db, string& username)
{
    MYSQL* conn = db.getConnection();
    // Construct the SQL query to retrieve student data
    string query = "SELECT student_id, real_name, address, roll_no, course, branch, semester, section, fees, cgpa "
        "FROM student WHERE username = '" + username + "'";
    // Execute the query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in retrieving student data: " << mysql_error(conn) << endl;
        return;
    }
    // Store the result
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    // Fetch the row and display the data
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        // Print the student data in table format
        cout << left << setw(15) << "Student ID"
            << left << setw(30) << "Real Name"
            << left << setw(40) << "Address"
            << left << setw(10) << "Roll No"
            << left << setw(30) << "Course"
            << left << setw(30) << "Branch"
            << left << setw(10) << "Semester"
            << left << setw(10) << "Section"
            << left << setw(10) << "Fees"
            << left << setw(10) << "CGPA" << endl;
        cout << string(195, '-') << endl;
        cout << left << setw(15) << row[0]
            << left << setw(30) << row[1]
            << left << setw(40) << row[2]
            << left << setw(10) << row[3]
            << left << setw(30) << row[4]
            << left << setw(30) << row[5]
            << left << setw(10) << row[6]
            << left << setw(10) << row[7]
            << left << setw(10) << row[8]
            << left << setw(10) << row[9] << endl;
    }
    else {
        cerr << "No student found with the given username." << endl;
    }
    // Free the result
    mysql_free_result(res);
}
void Student::viewTeachingFacultyData(Database& db, string& username)
{
    MYSQL* conn = db.getConnection();
    int studentid = getStudentId(db, username);
    // Query to retrieve branch_code, semester, and section using student_id
    string query = "SELECT branch_code, semester, section FROM student WHERE student_id = " + to_string(studentid);
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in retrieving student data: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        cerr << "Error in storing result: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    string branchCode, section;
    int semester = 0;
    if (row)
    {
        // Store the retrieved data in variables
        branchCode = row[0];
        semester = stoi(row[1]);
        section = row[2];
    }
    else
    {
        cerr << "No data found for the given student ID." << endl;
        return;
    }
    mysql_free_result(res);
    vector<string>subCodes = getSubCode(db, branchCode, semester);
    vector<int>staffIds = getStaffIds(db, subCodes, section);
    showStaffData(db, staffIds);
}

void Student::viewStudentMarks(Database& db, string& username, string marksField)
{
    int studentId = getStudentId(db, username);
    int semester = getSemesterFromStudentId(db, studentId);
    string branchCode = getBranchCodeFromStudentId(db, studentId);

    int StudentSemester;
    cout << "Enter the Semester : ";
    cin >> StudentSemester;

    if (!(checkSemesterExists(db, branchCode, StudentSemester)))
    {
        cout << "This Semester doesn't Exists in this branch!!";
        return;
    }

    if (StudentSemester > semester)
    {
        cout << "Can't show marks of semester not reached Yet!!\n";
        return;
    }
    

    MYSQL* conn = db.getConnection();

    // Query to get the attendance details
    string query = "SELECT sub_code, "+ marksField +" FROM marks WHERE student_id = " + to_string(studentId);
    if (mysql_query(conn, query.c_str())) 
    {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        cerr << "Failed to store result: " << mysql_error(conn) << endl;
        return;
    }

    map<string, int> marksMap;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) 
    {
        string sub_code = row[0];
        int marks = atoi(row[1]);
        marksMap[sub_code] = marks;
    }

    mysql_free_result(res);
    cout << "\n";
    // Print the attendance details
    cout << left << setw(10) <<"Semester"
        <<setw(10) << "Sub Code"
        << setw(40) << "Subject"
        << setw(10) << marksField << endl;
    cout << string(75, '-') << endl;

    for (const auto& pair : marksMap) 
    {
        string sub_code = pair.first;
        int marks = pair.second;

        string sub_name = getSubNameFromSubCode(db,sub_code);
        cout << left << setw(10) << StudentSemester
        << setw(10) << sub_code
        << setw(40) << sub_name
        << setw(10) << marks<< endl;
    }
}


void Student::studentMenu(Database& db, string& username)
{
    int choice,ch1,ch2;
    string userType = "student";
    do
    {
        cout << "\nStudent Menu:" << endl;
        cout << "1. View Your Own Data" << endl;
        cout << "2. View Teaching Faculty Data" << endl;
        cout << "3. View Attendance" << endl;
        cout << "4. View Marks" << endl;
        cout << "5. Change Password" << endl;
        cout << "6. Exit" << endl;
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
            viewStudentOwnData(db, username);
            break;
        case 2:
            viewTeachingFacultyData(db, username);
            break;
        case 3:
            viewStudentAttendance(db, username);
            //View Student Attendance
            break;
        case 4:
            do
            {
                cout << "\nStudent Marks Menu:" << endl;
                cout << "1. Show Mid Marks" << endl;
                cout << "2. Show End Marks" << endl;
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
                        viewStudentMarks(db, username, "mid_marks");
                        break;
                    case 2:
                        viewStudentMarks(db, username, "end_marks");
                        break;
                    case 3:
                        cout << "Exiting Marks Menu..." << endl;
                        break;
                    default:
                        cout << "Invalid choice. Please enter again." << endl;
                        break;
                }
            } while (ch1 != 3);
            break;
        case 5:
            do {
                cout << "1. By old Password\n";
                cout << "2. By Email OTP\n";
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
                    passFobj.changePasswordKnown(db);
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
                    if (!(passFobj.checkExistsUser(db, username, userType)))
                    {
                        cout << "User with this username doesn't exists!!\n";
                        break;
                    }
                    passFobj.changePasswordOtp(db, username, userType);
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

        case 6:
            cout << "Exiting Student Menu..." << endl;
            break;
        default:
            cout << "Invalid choice. Please enter again." << endl;
            break;
        }
    } while (choice != 6);
}

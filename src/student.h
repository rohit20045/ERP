#pragma once
#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include <vector>
using namespace std;

class Database; // Forward declaration of the Database class

class Student 
{
private:
    string username;
    string password;
    string realName;
    string address;
    string course;
    string branch;
    string section;
    int semester;
    double fees;
    string courseId;
    string branchCode;
    double cgpa;

    string getBranchCodeFromStudentId(Database& db, int studentId);
    int getSemesterFromStudentId(Database& db, int studentId);
    string getStudentSection(Database& db, int studentId);
    int getTotalAttendance(Database& db, string& branchCode, int semester, string& section, string subCode);
    string getSubNameFromSubCode(Database& db, const string& subCode);
    vector<string> getSubCodes(Database& db, string& branchCode, int semester);
    void showStaffData(Database& db, vector<int>& staffIds);
    vector<int> getStaffIds(Database& db, vector<string>& subcodes, string& section);
    vector<string> getSubCode(Database& db, string& branch_code, int semester);
    int getStudentId(Database& db, string& username);
    void viewStudentOwnData(Database& db, string& username);
    void viewTeachingFacultyData(Database& db, string& username);
    void viewStudentAttendance(Database& db, string& username);
    void viewStudentMarks(Database& db, string& username, string marksField);
    bool checkSemesterExists(Database& db, string branchCode, int semester);

public:
    // Constructors
    Student();
    Student(const string& username, const string& password, const string& realName, const string& address,
        const string& course, const string& branch, const string& section, const string& courseId, const string& branchCode, int semester);

    // Getter functions
    const string& getUsername() const;
    const string& getPassword() const;
    const string& getRealName() const;
    const string& getAddress() const;
    const string& getCourse() const;
    const string& getBranch() const;
    const string& getSection() const;
    int getSemester() const;
    const string& getCourseId() const;
    const string& getBranchCode() const;

    void studentMenu(Database& db, string& username);
};

#endif // STUDENT_H

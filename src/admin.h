#pragma once
#ifndef ADMIN_H
#define ADMIN_H

#include <string>
#include<mysql.h>
#include<vector>
#include "PasswordFunctions.h"

using namespace std;

class Database;
class Student;
class Staff;

class Admin 
{
private:
    pass passFobj;

    string username;
    string password;
    string realName;
    string dob;
    string address;
    string joiningDate;
    double salary;

    bool checkUserExists(Database& db, const string& username);
    void resetCurCgpa(Database& db, int studentId, const string& branchCode, int semester);
    void updateSemester(Database& db, int student_id, int semester);
    bool checkFeesPaid(Database& db, int student_id);
    string getBranchCodeFromSubCode(Database& db, const string& subCode);
    int getTotalAttendance(Database& db, string& branchCode, int semester, string& section, string subCode);
    int getStaffIdByUsername(Database& db, string& username);
    int getSemesterFromSubCode(Database& db, const string subCode);
    bool checkExistsUser(Database& db, string& username, string userType);
    string getCourseFromCourseId(Database& db, string& courseId);
    string getBranchFromBranchId(Database& db, string& branchId);
    int getStudentId(Database& db, string& username);
    bool checkSectionExists(Database& db, string branchCode, int semester, string& section);
    bool check_exist(Database& db, string& username, string& userType);
    bool checkStudentExists(Database& db, int studentId);
    bool checkSubExists(Database& db, string& subCode);
    bool checkSemesterExists(Database& db, string branchCode, int semester);
    bool checkCourseExists(Database& db, string& CourseId);
    bool checkBranchExists(Database& db, string& BranchCode);
    void viewAdminData(Database& db);
    void viewAdminOwnData(string& username, Database& db);
    void viewStudentData(Database& db);
    void showStaffSubjects(Database& db, string staff_id_str);
    void viewStaffData(Database& db);
    void viewStaffData_Branch(Database& db);
    void viewStaffOwnData(Database& db);
    void viewStudentDataOn(Database& db, string field);
    void UpdateStaffOtherField(Database& db, string& username);
    void UpdateData(Database& db, string& username, string userType);
    void deleteStaff(Database& db, string& username);
    void deleteUser(Database& db, string& username, string& userType);
    void insertAdmin(Database& db, const Admin& admin);
    vector<string> getSubCodes(Database& db, string& branchCode, int semester); 
    void insertMarks(Database& db, int studentId, string& branchCode, int semester, string& section);
    void insertCgpa(Database& db, int studentId, string& branchCode, int semester, string& realName, string courseId, int gradYear);
    void insertStudent(Database& db, const Student& student);
    void insertStaff(Database& db, const Staff& staff);
    void addNewUser(Database& db);
    void change_sem(Database& db);
    void addStudentFees(Database& db);
    void addStudentExtraFees(Database& db);
    void viewStudentPendingFeesBranchSem(Database& db);
    void viewStudentFeesBranchSem(Database& db);
    void viewAdminSalary(Database& db);
    void viewStaffSalary(Database& db);
    void viewSpecificSalary(Database& db);
    void updateSalary(Database& db);
    void viewStudentAttendenceSec(Database& db);
    void updateStudentAttendance(Database& db);
    string getStaffSection(Database& db, int staffId, const string& subCode);
    void deleteStudent(Database& db, int student_id);

public:
    // Constructors
    Admin();
    Admin(const string& username, const string& password, const string& realName, const string& dob,
        const string& address, const string& joiningDate, double salary);

    // Getter functions
    const string& getUsername() const;
    const string& getPassword() const;
    const string& getRealName() const;
    const string& getDob() const;
    const string& getAddress() const;
    const string& getJoiningDate() const;
    double getSalary() const;

    void adminMenu(Database& db, string& user);
};

#endif // ADMIN_H

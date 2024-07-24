#pragma once
#ifndef STAFF_H
#define STAFF_H

#include <string>
#include <vector>
#include "PasswordFunctions.h"
using namespace std;

class Database; // Forward declaration of the Database class

class Staff {
private:
    pass passFobj;

    string username;
    string password;
    string realName;
    string dob;
    string branch;
    double salary;
    string address;
    string joiningDate;
    int numOfSubjects;
    string branchCode; // Added field for the branch code

    int getNumSubjects(Database& db, const string& branchCode, int semester);
    void updateCgpa(Database& db, const vector<int>& studentIds, const string& branchCode, int semester, const string& section, const string& subCode, int num_subjects);
    int getPresentFromAttendance(Database& db, int studentId, const string& branchCode, int semester, const string& section, const string& subCode);
    vector<string> getSubCodes(Database& db, string& branchCode, int semester);
    bool staffSemesterSubTeach(Database& db, int staffId, int semester, string& branchCode);
    vector<int> getRollNo(Database& db, string& branch_code, int semester, string& section);
    vector<int> getStudentId(Database& db, string& branch_code, int semester, string& section);
    bool checkStudentExists(Database& db, int studentId);
    bool checkSubExists(Database& db, string& subCode);
    bool checkSemesterExists(Database& db, string& branchCode, int semester);
    bool checkBranchExists(Database& db, string& BranchCode);
    bool checkSectionExists(Database& db, string& branchCode, int semester, string& section);
    void viewStaffData(Database& db);
    void viewStudentDataOnBranchSem(Database& db, string& username);
    void viewStudentDataOnSec(Database& db, string& username);
    void viewSpecificStudentData(Database& db);
    void showStaffSubjects(Database& db, string staff_id_str);
    void viewStaffOwnData(Database& db, string& username);
    bool checkStaffBranch(Database& db, string& branchCode, string& username);
    int total_students(Database& db, string& branchCode, int semester, string& section);
    int getStaffIdByUsername(Database& db, string& username);
    void totalAttendanceUpdate(Database& db, string& branchCode, int semester, string& section, string& subCode);
    int getTotalAttendance(Database& db, string& branchCode, int semester, string& section, string& subCode);
    bool checkStaffSubTeach(Database& db, string& subCode, string& section, int staffId);
    bool checkStudentRollExist(Database& db, string& branchCode, int semester, string& section, int rollNo);
    void addAttendance(Database& db, string& username);
    void incrementCustomAttendance(Database& db, string& username);
    int getNoOfTotalStudentInSec(Database& db, string& branch_code, int semester, string& section);
    void showStudentMarks(Database& db, string& branch_code, int semester, string& section, string& sub_code, vector<int>& student_ids, vector<int>& rollNo);
    void viewStudentMarks(Database& db, string& username);
    int getStudentIdsByRollNo(Database& db, string& branch_code, int semester, string& section, int rollNo);
    void viewSpecificStudentMarks(Database& db, string& username);
    void updateMarks(Database& db, string& branch_code, int semester, string& section, string& sub_code, vector<int>& student_ids, vector<int>& rollNo, string mark_type);
    void addEndMarks(Database& db, string& branch_code, int semester, string& section, string& sub_code, vector<int>& student_ids, vector<int>& rollNo);
    void addMidMarks(Database& db, string& branch_code, int semester, string& section, string& sub_code, vector<int>& student_ids, vector<int>& rollNo);
    void addMarks(Database& db, string& username, string type);
    void addSpecificMarks(Database& db, string& username, string type);
    void viewStudentAttendenceSec(Database& db, string& username);
    string getBranchCodeFromStaffId(Database& db, int staffId);
    vector<int> getRollNoFromAttendance(Database& db, const string& branchCode, int semester, const string& section, const string& subCode);
    string getStaffSection(Database& db, int staffId, const string& subCode);
    void downloadAssign(Database& db, int assign_no, const string& assign_name, const string& section, int semester);
    void downloadAssignment(Database& db, string& username);
    vector<pair<int, string>> findAssignmentsByStaff(Database& db, int staff_id);
    void viewAssignment(Database& db, string& username);
    void assignment_upload(Database& db, string& username);


public:
    // Constructors
    Staff();
    Staff(const string& username, const string& password, const string& realName, const string& dob,
        const string& branch, double salary, const string& address, const string& joiningDate,
        int numOfSubjects, const string& branchCode);

    // Getter functions
    const string& getUsername() const;
    const string& getPassword() const;
    const string& getRealName() const;
    const string& getDob() const;
    const string& getBranch() const;
    double getSalary() const;
    const string& getAddress() const;
    const string& getJoiningDate() const;
    int getNumOfSubjects() const;
    const string& getBranchCode() const; // Added getter for branch code

    void staffMenu(Database& db, string& user);
};

#endif // STAFF_H

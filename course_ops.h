#ifndef COURSE_OPS_H
#define COURSE_OPS_H

#include <string>
#include "common.h"

enum EnrollResult {
    ENROLL_SUCCESS,
    ERR_STUDENT_INACTIVE,
    ERR_COURSE_NOT_FOUND,
    ERR_SEATS_FULL,
    ERR_ALREADY_ENROLLED,
    ERR_CREDIT_LIMIT_EXCEEDED,
    ERR_PREREQ_NOT_MET
};

// Stubs for course operations
EnrollResult enrollStudent(const std::string& rollNo, const std::string& courseCode, int semester);
bool dropCourse(const std::string& rollNo, const std::string& courseCode, int semester);
int getCreditLoad(const std::string& rollNo, int semester);
bool checkPrerequisite(const std::string& rollNo, const std::string& courseCode);
Table listEnrolledStudents(const std::string& courseCode);

#endif // COURSE_OPS_H

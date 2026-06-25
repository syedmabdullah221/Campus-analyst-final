#ifndef STUDENT_OPS_H
#define STUDENT_OPS_H

#include <string>
#include "common.h"

// Stubs for student operations
bool addStudent(const Student& student);
Student searchByRoll(const std::string& rollNo);
// SearchByName can return a Table or a count of students in a custom array. Let's return Table of matches.
Table searchByName(const std::string& nameSubstr);
bool updateStudent(const std::string& rollNo, const std::string& fieldName, const std::string& newValue);
bool softDeleteStudent(const std::string& rollNo);
Table listStudents();

// Backward compatibility wrappers
inline bool softDelete(const std::string& rollNo) { return softDeleteStudent(rollNo); }
inline Table listActiveStudents() { return listStudents(); }

#endif // STUDENT_OPS_H

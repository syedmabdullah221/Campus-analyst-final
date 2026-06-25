#ifndef ATTENDANCE_H
#define ATTENDANCE_H

#include <string>
#include "common.h"

// Stubs for attendance operations
bool markAttendance(const std::string& courseCode, int semester, const std::string& date);
double getAttendancePct(const std::string& rollNo, const std::string& courseCode);
Table getShortageList(const std::string& courseCode);
bool undoLastSession(const std::string& courseCode, int semester, const std::string& date);
void printDailySheet(const std::string& courseCode, const std::string& date);

#endif // ATTENDANCE_H

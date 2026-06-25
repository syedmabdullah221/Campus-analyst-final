#ifndef REPORTS_H
#define REPORTS_H

#include <string>
#include "common.h"

// Stubs for reports operations
void printMeritList();
void printAttendanceDefaulters();
void printFeeDefaulters();
void printSemesterResult(const std::string& rollNo, int semester);
void printDepartmentSummary();
bool exportReportToFile(const std::string& filename, void (*reportFunc)());

#endif // REPORTS_H

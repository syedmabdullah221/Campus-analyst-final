#ifndef FEE_TRACKER_H
#define FEE_TRACKER_H

#include <string>
#include "common.h"

// Stubs for fee tracker operations
bool recordPayment(const std::string& rollNo, int semester, double amount, const std::string& date, const std::string& method);
double computeLateFine(const std::string& rollNo, int semester);
int daysBetween(const std::string& date1, const std::string& date2);
void generateReceipt(const std::string& rollNo, int semester);
Table getDefaulters();

#endif // FEE_TRACKER_H

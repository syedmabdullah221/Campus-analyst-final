#ifndef COMMON_H
#define COMMON_H

#include <string>

// Maximum constraints for arrays representing tables
const int MAX_FIELDS = 20;
const int MAX_ROWS = 2000;

// Representation of a row in a TXT/CSV file
struct Row {
    std::string fields[MAX_FIELDS];
    int size = 0;
};

// Representation of a collection of rows (a table)
struct Table {
    Row rows[MAX_ROWS];
    int size = 0;
};

// Struct to represent Student data
struct Student {
    std::string rollNo;      // Format: BSAI-YY-XXX
    std::string name;
    std::string department;
    int semester = 0;
    double cgpa = 0.0;
    std::string status;      // active or inactive
};

// Struct to represent Course data
struct Course {
    std::string courseCode;
    std::string courseName;
    int creditHours = 0;
    std::string instructor;
    int capacity = 0;
    int enrolled = 0;
    std::string prerequisite; // course code or "NONE"
};

// Struct to represent Enrollment data
struct Enrollment {
    std::string enrollmentId;
    std::string rollNo;
    std::string courseCode;
    int semester = 0;
    std::string enrollmentDate; // Format: DD-MM-YYYY
    std::string status;         // enrolled or dropped
};

// Struct to represent Attendance data
struct Attendance {
    std::string rollNo;
    std::string courseCode;
    std::string date;           // Format: DD-MM-YYYY
    std::string status;         // P, A, L
};

// Struct to represent Fee data
struct Fee {
    std::string feeId;
    std::string rollNo;
    int semester = 0;
    double totalFee = 0.0;
    double amountPaid = 0.0;
    std::string dueDate;        // Format: DD-MM-YYYY
    std::string paymentDate;    // Format: DD-MM-YYYY or "00-00-0000"
    std::string paymentMethod;  // Cash, Online, Bank, NONE
    std::string status;         // paid, paid_late, partial, unpaid
};

#endif // COMMON_H

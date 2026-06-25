#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include "attendance.h"
#include "filehandler.h"
#include "student_ops.h"

// Helper functions for parsing and formatting
static int stringToInt(const std::string& str) {
    int val = 0;
    bool negative = false;
    int i = 0;
    if (str.empty()) return 0;
    if (str[0] == '-') {
        negative = true;
        i = 1;
    } else if (str[0] == '+') {
        i = 1;
    }
    for (; i < (int)str.length(); ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
            val = val * 10 + (str[i] - '0');
        } else {
            break;
        }
    }
    return negative ? -val : val;
}

static int parseLogIdNum(const std::string& id) {
    if (id.length() < 2 || id[0] != 'L') return 0;
    int val = 0;
    for (size_t i = 1; i < id.length(); ++i) {
        if (id[i] >= '0' && id[i] <= '9') {
            val = val * 10 + (id[i] - '0');
        } else {
            break;
        }
    }
    return val;
}

bool markAttendance(const std::string& courseCode, int semester, const std::string& date) {
    // 1. Read all enrollments
    Table* enrollments = new Table();
    if (!readTXT("enrollments.txt", *enrollments)) {
        std::cout << "Error: Failed to read enrollments.txt\n";
        delete enrollments;
        return false;
    }
    
    // 2. Filter enrolled students
    int count = 0;
    struct TempStudent {
        std::string rollNo;
        std::string name;
    }* studentsToMark = new TempStudent[MAX_ROWS];
    
    for (int i = 0; i < enrollments->size; ++i) {
        const Row& r = enrollments->rows[i];
        if (r.size >= 6 && r.fields[2] == courseCode && r.fields[3] == std::to_string(semester) && r.fields[5] == "active") {
            Student s = searchByRoll(r.fields[1]);
            if (!s.rollNo.empty() && s.status == "active") {
                studentsToMark[count].rollNo = s.rollNo;
                studentsToMark[count].name = s.name;
                count++;
            }
        }
    }
    
    delete enrollments; // free memory early
    
    if (count == 0) {
        std::cout << "No active students enrolled in course " << courseCode << " for semester " << semester << ".\n";
        delete[] studentsToMark;
        return false;
    }
    
    // 3. For each student, prompt for status
    std::cout << "\nMarking attendance for course " << courseCode << ", Semester " << semester << " on Date " << date << ":\n";
    
    // Find last log ID in attendance_log.txt to auto-increment nextIdNum
    Table* logTable = new Table();
    int nextIdNum = 1;
    if (readTXT("attendance_log.txt", *logTable)) {
        for (int i = 0; i < logTable->size; ++i) {
            std::string logId = logTable->rows[i].fields[0];
            int num = parseLogIdNum(logId);
            if (num >= nextIdNum) {
                nextIdNum = num + 1;
            }
        }
    }
    delete logTable;
    
    for (int i = 0; i < count; ++i) {
        std::string status = "";
        while (true) {
            std::cout << studentsToMark[i].rollNo << " (" << studentsToMark[i].name << ") - Enter status (P/A/L) or 'Q' to quit: ";
            std::getline(std::cin, status);
            while (!status.empty() && (status.back() == '\r' || status.back() == '\n')) {
                status.pop_back();
            }
            if (status == "p" || status == "P") { status = "P"; break; }
            if (status == "a" || status == "A") { status = "A"; break; }
            if (status == "l" || status == "L") { status = "L"; break; }
            if (status == "q" || status == "Q") {
                std::cout << "Attendance marking cancelled. Returning to menu.\n";
                delete[] studentsToMark;
                return false;
            }
            std::cout << "Invalid status. Please enter P, A, L, or Q to quit.\n";
        }
        
        char idStr[32];
        sprintf(idStr, "L%05d", nextIdNum++);
        
        Row newRow;
        newRow.fields[0] = idStr;
        newRow.fields[1] = studentsToMark[i].rollNo;
        newRow.fields[2] = courseCode;
        newRow.fields[3] = date;
        newRow.fields[4] = status;
        newRow.size = 5;
        
        if (!appendTXT("attendance_log.txt", newRow)) {
            std::cout << "Error: Failed to write attendance log for " << studentsToMark[i].rollNo << "\n";
            delete[] studentsToMark;
            return false;
        }
    }
    
    delete[] studentsToMark;
    std::cout << "Attendance marked successfully for " << count << " students.\n";
    return true;
}

double getAttendancePct(const std::string& rollNo, const std::string& courseCode) {
    Table* logTable = new Table();
    if (!readTXT("attendance_log.txt", *logTable)) {
        delete logTable;
        return 0.0;
    }
    
    int totalSessions = 0;
    int attendedSessions = 0;
    
    for (int i = 0; i < logTable->size; ++i) {
        const Row& r = logTable->rows[i];
        if (r.size >= 5 && r.fields[1] == rollNo && r.fields[2] == courseCode) {
            totalSessions++;
            if (r.fields[4] == "P" || r.fields[4] == "L") {
                attendedSessions++;
            }
        }
    }
    
    delete logTable;
    
    if (totalSessions == 0) {
        return 0.0;
    }
    
    return ((double)attendedSessions / totalSessions) * 100.0;
}

Table getShortageList(const std::string& courseCode) {
    static Table shortageList;
    shortageList.size = 0;
    
    Table* enrollments = new Table();
    if (!readTXT("enrollments.txt", *enrollments)) {
        std::cout << "Error: Failed to read enrollments.txt\n";
        delete enrollments;
        return shortageList;
    }
    
    for (int i = 0; i < enrollments->size; ++i) {
        const Row& r = enrollments->rows[i];
        if (r.size >= 6 && r.fields[2] == courseCode && r.fields[5] == "active") {
            std::string rollNo = r.fields[1];
            Student s = searchByRoll(rollNo);
            if (!s.rollNo.empty() && s.status == "active") {
                double pct = getAttendancePct(rollNo, courseCode);
                if (pct < 75.0) {
                    if (shortageList.size < MAX_ROWS) {
                        Row row;
                        row.fields[0] = rollNo;
                        row.fields[1] = s.name;
                        char pctStr[32];
                        sprintf(pctStr, "%.2f%%", pct);
                        row.fields[2] = pctStr;
                        row.size = 3;
                        shortageList.rows[shortageList.size++] = row;
                    }
                }
            }
        }
    }
    
    delete enrollments;
    return shortageList;
}

bool undoLastSession(const std::string& courseCode, int semester, const std::string& date) {
    Table* logTable = new Table();
    if (!readTXT("attendance_log.txt", *logTable)) {
        std::cout << "Error: Failed to read attendance_log.txt\n";
        delete logTable;
        return false;
    }
    
    Table* remainingTable = new Table();
    remainingTable->size = 0;
    int removedCount = 0;
    
    for (int i = 0; i < logTable->size; ++i) {
        const Row& r = logTable->rows[i];
        if (r.size >= 5 && r.fields[2] == courseCode && r.fields[3] == date) {
            removedCount++;
        } else {
            if (remainingTable->size < MAX_ROWS) {
                remainingTable->rows[remainingTable->size++] = r;
            }
        }
    }
    
    delete logTable;
    
    if (removedCount == 0) {
        std::cout << "No attendance records found for course " << courseCode << " on date " << date << ".\n";
        delete remainingTable;
        return false;
    }
    
    bool success = writeTXT("attendance_log.txt", "log_id,roll_no,course_code,session_date,status", *remainingTable);
    delete remainingTable;
    
    if (success) {
        std::cout << "Successfully reverted attendance for " << removedCount << " records on date " << date << ".\n";
        return true;
    } else {
        std::cout << "Error: Failed to write back attendance_log.txt\n";
        return false;
    }
}

void printDailySheet(const std::string& courseCode, const std::string& date) {
    Table* logTable = new Table();
    if (!readTXT("attendance_log.txt", *logTable)) {
        std::cout << "Error: Failed to read attendance_log.txt\n";
        delete logTable;
        return;
    }
    
    std::cout << "\nDaily Attendance Sheet for " << courseCode << " on " << date << ":\n";
    std::cout << "---------------------------------------------------------\n";
    std::cout << "Roll Number       | Student Name             | Status\n";
    std::cout << "---------------------------------------------------------\n";
    
    int count = 0;
    for (int i = 0; i < logTable->size; ++i) {
        const Row& r = logTable->rows[i];
        if (r.size >= 5 && r.fields[2] == courseCode && r.fields[3] == date) {
            std::string rollNo = r.fields[1];
            std::string status = r.fields[4];
            
            Student s = searchByRoll(rollNo);
            std::string name = s.rollNo.empty() ? "Unknown Student" : s.name;
            
            char line[200];
            sprintf(line, "%-17s | %-24s | %-6s", rollNo.c_str(), name.c_str(), status.c_str());
            std::cout << line << "\n";
            count++;
        }
    }
    
    delete logTable;
    
    if (count == 0) {
        std::cout << "No attendance records found for " << courseCode << " on date " << date << ".\n";
    }
    std::cout << "---------------------------------------------------------\n";
}

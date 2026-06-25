#include <string>
#include "course_ops.h"
#include "filehandler.h"
#include <fstream>
#include <cstdio>
#include <iostream>

static int stringToInt(const std::string& str) {
    int val = 0;
    for (char c : str) {
        if (c >= '0' && c <= '9') {
            val = val * 10 + (c - '0');
        }
    }
    return val;
}

int getCreditLoad(const std::string& rollNo, int semester) {
    std::ifstream file("enrollments.txt");
    if (!file.is_open()) return 0;
    std::string line;
    bool isHeader = true;
    int totalCredits = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        if (isHeader) {
            isHeader = false;
            continue;
        }
        Row row = parseCSVLine(line);
        if (row.size >= 6 && row.fields[1] == rollNo && std::to_string(semester) == row.fields[3] && row.fields[5] == "active") {
            std::ifstream cFile("courses.txt");
            if (cFile.is_open()) {
                std::string cLine;
                bool cHeader = true;
                while (std::getline(cFile, cLine)) {
                    if (cLine.empty()) continue;
                    if (cLine.back() == '\r') cLine.pop_back();
                    if (cHeader) {
                        cHeader = false;
                        continue;
                    }
                    Row cRow = parseCSVLine(cLine);
                    if (cRow.size >= 7 && cRow.fields[0] == row.fields[2]) {
                        totalCredits += stringToInt(cRow.fields[2]);
                        break;
                    }
                }
                cFile.close();
            }
        }
    }
    file.close();
    return totalCredits;
}

bool checkPrerequisite(const std::string& rollNo, const std::string& courseCode) {
    std::ifstream file("courses.txt");
    if (!file.is_open()) return false;
    std::string line;
    bool isHeader = true;
    std::string prereq = "NONE";
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        if (isHeader) {
            isHeader = false;
            continue;
        }
        Row row = parseCSVLine(line);
        if (row.size >= 7 && row.fields[0] == courseCode) {
            prereq = row.fields[6];
            break;
        }
    }
    file.close();

    if (prereq == "NONE" || prereq.empty()) {
        return true;
    }

    std::ifstream eFile("enrollments.txt");
    if (!eFile.is_open()) return false;
    isHeader = true;
    bool met = false;
    while (std::getline(eFile, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        if (isHeader) {
            isHeader = false;
            continue;
        }
        Row row = parseCSVLine(line);
        if (row.size >= 6 && row.fields[1] == rollNo && row.fields[2] == prereq && row.fields[5] == "active") {
            met = true;
            break;
        }
    }
    eFile.close();
    return met;
}

EnrollResult enrollStudent(const std::string& rollNo, const std::string& courseCode, int semester) {
    // 1. Verify Student Roll Number exists and is active
    bool studentActive = false;
    std::ifstream sFile("students.txt");
    if (sFile.is_open()) {
        std::string line;
        bool isHeader = true;
        while (std::getline(sFile, line)) {
            if (line.empty()) continue;
            if (line.back() == '\r') line.pop_back();
            if (isHeader) {
                isHeader = false;
                continue;
            }
            Row row = parseCSVLine(line);
            if (row.size >= 6 && row.fields[0] == rollNo && row.fields[5] == "active") {
                studentActive = true;
                break;
            }
        }
        sFile.close();
    }
    if (!studentActive) {
        return ERR_STUDENT_INACTIVE;
    }

    // 2. Verify Course Code exists and check capacity
    bool courseFound = false;
    int capacity = 0;
    int enrolled = 0;
    int courseCredits = 0;
    std::ifstream cFile("courses.txt");
    if (cFile.is_open()) {
        std::string line;
        bool isHeader = true;
        while (std::getline(cFile, line)) {
            if (line.empty()) continue;
            if (line.back() == '\r') line.pop_back();
            if (isHeader) {
                isHeader = false;
                continue;
            }
            Row row = parseCSVLine(line);
            if (row.size >= 7 && row.fields[0] == courseCode) {
                courseFound = true;
                courseCredits = stringToInt(row.fields[2]);
                capacity = stringToInt(row.fields[4]);
                enrolled = stringToInt(row.fields[5]);
                break;
            }
        }
        cFile.close();
    }
    if (!courseFound) {
        return ERR_COURSE_NOT_FOUND;
    }
    if (enrolled >= capacity) {
        return ERR_SEATS_FULL;
    }

    // 3. Verify student is not already active in this course
    bool alreadyEnrolled = false;
    std::ifstream eFile("enrollments.txt");
    if (eFile.is_open()) {
        std::string line;
        bool isHeader = true;
        while (std::getline(eFile, line)) {
            if (line.empty()) continue;
            if (line.back() == '\r') line.pop_back();
            if (isHeader) {
                isHeader = false;
                continue;
            }
            Row row = parseCSVLine(line);
            if (row.size >= 6 && row.fields[1] == rollNo && row.fields[2] == courseCode && row.fields[5] == "active") {
                alreadyEnrolled = true;
                break;
            }
        }
        eFile.close();
    }
    if (alreadyEnrolled) {
        return ERR_ALREADY_ENROLLED;
    }

    // 4. Verify prerequisite met
    if (!checkPrerequisite(rollNo, courseCode)) {
        return ERR_PREREQ_NOT_MET;
    }

    // 5. Verify credit load limit
    int currentLoad = getCreditLoad(rollNo, semester);
    if (currentLoad + courseCredits > 21) {
        return ERR_CREDIT_LIMIT_EXCEEDED;
    }

    // 6. Generate next enrollment ID
    int maxId = 0;
    std::ifstream eFileRead("enrollments.txt");
    if (eFileRead.is_open()) {
        std::string line;
        bool isHeader = true;
        while (std::getline(eFileRead, line)) {
            if (line.empty()) continue;
            if (line.back() == '\r') line.pop_back();
            if (isHeader) {
                isHeader = false;
                continue;
            }
            Row row = parseCSVLine(line);
            if (row.size >= 1 && !row.fields[0].empty() && row.fields[0][0] == 'E') {
                int idVal = stringToInt(row.fields[0].substr(1));
                if (idVal > maxId) {
                    maxId = idVal;
                }
            }
        }
        eFileRead.close();
    }
    int nextIdVal = maxId + 1;
    std::string nextId = "E";
    if (nextIdVal < 10) nextId += "000";
    else if (nextIdVal < 100) nextId += "00";
    else if (nextIdVal < 1000) nextId += "0";
    nextId += std::to_string(nextIdVal);

    // 7. Write new enrollment row
    Row newEnrollment;
    newEnrollment.fields[0] = nextId;
    newEnrollment.fields[1] = rollNo;
    newEnrollment.fields[2] = courseCode;
    newEnrollment.fields[3] = std::to_string(semester);
    newEnrollment.fields[4] = "15-01-2024";
    newEnrollment.fields[5] = "active";
    newEnrollment.size = 6;
    if (!appendTXT("enrollments.txt", newEnrollment)) {
        return ERR_STUDENT_INACTIVE; // General fail
    }

    // 8. Update enrolled count in courses.txt via Read-Modify-Write
    std::ifstream file("courses.txt");
    std::ofstream tempFile("temp_courses.txt");
    if (file.is_open() && tempFile.is_open()) {
        std::string line;
        bool isHeader = true;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            if (line.back() == '\r') line.pop_back();
            if (isHeader) {
                tempFile << line << "\n";
                isHeader = false;
                continue;
            }
            Row row = parseCSVLine(line);
            if (row.size >= 7 && row.fields[0] == courseCode) {
                int currentEnrolled = stringToInt(row.fields[5]);
                row.fields[5] = std::to_string(currentEnrolled + 1);
            }
            tempFile << formatRowAsCSV(row) << "\n";
        }
        file.close();
        tempFile.close();
        std::remove("courses.txt");
        std::rename("temp_courses.txt", "courses.txt");
    }

    return ENROLL_SUCCESS;
}

bool dropCourse(const std::string& rollNo, const std::string& courseCode, int semester) {
    std::ifstream file("enrollments.txt");
    if (!file.is_open()) {
        return false;
    }
    std::ofstream tempFile("temp_enrollments.txt");
    if (!tempFile.is_open()) {
        file.close();
        return false;
    }

    std::string line;
    bool isHeader = true;
    bool found = false;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        if (isHeader) {
            tempFile << line << "\n";
            isHeader = false;
            continue;
        }
        Row row = parseCSVLine(line);
        
        // Debugging print statement showing the data being parsed while iterating
        if (row.size >= 6) {
            std::cout << "[DEBUG] Parsing enrollment record - ID: " << row.fields[0]
                      << ", Roll: " << row.fields[1]
                      << ", Course: " << row.fields[2]
                      << ", Sem: " << row.fields[3]
                      << ", Date: " << row.fields[4]
                      << ", Status: " << row.fields[5] << "\n";
        }

        if (row.size >= 6 && row.fields[1] == rollNo && row.fields[2] == courseCode && std::to_string(semester) == row.fields[3] && row.fields[5] == "active") {
            row.fields[5] = "dropped";
            found = true;
        }
        tempFile << formatRowAsCSV(row) << "\n";
    }
    file.close();
    tempFile.close();

    if (!found) {
        std::remove("temp_enrollments.txt");
        return false;
    }

    std::remove("enrollments.txt");
    std::rename("temp_enrollments.txt", "enrollments.txt");

    // Decrement enrolled count in courses.txt via Read-Modify-Write
    std::ifstream cFile("courses.txt");
    std::ofstream cTempFile("temp_courses.txt");
    if (cFile.is_open() && cTempFile.is_open()) {
        isHeader = true;
        while (std::getline(cFile, line)) {
            if (line.empty()) continue;
            if (line.back() == '\r') line.pop_back();
            if (isHeader) {
                cTempFile << line << "\n";
                isHeader = false;
                continue;
            }
            Row row = parseCSVLine(line);
            if (row.size >= 7 && row.fields[0] == courseCode) {
                int currentEnrolled = stringToInt(row.fields[5]);
                if (currentEnrolled > 0) {
                    row.fields[5] = std::to_string(currentEnrolled - 1);
                }
            }
            cTempFile << formatRowAsCSV(row) << "\n";
        }
        cFile.close();
        cTempFile.close();
        std::remove("courses.txt");
        std::rename("temp_courses.txt", "courses.txt");
    }

    return true;
}

Table listEnrolledStudents(const std::string& courseCode) {
    Table results;
    results.size = 0;
    std::ifstream file("enrollments.txt");
    if (!file.is_open()) {
        return results;
    }
    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        if (isHeader) {
            isHeader = false;
            continue;
        }
        Row row = parseCSVLine(line);
        
        // Debugging print statement showing the data being parsed while iterating
        if (row.size >= 6) {
            std::cout << "[DEBUG] Parsing enrollment record - ID: " << row.fields[0]
                      << ", Roll: " << row.fields[1]
                      << ", Course: " << row.fields[2]
                      << ", Sem: " << row.fields[3]
                      << ", Date: " << row.fields[4]
                      << ", Status: " << row.fields[5] << "\n";
        }

        if (row.size >= 6 && row.fields[2] == courseCode && row.fields[5] == "active") {
            if (results.size < MAX_ROWS) {
                results.rows[results.size++] = row;
            }
        }
    }
    file.close();
    return results;
}

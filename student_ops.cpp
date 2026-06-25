#include <string>
#include "student_ops.h"
#include "filehandler.h"
#include <fstream>
#include <cstdio>
#include <iostream>

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

static double stringToDouble(const std::string& str) {
    double val = 0.0;
    bool negative = false;
    int i = 0;
    if (str.empty()) return 0.0;
    if (str[0] == '-') {
        negative = true;
        i = 1;
    } else if (str[0] == '+') {
        i = 1;
    }
    for (; i < (int)str.length(); ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
            val = val * 10.0 + (str[i] - '0');
        } else if (str[i] == '.') {
            ++i;
            break;
        } else {
            return negative ? -val : val;
        }
    }
    double dec = 0.1;
    for (; i < (int)str.length(); ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
            val += (str[i] - '0') * dec;
            dec *= 0.1;
        } else {
            break;
        }
    }
    return negative ? -val : val;
}

static std::string doubleToString(double val) {
    if (val < 0) return "-" + doubleToString(-val);
    long long intPart = (long long)val;
    long long fracPart = (long long)((val - intPart) * 100.0 + 0.5);
    if (fracPart >= 100) {
        intPart += 1;
        fracPart -= 100;
    }
    std::string res = std::to_string(intPart) + ".";
    if (fracPart < 10) {
        res += "0";
    }
    res += std::to_string(fracPart);
    return res;
}

static Student rowToStudent(const Row& row) {
    Student s;
    if (row.size >= 6) {
        s.rollNo = row.fields[0];
        s.name = row.fields[1];
        s.department = row.fields[2];
        s.semester = stringToInt(row.fields[3]);
        s.cgpa = stringToDouble(row.fields[4]);
        s.status = row.fields[5];
    }
    return s;
}

static Row studentToRow(const Student& s) {
    Row r;
    r.fields[0] = s.rollNo;
    r.fields[1] = s.name;
    r.fields[2] = s.department;
    r.fields[3] = std::to_string(s.semester);
    r.fields[4] = doubleToString(s.cgpa);
    r.fields[5] = s.status;
    r.size = 6;
    return r;
}

bool addStudent(const Student& student) {
    if (student.rollNo.empty()) {
        return false;
    }
    if (rowExists("students.txt", student.rollNo, 0)) {
        return false;
    }
    Row r = studentToRow(student);
    return appendTXT("students.txt", r);
}

Student searchByRoll(const std::string& rollNo) {
    Row r = findRow("students.txt", rollNo, 0);
    if (r.size == 0) {
        Student emptyStudent;
        return emptyStudent;
    }
    return rowToStudent(r);
}

Table searchByName(const std::string& nameSubstr) {
    Table results;
    results.size = 0;
    Table allStudents;
    if (!readTXT("students.txt", allStudents)) {
        return results;
    }
    
    std::string queryLower = nameSubstr;
    for (int k = 0; k < (int)queryLower.length(); ++k) {
        queryLower[k] = tolower(queryLower[k]);
    }

    for (int i = 0; i < allStudents.size; ++i) {
        const Row& row = allStudents.rows[i];
        if (row.size >= 2) {
            std::string nameLower = row.fields[1];
            for (int k = 0; k < (int)nameLower.length(); ++k) {
                nameLower[k] = tolower(nameLower[k]);
            }
            if (nameLower.find(queryLower) != std::string::npos) {
                if (results.size < MAX_ROWS) {
                    results.rows[results.size++] = row;
                }
            }
        }
    }
    return results;
}

bool updateStudent(const std::string& rollNo, const std::string& fieldName, const std::string& newValue) {
    int targetCol = -1;
    if (fieldName == "rollNo" || fieldName == "roll_no") targetCol = 0;
    else if (fieldName == "name") targetCol = 1;
    else if (fieldName == "department") targetCol = 2;
    else if (fieldName == "semester") targetCol = 3;
    else if (fieldName == "cgpa") targetCol = 4;
    else if (fieldName == "status") targetCol = 5;

    if (targetCol == -1) {
        std::cout << "Invalid field name.\n";
        return false;
    }

    std::ifstream file("students.txt");
    if (!file.is_open()) {
        std::cout << "Error: Could not open students.txt\n";
        return false;
    }
    std::ofstream tempFile("temp_students.txt");
    if (!tempFile.is_open()) {
        std::cout << "Error: Could not create temp_students.txt\n";
        file.close();
        return false;
    }

    std::string line;
    bool isHeader = true;
    bool found = false;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        if (line.back() == '\r') {
            line.pop_back();
        }
        if (isHeader) {
            tempFile << line << "\n";
            isHeader = false;
            continue;
        }

        Row row = parseCSVLine(line);

        // Debugging print statement showing the data being parsed while iterating
        if (row.size >= 6) {
            std::cout << "[DEBUG] Parsing student record - Roll: " << row.fields[0]
                      << ", Name: " << row.fields[1]
                      << ", Dept: " << row.fields[2]
                      << ", Sem: " << row.fields[3]
                      << ", CGPA: " << row.fields[4]
                      << ", Status: " << row.fields[5] << "\n";
        }

        if (row.size >= 6 && row.fields[0] == rollNo) {
            row.fields[targetCol] = newValue;
            found = true;
        }

        tempFile << formatRowAsCSV(row) << "\n";
    }

    file.close();
    tempFile.close();

    if (!found) {
        std::cout << "Student ID not found\n";
    }

    std::remove("students.txt");
    std::rename("temp_students.txt", "students.txt");

    return found;
}

bool softDeleteStudent(const std::string& rollNo) {
    std::ifstream file("students.txt");
    if (!file.is_open()) {
        std::cout << "Error: Could not open students.txt\n";
        return false;
    }
    std::ofstream tempFile("temp_students.txt");
    if (!tempFile.is_open()) {
        std::cout << "Error: Could not create temp_students.txt\n";
        file.close();
        return false;
    }

    std::string line;
    bool isHeader = true;
    bool found = false;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        if (line.back() == '\r') {
            line.pop_back();
        }
        if (isHeader) {
            tempFile << line << "\n";
            isHeader = false;
            continue;
        }

        Row row = parseCSVLine(line);

        // Debugging print statement showing the data being parsed while iterating
        if (row.size >= 6) {
            std::cout << "[DEBUG] Parsing student record - Roll: " << row.fields[0]
                      << ", Name: " << row.fields[1]
                      << ", Dept: " << row.fields[2]
                      << ", Sem: " << row.fields[3]
                      << ", CGPA: " << row.fields[4]
                      << ", Status: " << row.fields[5] << "\n";
        }

        if (row.size >= 6 && row.fields[0] == rollNo) {
            row.fields[5] = "inactive";
            found = true;
        }

        tempFile << formatRowAsCSV(row) << "\n";
    }

    file.close();
    tempFile.close();

    if (!found) {
        std::cout << "Student ID not found\n";
    }

    std::remove("students.txt");
    std::rename("temp_students.txt", "students.txt");

    return found;
}

Table listStudents() {
    Table results;
    results.size = 0;
    std::ifstream file("students.txt");
    if (!file.is_open()) {
        return results;
    }

    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        if (line.back() == '\r') {
            line.pop_back();
        }
        if (isHeader) {
            isHeader = false;
            continue;
        }

        Row row = parseCSVLine(line);

        // Debugging print statement showing the data being parsed while iterating
        if (row.size >= 6) {
            std::cout << "[DEBUG] Parsing student record - Roll: " << row.fields[0]
                      << ", Name: " << row.fields[1]
                      << ", Dept: " << row.fields[2]
                      << ", Sem: " << row.fields[3]
                      << ", CGPA: " << row.fields[4]
                      << ", Status: " << row.fields[5] << "\n";
        }

        if (row.size >= 6 && row.fields[5] == "active") {
            if (results.size < MAX_ROWS) {
                results.rows[results.size++] = row;
            }
        }
    }

    file.close();
    return results;
}

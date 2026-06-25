#include <string>
#include "reports.h"
#include "filehandler.h"
#include <fstream>
#include <iostream>
#include <sstream>

static double gradeToPoints(const std::string& grade) {
    if (grade == "A") return 4.0;
    if (grade == "A-") return 3.7;
    if (grade == "B+") return 3.3;
    if (grade == "B") return 3.0;
    if (grade == "B-") return 2.7;
    if (grade == "C+") return 2.3;
    if (grade == "C") return 2.0;
    if (grade == "C-") return 1.7;
    if (grade == "D") return 1.0;
    return 0.0;
}

static std::string getStudentName(const std::string& rollNo) {
    std::ifstream file("students.txt");
    if (!file.is_open()) return "";
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
        if (row.size >= 2 && row.fields[0] == rollNo) {
            file.close();
            return row.fields[1];
        }
    }
    file.close();
    return "";
}

static int getCourseCredits(const std::string& courseCode) {
    std::ifstream file("courses.txt");
    if (!file.is_open()) return 0;
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
        if (row.size >= 3 && row.fields[0] == courseCode) {
            file.close();
            int val = 0;
            std::stringstream ss(row.fields[2]);
            ss >> val;
            return val;
        }
    }
    file.close();
    return 0;
}

struct StudentGPA {
    std::string rollNo;
    std::string name;
    double totalPoints = 0.0;
    int totalCredits = 0;
    double gpa = 0.0;
};

void printMeritList() {
    std::ifstream file("grades.txt");
    if (!file.is_open()) {
        std::cout << "No grades recorded yet.\n";
        return;
    }

    static StudentGPA list[MAX_ROWS];
    int size = 0;

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
        if (row.size >= 8) {
            std::string rollNo = row.fields[0];
            std::string courseCode = row.fields[1];
            std::string grade = row.fields[7];

            int credits = getCourseCredits(courseCode);
            double points = gradeToPoints(grade);

            // Find if student is already in list
            int idx = -1;
            for (int i = 0; i < size; ++i) {
                if (list[i].rollNo == rollNo) {
                    idx = i;
                    break;
                }
            }
            if (idx == -1) {
                if (size < MAX_ROWS) {
                    idx = size++;
                    list[idx].rollNo = rollNo;
                    list[idx].name = getStudentName(rollNo);
                    list[idx].totalPoints = 0.0;
                    list[idx].totalCredits = 0;
                    list[idx].gpa = 0.0;
                } else {
                    continue;
                }
            }
            list[idx].totalPoints += points * credits;
            list[idx].totalCredits += credits;
        }
    }
    file.close();

    if (size == 0) {
        std::cout << "No grades found to generate merit list.\n";
        return;
    }

    // Calculate GPAs
    for (int i = 0; i < size; ++i) {
        if (list[i].totalCredits > 0) {
            list[i].gpa = list[i].totalPoints / list[i].totalCredits;
        } else {
            list[i].gpa = 0.0;
        }
    }

    // Sort descending by GPA
    for (int i = 0; i < size - 1; ++i) {
        for (int j = 0; j < size - i - 1; ++j) {
            if (list[j].gpa < list[j+1].gpa) {
                StudentGPA temp = list[j];
                list[j] = list[j+1];
                list[j+1] = temp;
            }
        }
    }

    // Print
    std::cout << "\n--- Student Merit List (Sorted by GPA) ---\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << "Rank | Roll Number  | Name                     | GPA\n";
    std::cout << "--------------------------------------------------------\n";
    for (int i = 0; i < size; ++i) {
        std::cout << (i + 1);
        if (i + 1 < 10) std::cout << "    | ";
        else std::cout << "   | ";
        std::cout << list[i].rollNo << " | "
                  << list[i].name;
        int pad = 24 - list[i].name.length();
        if (pad < 0) pad = 0;
        std::cout << std::string(pad, ' ') << " | ";
        printf("%.2f\n", list[i].gpa);
    }
    std::cout << "--------------------------------------------------------\n";
}

struct AttendanceSummary {
    std::string rollNo;
    std::string courseCode;
    int total = 0;
    int attended = 0;
};

void printAttendanceDefaulters() {
    std::ifstream file("attendance_log.txt");
    if (!file.is_open()) {
        std::cout << "No attendance logs found.\n";
        return;
    }

    static AttendanceSummary list[MAX_ROWS];
    int size = 0;

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
        if (row.size >= 5) {
            std::string rollNo = row.fields[1];
            std::string courseCode = row.fields[2];
            std::string status = row.fields[4];

            // Find student + course combination
            int idx = -1;
            for (int i = 0; i < size; ++i) {
                if (list[i].rollNo == rollNo && list[i].courseCode == courseCode) {
                    idx = i;
                    break;
                }
            }
            if (idx == -1) {
                if (size < MAX_ROWS) {
                    idx = size++;
                    list[idx].rollNo = rollNo;
                    list[idx].courseCode = courseCode;
                    list[idx].total = 0;
                    list[idx].attended = 0;
                } else {
                    continue;
                }
            }
            list[idx].total++;
            if (status == "P" || status == "L") {
                list[idx].attended++;
            }
        }
    }
    file.close();

    std::cout << "\n--- Attendance Defaulters Report (Below 75%) ---\n";
    std::cout << "-----------------------------------------------------------------------------\n";
    std::cout << "Roll Number  | Name                     | CourseCode | Attended/Total | Percentage\n";
    std::cout << "-----------------------------------------------------------------------------\n";

    bool found = false;
    for (int i = 0; i < size; ++i) {
        if (list[i].total > 0) {
            double pct = ((double)list[i].attended / list[i].total) * 100.0;
            if (pct < 75.0) {
                std::string name = getStudentName(list[i].rollNo);
                if (!name.empty()) {
                    found = true;
                    std::cout << list[i].rollNo << " | "
                              << name;
                    int pad = 24 - name.length();
                    if (pad < 0) pad = 0;
                    std::cout << std::string(pad, ' ') << " | "
                              << list[i].courseCode << "       | "
                              << list[i].attended << "/" << list[i].total;
                    int pad2 = 14 - (std::to_string(list[i].attended).length() + 1 + std::to_string(list[i].total).length());
                    if (pad2 < 0) pad2 = 0;
                    std::cout << std::string(pad2, ' ') << " | ";
                    printf("%.2f%%\n", pct);
                }
            }
        }
    }
    std::cout << "-----------------------------------------------------------------------------\n";
    if (!found) {
        std::cout << "No attendance defaulters found.\n";
    }
}

void printFeeDefaulters() {
    std::ifstream file("fees.txt");
    if (!file.is_open()) {
        std::cout << "No fee records found.\n";
        return;
    }
    std::string line;
    bool isHeader = true;
    bool found = false;

    std::cout << "\n--- Fee Defaulters Report ---\n";
    std::cout << "---------------------------------------------------------------------------------\n";
    std::cout << "Roll Number  | Semester | Total Fee  | Paid       | Remaining  | Due Date   | Status\n";
    std::cout << "---------------------------------------------------------------------------------\n";

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        if (isHeader) {
            isHeader = false;
            continue;
        }
        Row row = parseCSVLine(line);
        if (row.size >= 9) {
            std::string rollNo = row.fields[1];
            double total = std::stod(row.fields[3]);
            double paid = std::stod(row.fields[4]);
            std::string dueDate = row.fields[5];
            std::string status = row.fields[8];

            if (status == "unpaid" || status == "partial" || status == "overdue" || paid < total) {
                found = true;
                double remaining = total - paid;
                std::cout << rollNo << " | "
                          << row.fields[2] << "        | "
                          << (long long)total;
                int pad1 = 10 - std::to_string((long long)total).length();
                if (pad1 < 0) pad1 = 0;
                std::cout << std::string(pad1, ' ') << " | "
                          << (long long)paid;
                int pad2 = 10 - std::to_string((long long)paid).length();
                if (pad2 < 0) pad2 = 0;
                std::cout << std::string(pad2, ' ') << " | "
                          << (long long)remaining;
                int pad3 = 10 - std::to_string((long long)remaining).length();
                if (pad3 < 0) pad3 = 0;
                std::cout << std::string(pad3, ' ') << " | "
                          << dueDate << " | "
                          << status << "\n";
            }
        }
    }
    file.close();
    std::cout << "---------------------------------------------------------------------------------\n";
    if (!found) {
        std::cout << "No fee defaulters found.\n";
    }
}

void printSemesterResult(const std::string& rollNo, int semester) {
    std::string name = getStudentName(rollNo);
    if (name.empty()) {
        std::cout << "Student " << rollNo << " not found.\n";
        return;
    }
    std::cout << "\n========================================\n";
    std::cout << "             SEMESTER REPORT            \n";
    std::cout << "========================================\n";
    std::cout << "Roll Number: " << rollNo << "\n";
    std::cout << "Name:        " << name << "\n";
    std::cout << "Semester:    " << semester << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Course Code | Total Marks | Grade\n";
    std::cout << "----------------------------------------\n";

    std::ifstream file("grades.txt");
    bool hasGrades = false;
    if (file.is_open()) {
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
            if (row.size >= 8 && row.fields[0] == rollNo) {
                std::cout << row.fields[1] << "       | "
                          << row.fields[6] << "        | "
                          << row.fields[7] << "\n";
                hasGrades = true;
            }
        }
        file.close();
    }
    std::cout << "----------------------------------------\n";
    if (!hasGrades) {
        std::cout << "No grades found for this semester.\n";
    }
    std::cout << "========================================\n";
}

struct DeptSummary {
    std::string deptName;
    int count = 0;
};

void printDepartmentSummary() {
    std::ifstream file("students.txt");
    if (!file.is_open()) {
        std::cout << "No student records found.\n";
        return;
    }

    static DeptSummary summaries[MAX_ROWS];
    int size = 0;

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
        if (row.size >= 6) {
            std::string dept = row.fields[2];
            std::string status = row.fields[5];

            if (status == "active") {
                int idx = -1;
                for (int i = 0; i < size; ++i) {
                    if (summaries[i].deptName == dept) {
                        idx = i;
                        break;
                    }
                }
                if (idx == -1) {
                    if (size < MAX_ROWS) {
                        idx = size++;
                        summaries[idx].deptName = dept;
                        summaries[idx].count = 0;
                    } else {
                        continue;
                    }
                }
                summaries[idx].count++;
            }
        }
    }
    file.close();

    std::cout << "\n--- Department Summary Report ---\n";
    std::cout << "--------------------------------------------\n";
    std::cout << "Department                          | Active Count\n";
    std::cout << "--------------------------------------------\n";
    for (int i = 0; i < size; ++i) {
        std::cout << summaries[i].deptName;
        int pad = 35 - summaries[i].deptName.length();
        if (pad < 0) pad = 0;
        std::cout << std::string(pad, ' ') << " | " << summaries[i].count << "\n";
    }
    std::cout << "--------------------------------------------\n";
}

bool exportReportToFile(const std::string& filename, void (*reportFunc)()) {
    return false;
}

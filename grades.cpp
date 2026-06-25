#include <string>
#include "grades.h"
#include "filehandler.h"
#include "student_ops.h"
#include "attendance.h"
#include <fstream>
#include <cstdio>
#include <iostream>
#include <sstream>

static int stringToInt(const std::string& str) {
    int val = 0;
    for (char c : str) {
        if (c >= '0' && c <= '9') {
            val = val * 10 + (c - '0');
        }
    }
    return val;
}

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

double bestThreeOfFive(double quizzes[], int numQuizzes) {
    if (numQuizzes <= 0) return 0.0;
    if (numQuizzes <= 3) {
        double sum = 0.0;
        for (int i = 0; i < numQuizzes; ++i) {
            sum += quizzes[i];
        }
        return sum / numQuizzes;
    }
    double temp[20];
    for (int i = 0; i < numQuizzes && i < 20; ++i) {
        temp[i] = quizzes[i];
    }
    for (int i = 0; i < numQuizzes - 1; ++i) {
        for (int j = 0; j < numQuizzes - i - 1; ++j) {
            if (temp[j] < temp[j+1]) {
                double t = temp[j];
                temp[j] = temp[j+1];
                temp[j+1] = t;
            }
        }
    }
    return (temp[0] + temp[1] + temp[2]) / 3.0;
}

double computeWeightedTotal(double quizAvg, double assignmentAvg, double mid, double final_) {
    return quizAvg * 0.20 + assignmentAvg * 0.20 + mid * 0.30 + final_ * 0.30;
}

std::string getLetterGrade(double totalMarks) {
    if (totalMarks >= 85.0) return "A";
    if (totalMarks >= 80.0) return "A-";
    if (totalMarks >= 75.0) return "B+";
    if (totalMarks >= 70.0) return "B";
    if (totalMarks >= 65.0) return "B-";
    if (totalMarks >= 61.0) return "C+";
    if (totalMarks >= 58.0) return "C";
    if (totalMarks >= 55.0) return "C-";
    if (totalMarks >= 50.0) return "D";
    return "F";
}

bool enterMarks(const std::string& rollNo, const std::string& courseCode, double quizzes[], int numQuizzes, double assignments[], int numAssignments, double mid, double final_) {
    // 1. Verify student is actively enrolled in course
    bool enrolled = false;
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
                enrolled = true;
                break;
            }
        }
        eFile.close();
    }
    if (!enrolled) {
        std::cout << "Error: Student is not enrolled in this course.\n";
        return false;
    }

    double quizAvg = bestThreeOfFive(quizzes, numQuizzes);
    double sumAssign = 0.0;
    for (int i = 0; i < numAssignments; ++i) {
        sumAssign += assignments[i];
    }
    double assignmentAvg = numAssignments > 0 ? (sumAssign / numAssignments) : 0.0;
    double weightedTotal = computeWeightedTotal(quizAvg, assignmentAvg, mid, final_);
    std::string grade = getLetterGrade(weightedTotal);

    // 2. Write or update in grades.txt using RMW
    std::ifstream file("grades.txt");
    std::ofstream tempFile("temp_grades.txt");
    bool updated = false;

    if (!file.is_open()) {
        tempFile << "roll_no,course_code,quiz_avg,assignment_avg,mid_marks,final_marks,total_marks,grade\n";
    } else {
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
            
            // Debugging print statement showing the data being parsed while iterating
            if (row.size >= 8) {
                std::cout << "[DEBUG] Parsing grade record - Roll: " << row.fields[0]
                          << ", Course: " << row.fields[1]
                          << ", Total Marks: " << row.fields[6]
                          << ", Grade: " << row.fields[7] << "\n";
            }

            if (row.size >= 8 && row.fields[0] == rollNo && row.fields[1] == courseCode) {
                row.fields[2] = std::to_string(quizAvg);
                row.fields[3] = std::to_string(assignmentAvg);
                row.fields[4] = std::to_string(mid);
                row.fields[5] = std::to_string(final_);
                row.fields[6] = std::to_string(weightedTotal);
                row.fields[7] = grade;
                updated = true;
            }
            tempFile << formatRowAsCSV(row) << "\n";
        }
        file.close();
    }

    if (!updated) {
        Row newRow;
        newRow.fields[0] = rollNo;
        newRow.fields[1] = courseCode;
        newRow.fields[2] = std::to_string(quizAvg);
        newRow.fields[3] = std::to_string(assignmentAvg);
        newRow.fields[4] = std::to_string(mid);
        newRow.fields[5] = std::to_string(final_);
        newRow.fields[6] = std::to_string(weightedTotal);
        newRow.fields[7] = grade;
        newRow.size = 8;
        tempFile << formatRowAsCSV(newRow) << "\n";
    }

    tempFile.close();
    std::remove("grades.txt");
    std::rename("temp_grades.txt", "grades.txt");

    std::cout << "Marks saved successfully.\n";
    return true;
}

double computeGPA(const std::string& rollNo, int semester) {
    std::ifstream eFile("enrollments.txt");
    if (!eFile.is_open()) return 0.0;

    std::string line;
    bool isHeader = true;
    double totalGradePoints = 0.0;
    int totalCredits = 0;

    while (std::getline(eFile, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        if (isHeader) {
            isHeader = false;
            continue;
        }
        Row eRow = parseCSVLine(line);
        if (eRow.size >= 6 && eRow.fields[1] == rollNo && std::to_string(semester) == eRow.fields[3] && eRow.fields[5] == "active") {
            std::string courseCode = eRow.fields[2];
            
            // Get course credits
            int credits = 0;
            std::ifstream cFile("courses.txt");
            if (cFile.is_open()) {
                bool cHeader = true;
                std::string cLine;
                while (std::getline(cFile, cLine)) {
                    if (cLine.empty()) continue;
                    if (cLine.back() == '\r') cLine.pop_back();
                    if (cHeader) {
                        cHeader = false;
                        continue;
                    }
                    Row cRow = parseCSVLine(cLine);
                    if (cRow.size >= 7 && cRow.fields[0] == courseCode) {
                        credits = stringToInt(cRow.fields[2]);
                        break;
                    }
                }
                cFile.close();
            }

            // Get grade from grades.txt
            std::string grade = "";
            std::ifstream gFile("grades.txt");
            if (gFile.is_open()) {
                bool gHeader = true;
                std::string gLine;
                while (std::getline(gFile, gLine)) {
                    if (gLine.empty()) continue;
                    if (gLine.back() == '\r') gLine.pop_back();
                    if (gHeader) {
                        gHeader = false;
                        continue;
                    }
                    Row gRow = parseCSVLine(gLine);
                    
                    // Debugging print statement showing the data being parsed while iterating
                    if (gRow.size >= 8) {
                        std::cout << "[DEBUG] Parsing grade record - Roll: " << gRow.fields[0]
                                  << ", Course: " << gRow.fields[1]
                                  << ", Total Marks: " << gRow.fields[6]
                                  << ", Grade: " << gRow.fields[7] << "\n";
                    }

                    if (gRow.size >= 8 && gRow.fields[0] == rollNo && gRow.fields[1] == courseCode) {
                        grade = gRow.fields[7];
                        break;
                    }
                }
                gFile.close();
            }

            if (!grade.empty()) {
                totalGradePoints += gradeToPoints(grade) * credits;
                totalCredits += credits;
            }
        }
    }
    eFile.close();

    if (totalCredits == 0) return 0.0;
    return totalGradePoints / totalCredits;
}

Stats computeClassState(const std::string& courseCode) {
    Stats s;
    std::ifstream file("grades.txt");
    if (!file.is_open()) return s;
    
    std::string line;
    bool isHeader = true;
    double marks[MAX_ROWS];
    int count = 0;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        if (isHeader) {
            isHeader = false;
            continue;
        }
        Row row = parseCSVLine(line);
        
        // Debugging print statement showing the data being parsed while iterating
        if (row.size >= 8) {
            std::cout << "[DEBUG] Parsing grade record - Roll: " << row.fields[0]
                      << ", Course: " << row.fields[1]
                      << ", Total Marks: " << row.fields[6]
                      << ", Grade: " << row.fields[7] << "\n";
        }

        if (row.size >= 8 && row.fields[1] == courseCode) {
            double m = 0.0;
            try {
                m = std::stod(row.fields[6]);
            } catch (...) {}
            if (count < MAX_ROWS) {
                marks[count++] = m;
            }
        }
    }
    file.close();
    
    if (count == 0) return s;
    
    double sum = 0.0;
    double highest = marks[0];
    double lowest = marks[0];
    for (int i = 0; i < count; ++i) {
        sum += marks[i];
        if (marks[i] > highest) highest = marks[i];
        if (marks[i] < lowest) lowest = marks[i];
    }
    
    s.mean = sum / count;
    s.highest = highest;
    s.lowest = lowest;
    
    for (int i = 0; i < count - 1; ++i) {
        for (int j = 0; j < count - i - 1; ++j) {
            if (marks[j] > marks[j+1]) {
                double t = marks[j];
                marks[j] = marks[j+1];
                marks[j+1] = t;
            }
        }
    }
    if (count % 2 == 0) {
        s.median = (marks[count / 2 - 1] + marks[count / 2]) / 2.0;
    } else {
        s.median = marks[count / 2];
    }
    
    return s;
}

void applyAttendancePenalty(const std::string& rollNo, const std::string& courseCode) {
    double pct = getAttendancePct(rollNo, courseCode);
    if (pct >= 75.0) {
        std::cout << "Attendance is satisfactory (" << pct << "%). No penalty applied.\n";
        return;
    }

    std::ifstream file("grades.txt");
    if (!file.is_open()) {
        std::cout << "No grades recorded yet.\n";
        return;
    }
    std::ofstream tempFile("temp_grades.txt");
    if (!tempFile.is_open()) {
        file.close();
        return;
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
        if (row.size >= 8) {
            std::cout << "[DEBUG] Parsing grade record - Roll: " << row.fields[0]
                      << ", Course: " << row.fields[1]
                      << ", Total Marks: " << row.fields[6]
                      << ", Grade: " << row.fields[7] << "\n";
        }

        if (row.size >= 8 && row.fields[0] == rollNo && row.fields[1] == courseCode) {
            double total = 0.0;
            try {
                total = std::stod(row.fields[6]);
            } catch (...) {}
            double newTotal = total - 5.0;
            if (newTotal < 0.0) newTotal = 0.0;
            row.fields[6] = std::to_string(newTotal);
            row.fields[7] = getLetterGrade(newTotal);
            found = true;
            std::cout << "Attendance penalty applied: " << rollNo << " in " << courseCode 
                      << " has " << pct << "% attendance. Deducted 5 marks. New Grade: " << row.fields[7] << "\n";
        }
        tempFile << formatRowAsCSV(row) << "\n";
    }
    file.close();
    tempFile.close();

    std::remove("grades.txt");
    std::rename("temp_grades.txt", "grades.txt");

    if (!found) {
        std::cout << "No grade record found for student " << rollNo << " in course " << courseCode << ".\n";
    }
}

#include <string>
#include <iostream>
#include "common.h"
#include "student_ops.h"
#include "course_ops.h"
#include "grades.h"
#include "fee_tracker.h"
#include "attendance.h"
#include "reports.h"



static void studentMenu() {
    int choice;
    do {
        std::cout << "\n--- Student Management Module ---\n";
        std::cout << "1. Add Student\n";
        std::cout << "2. Search Student by Roll Number\n";
        std::cout << "3. Search Student by Name\n";
        std::cout << "4. Update Student Details\n";
        std::cout << "5. Soft Delete Student\n";
        std::cout << "6. List Active Students\n";
        std::cout << "0. Back to Main Menu\n";
        std::cout << "Enter choice: ";
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) {
                break;
            }
            std::cout << "Invalid input. Please enter a number.\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            choice = -1;
            continue;
        }
        std::cin.ignore(10000, '\n'); // clear newline

        switch (choice) {
            case 1: {
                Student s;
                std::cout << "Enter Roll Number (BSAI-YY-XXX): ";
                if (!std::getline(std::cin, s.rollNo)) break;
                std::cout << "Enter Name: ";
                if (!std::getline(std::cin, s.name)) break;
                std::cout << "Enter Department: ";
                if (!std::getline(std::cin, s.department)) break;
                std::cout << "Enter Semester: ";
                if (!(std::cin >> s.semester)) {
                    if (std::cin.eof()) break;
                    std::cin.clear();
                    s.semester = 0;
                }
                std::cin.ignore(10000, '\n');
                std::cout << "Enter CGPA: ";
                if (!(std::cin >> s.cgpa)) {
                    if (std::cin.eof()) break;
                    std::cin.clear();
                    s.cgpa = 0.0;
                }
                std::cin.ignore(10000, '\n');
                s.status = "active";
                if (addStudent(s)) {
                    std::cout << "Student added successfully.\n";
                } else {
                    std::cout << "Failed to add student. Roll number might already exist.\n";
                }
                break;
            }
            case 2: {
                std::cout << "Enter Roll Number to search: ";
                std::string roll;
                if (!std::getline(std::cin, roll)) break;
                Student s = searchByRoll(roll);
                if (s.rollNo.empty()) {
                    std::cout << "Student not found.\n";
                } else {
                    std::cout << "Student Found:\n";
                    std::cout << "Roll No: " << s.rollNo << "\n";
                    std::cout << "Name: " << s.name << "\n";
                    std::cout << "Department: " << s.department << "\n";
                    std::cout << "Semester: " << s.semester << "\n";
                    std::cout << "CGPA: " << s.cgpa << "\n";
                    std::cout << "Status: " << s.status << "\n";
                }
                break;
            }
            case 3: {
                std::cout << "Enter student name substring to search: ";
                std::string nameSub;
                if (!std::getline(std::cin, nameSub)) break;
                Table results = searchByName(nameSub);
                if (results.size == 0) {
                    std::cout << "No matching students found.\n";
                } else {
                    std::cout << "Matching Students:\n";
                    for (int i = 0; i < results.size; ++i) {
                        std::cout << results.rows[i].fields[0] << " - "
                                  << results.rows[i].fields[1] << " - "
                                  << results.rows[i].fields[2] << " - Sem "
                                  << results.rows[i].fields[3] << " - GPA "
                                  << results.rows[i].fields[4] << " - "
                                  << results.rows[i].fields[5] << "\n";
                    }
                }
                break;
            }
            case 4: {
                std::cout << "Enter Roll Number of the student to update: ";
                std::string roll;
                if (!std::getline(std::cin, roll)) break;
                std::cout << "Enter field to update (name, department, semester, cgpa, status): ";
                std::string field;
                if (!std::getline(std::cin, field)) break;
                std::cout << "Enter new value: ";
                std::string newVal;
                if (!std::getline(std::cin, newVal)) break;
                if (updateStudent(roll, field, newVal)) {
                    std::cout << "Student updated successfully.\n";
                } else {
                    std::cout << "Failed to update student. Please check roll number and field name.\n";
                }
                break;
            }
            case 5: {
                std::cout << "Enter Roll Number to delete (soft delete): ";
                std::string roll;
                if (!std::getline(std::cin, roll)) break;
                if (softDeleteStudent(roll)) {
                    std::cout << "Student deleted (status marked as inactive).\n";
                } else {
                    std::cout << "Failed to delete student.\n";
                }
                break;
            }
            case 6: {
                Table results = listStudents();
                if (results.size == 0) {
                    std::cout << "No active students found.\n";
                } else {
                    std::cout << "Active Students:\n";
                    for (int i = 0; i < results.size; ++i) {
                        std::cout << results.rows[i].fields[0] << " - "
                                  << results.rows[i].fields[1] << " - "
                                  << results.rows[i].fields[2] << " - Sem "
                                  << results.rows[i].fields[3] << " - GPA "
                                  << results.rows[i].fields[4] << "\n";
                    }
                }
                break;
            }
            case 0:
                break;
            default:
                std::cout << "Invalid option. Please try again.\n";
                break;
        }
    } while (choice != 0);
}

static void courseMenu() {
    int choice;
    do {
        std::cout << "\n--- Course Management Module ---\n";
        std::cout << "1. Enroll Student in Course\n";
        std::cout << "2. Drop Course\n";
        std::cout << "3. List Enrolled Students\n";
        std::cout << "0. Back to Main Menu\n";
        std::cout << "Enter choice: ";
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) break;
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            choice = -1;
            continue;
        }
        std::cin.ignore(10000, '\n');
        switch (choice) {
            case 1: {
                std::cout << "Enter Student Roll Number (BSAI-YY-XXX): ";
                std::string roll;
                if (!std::getline(std::cin, roll)) break;
                std::cout << "Enter Course Code (e.g. CS101): ";
                std::string course;
                if (!std::getline(std::cin, course)) break;
                std::cout << "Enter Semester: ";
                int semester;
                if (!(std::cin >> semester)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    std::cout << "Invalid semester input.\n";
                    break;
                }
                std::cin.ignore(10000, '\n');
                
                EnrollResult res = enrollStudent(roll, course, semester);
                switch (res) {
                    case ENROLL_SUCCESS:
                        std::cout << "Student enrolled successfully.\n";
                        break;
                    case ERR_STUDENT_INACTIVE:
                        std::cout << "Failed to enroll. Student is inactive or does not exist.\n";
                        break;
                    case ERR_COURSE_NOT_FOUND:
                        std::cout << "Failed to enroll. Course code not found.\n";
                        break;
                    case ERR_SEATS_FULL:
                        std::cout << "Failed to enroll. Course capacity is full.\n";
                        break;
                    case ERR_ALREADY_ENROLLED:
                        std::cout << "Failed to enroll. Student is already enrolled in this course.\n";
                        break;
                    case ERR_CREDIT_LIMIT_EXCEEDED:
                        std::cout << "Failed to enroll. Credit hour limit exceeded (max 21 credits per semester).\n";
                        break;
                    case ERR_PREREQ_NOT_MET:
                        std::cout << "Failed to enroll. Course prerequisite is not met.\n";
                        break;
                }
                break;
            }
            case 2: {
                std::cout << "Enter Student Roll Number (BSAI-YY-XXX): ";
                std::string roll;
                if (!std::getline(std::cin, roll)) break;
                std::cout << "Enter Course Code (e.g. CS101): ";
                std::string course;
                if (!std::getline(std::cin, course)) break;
                std::cout << "Enter Semester: ";
                int semester;
                if (!(std::cin >> semester)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    std::cout << "Invalid semester input.\n";
                    break;
                }
                std::cin.ignore(10000, '\n');

                if (dropCourse(roll, course, semester)) {
                    std::cout << "Course dropped successfully.\n";
                } else {
                    std::cout << "Failed to drop course. Record not found or student is not enrolled.\n";
                }
                break;
            }
            case 3: {
                std::cout << "Enter Course Code to view enrolled students: ";
                std::string course;
                if (!std::getline(std::cin, course)) break;
                Table results = listEnrolledStudents(course);
                if (results.size == 0) {
                    std::cout << "No students currently enrolled in this course.\n";
                } else {
                    std::cout << "Enrolled Students in " << course << ":\n";
                    for (int i = 0; i < results.size; ++i) {
                        std::cout << results.rows[i].fields[0] << " - "
                                  << results.rows[i].fields[1] << " - "
                                  << results.rows[i].fields[2] << " - Sem "
                                  << results.rows[i].fields[3] << " - "
                                  << results.rows[i].fields[4] << " - "
                                  << results.rows[i].fields[5] << "\n";
                    }
                }
                break;
            }
            case 0:
                break;
            default:
                std::cout << "Invalid option. Please try again.\n";
                break;
        }
    } while (choice != 0);
}

static void attendanceMenu() {
    int choice;
    do {
        std::cout << "\n--- Attendance Management Module ---\n";
        std::cout << "1. Mark Daily Attendance\n";
        std::cout << "2. Calculate Attendance Percentage\n";
        std::cout << "3. Get Attendance Shortage List\n";
        std::cout << "4. Undo Last Attendance Session\n";
        std::cout << "0. Back to Main Menu\n";
        std::cout << "Enter choice: ";
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) break;
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            choice = -1;
            continue;
        }
        std::cin.ignore(10000, '\n');
        switch (choice) {
            case 1: {
                std::string courseCode;
                int semester;
                std::string date;
                std::cout << "Enter Course Code: ";
                if (!std::getline(std::cin, courseCode)) break;
                std::cout << "Enter Semester: ";
                if (!(std::cin >> semester)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    break;
                }
                std::cin.ignore(10000, '\n'); // clear newline
                std::cout << "Enter Date (DD-MM-YYYY): ";
                if (!std::getline(std::cin, date)) break;
                markAttendance(courseCode, semester, date);
                break;
            }
            case 2: {
                std::string rollNo;
                std::string courseCode;
                std::cout << "Enter Roll Number (BSAI-YY-XXX): ";
                if (!std::getline(std::cin, rollNo)) break;
                std::cout << "Enter Course Code: ";
                if (!std::getline(std::cin, courseCode)) break;
                double pct = getAttendancePct(rollNo, courseCode);
                std::cout << "Attendance Percentage for " << rollNo << " in " << courseCode << ": " << pct << "%\n";
                break;
            }
            case 3: {
                std::string courseCode;
                std::cout << "Enter Course Code: ";
                if (!std::getline(std::cin, courseCode)) break;
                Table shortage = getShortageList(courseCode);
                if (shortage.size == 0) {
                    std::cout << "No students with shortage (<75%) found for course " << courseCode << ".\n";
                } else {
                    std::cout << "\nAttendance Shortage List for course " << courseCode << ":\n";
                    std::cout << "---------------------------------------------------------\n";
                    std::cout << "Roll Number       | Student Name             | Attendance\n";
                    std::cout << "---------------------------------------------------------\n";
                    for (int i = 0; i < shortage.size; ++i) {
                        char line[200];
                        sprintf(line, "%-17s | %-24s | %s", shortage.rows[i].fields[0].c_str(), shortage.rows[i].fields[1].c_str(), shortage.rows[i].fields[2].c_str());
                        std::cout << line << "\n";
                    }
                    std::cout << "---------------------------------------------------------\n";
                }
                break;
            }
            case 4: {
                std::string courseCode;
                int semester;
                std::string date;
                std::cout << "Enter Course Code: ";
                if (!std::getline(std::cin, courseCode)) break;
                std::cout << "Enter Semester: ";
                if (!(std::cin >> semester)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    break;
                }
                std::cin.ignore(10000, '\n'); // clear newline
                std::cout << "Enter Date (DD-MM-YYYY): ";
                if (!std::getline(std::cin, date)) break;
                undoLastSession(courseCode, semester, date);
                break;
            }
            case 0:
                break;
            default:
                std::cout << "Invalid option. Please try again.\n";
                break;
        }
    } while (choice != 0);
}

static void gradesMenu() {
    int choice;
    do {
        std::cout << "\n--- Grade Management Module ---\n";
        std::cout << "1. Enter Student Marks\n";
        std::cout << "2. View Class Grade Stats\n";
        std::cout << "3. Compute Student GPA\n";
        std::cout << "4. Apply Attendance Grade Penalty\n";
        std::cout << "0. Back to Main Menu\n";
        std::cout << "Enter choice: ";
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) break;
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            choice = -1;
            continue;
        }
        std::cin.ignore(10000, '\n');
        switch (choice) {
            case 1: {
                std::cout << "Enter Student Roll Number (BSAI-YY-XXX): ";
                std::string roll;
                if (!std::getline(std::cin, roll)) break;
                std::cout << "Enter Course Code (e.g. CS101): ";
                std::string course;
                if (!std::getline(std::cin, course)) break;
                
                std::cout << "Enter number of quizzes (up to 5): ";
                int numQuizzes;
                if (!(std::cin >> numQuizzes)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    break;
                }
                std::cin.ignore(10000, '\n');
                if (numQuizzes < 0 || numQuizzes > 5) numQuizzes = 5;
                double quizzes[5] = {0.0};
                for (int i = 0; i < numQuizzes; ++i) {
                    std::cout << "  Enter marks for Quiz " << (i + 1) << ": ";
                    if (!(std::cin >> quizzes[i])) {
                        std::cin.clear();
                        std::cin.ignore(10000, '\n');
                    }
                    std::cin.ignore(10000, '\n');
                }

                std::cout << "Enter number of assignments: ";
                int numAssigns;
                if (!(std::cin >> numAssigns)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    break;
                }
                std::cin.ignore(10000, '\n');
                if (numAssigns < 0 || numAssigns > 20) numAssigns = 0;
                double assigns[20] = {0.0};
                for (int i = 0; i < numAssigns; ++i) {
                    std::cout << "  Enter marks for Assignment " << (i + 1) << ": ";
                    if (!(std::cin >> assigns[i])) {
                        std::cin.clear();
                        std::cin.ignore(10000, '\n');
                    }
                    std::cin.ignore(10000, '\n');
                }

                std::cout << "Enter Midterm marks: ";
                double mid;
                if (!(std::cin >> mid)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    mid = 0.0;
                }
                std::cin.ignore(10000, '\n');

                std::cout << "Enter Final term marks: ";
                double final_;
                if (!(std::cin >> final_)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    final_ = 0.0;
                }
                std::cin.ignore(10000, '\n');

                enterMarks(roll, course, quizzes, numQuizzes, assigns, numAssigns, mid, final_);
                break;
            }
            case 2: {
                std::cout << "Enter Course Code: ";
                std::string course;
                if (!std::getline(std::cin, course)) break;
                Stats stats = computeClassState(course);
                if (stats.mean == 0.0 && stats.highest == 0.0) {
                    std::cout << "No grades found for course " << course << ".\n";
                } else {
                    std::cout << "\nClass Grade Stats for " << course << ":\n";
                    std::cout << "  Mean Marks: " << stats.mean << "\n";
                    std::cout << "  Median Marks: " << stats.median << "\n";
                    std::cout << "  Highest Marks: " << stats.highest << "\n";
                    std::cout << "  Lowest Marks: " << stats.lowest << "\n";
                }
                break;
            }
            case 3: {
                std::cout << "Enter Student Roll Number (BSAI-YY-XXX): ";
                std::string roll;
                if (!std::getline(std::cin, roll)) break;
                std::cout << "Enter Semester: ";
                int semester;
                if (!(std::cin >> semester)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    break;
                }
                std::cin.ignore(10000, '\n');
                double gpa = computeGPA(roll, semester);
                std::cout << "Computed GPA for semester " << semester << ": " << gpa << "\n";
                break;
            }
            case 4: {
                std::cout << "Enter Student Roll Number (BSAI-YY-XXX): ";
                std::string roll;
                if (!std::getline(std::cin, roll)) break;
                std::cout << "Enter Course Code: ";
                std::string course;
                if (!std::getline(std::cin, course)) break;
                applyAttendancePenalty(roll, course);
                break;
            }
            case 0:
                break;
            default:
                std::cout << "Invalid option. Please try again.\n";
                break;
        }
    } while (choice != 0);
}

static void feeMenu() {
    int choice;
    do {
        std::cout << "\n--- Fee Tracker Module ---\n";
        std::cout << "1. Record Fee Payment\n";
        std::cout << "2. Compute Late Fine\n";
        std::cout << "3. Generate Receipt\n";
        std::cout << "4. Get Defaulters List\n";
        std::cout << "0. Back to Main Menu\n";
        std::cout << "Enter choice: ";
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) break;
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            choice = -1;
            continue;
        }
        std::cin.ignore(10000, '\n');
        switch (choice) {
            case 1: {
                std::string roll;
                int semester;
                double amount;
                std::string date;
                std::string method;

                std::cout << "Enter Student Roll Number (BSAI-YY-XXX): ";
                if (!std::getline(std::cin, roll)) break;
                std::cout << "Enter Semester: ";
                if (!(std::cin >> semester)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    break;
                }
                std::cin.ignore(10000, '\n');
                std::cout << "Enter Amount: ";
                if (!(std::cin >> amount)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    break;
                }
                std::cin.ignore(10000, '\n');
                std::cout << "Enter Payment Date (DD-MM-YYYY): ";
                if (!std::getline(std::cin, date)) break;
                std::cout << "Enter Payment Method (Online/Cash/Bank): ";
                if (!std::getline(std::cin, method)) break;

                recordPayment(roll, semester, amount, date, method);
                break;
            }
            case 2: {
                std::string roll;
                int semester;

                std::cout << "Enter Student Roll Number (BSAI-YY-XXX): ";
                if (!std::getline(std::cin, roll)) break;
                std::cout << "Enter Semester: ";
                if (!(std::cin >> semester)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    break;
                }
                std::cin.ignore(10000, '\n');

                double fine = computeLateFine(roll, semester);
                std::cout << "Late Fine: Rs. " << fine << "\n";
                break;
            }
            case 3: {
                std::string roll;
                int semester;

                std::cout << "Enter Student Roll Number (BSAI-YY-XXX): ";
                if (!std::getline(std::cin, roll)) break;
                std::cout << "Enter Semester: ";
                if (!(std::cin >> semester)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    break;
                }
                std::cin.ignore(10000, '\n');

                generateReceipt(roll, semester);
                break;
            }
            case 4: {
                Table defaulters = getDefaulters();
                if (defaulters.size == 0) {
                    std::cout << "No fee defaulters found.\n";
                } else {
                    std::cout << "\nFee Defaulters List:\n";
                    std::cout << "------------------------------------------------------------\n";
                    std::cout << "Roll Number  | Semester | Total Fee | Paid      | Status\n";
                    std::cout << "------------------------------------------------------------\n";
                    for (int i = 0; i < defaulters.size; ++i) {
                        Row row = defaulters.rows[i];
                        std::cout << row.fields[1] << " | "
                                  << row.fields[2] << "        | "
                                  << row.fields[3] << "     | "
                                  << row.fields[4] << "     | "
                                  << row.fields[8] << "\n";
                    }
                    std::cout << "------------------------------------------------------------\n";
                }
                break;
            }
            case 0:
                break;
            default:
                std::cout << "Invalid option. Please try again.\n";
                break;
        }
    } while (choice != 0);
}

static void reportsMenu() {
    int choice;
    do {
        std::cout << "\n--- Report Generation Module ---\n";
        std::cout << "1. Print Merit List\n";
        std::cout << "2. Print Attendance Defaulters\n";
        std::cout << "3. Print Fee Defaulters\n";
        std::cout << "4. Print Department Summary\n";
        std::cout << "0. Back to Main Menu\n";
        std::cout << "Enter choice: ";
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) break;
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            choice = -1;
            continue;
        }
        std::cin.ignore(10000, '\n');
        switch (choice) {
            case 1:
                printMeritList();
                break;
            case 2:
                printAttendanceDefaulters();
                break;
            case 3:
                printFeeDefaulters();
                break;
            case 4:
                printDepartmentSummary();
                break;
            case 0:
                break;
            default:
                std::cout << "Invalid option. Please try again.\n";
                break;
        }
    } while (choice != 0);
}

int main() {
    std::cout << "Campus Analytics Engine Initialized." << std::endl;

    int choice;
    do {
        std::cout << "\n===============================\n";
        std::cout << "    Campus Analytics Engine    \n";
        std::cout << "===============================\n";
        std::cout << "1. Student Management\n";
        std::cout << "2. Course Management\n";
        std::cout << "3. Attendance Management\n";
        std::cout << "4. Grade Management\n";
        std::cout << "5. Fee Tracker\n";
        std::cout << "6. Report Generation\n";
        std::cout << "0. Exit\n";
        std::cout << "===============================\n";
        std::cout << "Enter your choice: ";
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) {
                break;
            }
            std::cout << "Invalid input. Please enter a number.\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            choice = -1;
            continue;
        }

        if (choice != 0) {
            std::cin.ignore(10000, '\n');
        }

        switch (choice) {
            case 1:
                studentMenu();
                break;
            case 2:
                courseMenu();
                break;
            case 3:
                attendanceMenu();
                break;
            case 4:
                gradesMenu();
                break;
            case 5:
                feeMenu();
                break;
            case 6:
                reportsMenu();
                break;
            case 0:
                std::cout << "Exiting Campus Analytics Engine...\n";
                break;
            default:
                std::cout << "Invalid choice. Please choose between 0 and 6.\n";
                break;
        }
    } while (choice != 0);

    if (!std::cin.eof()) {
        std::cout << "\nProgram execution finished. Press Enter to exit..." << std::endl;
        std::cin.ignore(10000, '\n');
        std::cin.get();
    }
    return 0;
}

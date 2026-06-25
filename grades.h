#ifndef GRADES_H
#define GRADES_H

#include <string>
#include "common.h"

struct Stats {
    double highest = 0.0;
    double lowest = 0.0;
    double mean = 0.0;
    double median = 0.0;
};

// Stubs for grades operations
bool enterMarks(const std::string& rollNo, const std::string& courseCode, double quizzes[], int numQuizzes, double assignments[], int numAssignments, double mid, double final_);
double bestThreeOfFive(double quizzes[], int numQuizzes);
double computeWeightedTotal(double quizAvg, double assignmentAvg, double mid, double final_);
std::string getLetterGrade(double totalMarks);
double computeGPA(const std::string& rollNo, int semester);
Stats computeClassState(const std::string& courseCode);
void applyAttendancePenalty(const std::string& rollNo, const std::string& courseCode);

#endif // GRADES_H

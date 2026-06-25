#include <string>
#include "fee_tracker.h"
#include "filehandler.h"
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

static bool isLeap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

static int daysInMonth(int m, int y) {
    if (m == 2) return isLeap(y) ? 29 : 28;
    if (m == 4 || m == 6 || m == 9 || m == 11) return 30;
    return 31;
}

static long long daysFromEpoch(int d, int m, int y) {
    long long total = d;
    for (int yr = 1; yr < y; ++yr) {
        total += isLeap(yr) ? 366 : 365;
    }
    for (int mon = 1; mon < m; ++mon) {
        total += daysInMonth(mon, y);
    }
    return total;
}

int daysBetween(const std::string& date1, const std::string& date2) {
    if (date1.length() < 10 || date2.length() < 10) return 0;
    int d1 = stringToInt(date1.substr(0, 2));
    int m1 = stringToInt(date1.substr(3, 2));
    int y1 = stringToInt(date1.substr(6, 4));
    
    int d2 = stringToInt(date2.substr(0, 2));
    int m2 = stringToInt(date2.substr(3, 2));
    int y2 = stringToInt(date2.substr(6, 4));
    
    long long total1 = daysFromEpoch(d1, m1, y1);
    long long total2 = daysFromEpoch(d2, m2, y2);
    
    return (int)(total2 - total1);
}

double computeLateFine(const std::string& rollNo, int semester) {
    std::ifstream file("fees.txt");
    if (!file.is_open()) return 0.0;
    std::string line;
    bool isHeader = true;
    double fine = 0.0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        if (isHeader) {
            isHeader = false;
            continue;
        }
        Row row = parseCSVLine(line);
        
        // Debugging print statement showing the data being parsed while iterating
        if (row.size >= 9) {
            std::cout << "[DEBUG] Parsing fee record - Roll: " << row.fields[1]
                      << ", Sem: " << row.fields[2]
                      << ", Total: " << row.fields[3]
                      << ", Paid: " << row.fields[4]
                      << ", Status: " << row.fields[8] << "\n";
        }

        if (row.size >= 9 && row.fields[1] == rollNo && row.fields[2] == std::to_string(semester)) {
            std::string dueDate = row.fields[5];
            std::string payDate = row.fields[6];
            if (payDate != "00-00-0000" && payDate != "NONE") {
                int diff = daysBetween(dueDate, payDate);
                if (diff > 0) {
                    fine = diff * 100.0; // Rs. 100 per day fine
                }
            }
            break;
        }
    }
    file.close();
    return fine;
}

bool recordPayment(const std::string& rollNo, int semester, double amount, const std::string& date, const std::string& method) {
    std::ifstream file("fees.txt");
    if (!file.is_open()) {
        std::cout << "Error: Could not open fees.txt\n";
        return false;
    }
    std::ofstream tempFile("temp_fees.txt");
    if (!tempFile.is_open()) {
        file.close();
        std::cout << "Error: Could not create temp_fees.txt\n";
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
        if (row.size >= 9) {
            std::cout << "[DEBUG] Parsing fee record - Roll: " << row.fields[1]
                      << ", Sem: " << row.fields[2]
                      << ", Total: " << row.fields[3]
                      << ", Paid: " << row.fields[4]
                      << ", Status: " << row.fields[8] << "\n";
        }

        if (row.size >= 9 && row.fields[1] == rollNo && row.fields[2] == std::to_string(semester)) {
            double totalFee = std::stod(row.fields[3]);
            double currentPaid = std::stod(row.fields[4]);
            double newPaid = currentPaid + amount;
            row.fields[4] = std::to_string((long long)newPaid);
            row.fields[6] = date;
            row.fields[7] = method;
            
            if (newPaid >= totalFee) {
                int diff = daysBetween(row.fields[5], date);
                if (diff > 0) {
                    row.fields[8] = "paid_late";
                } else {
                    row.fields[8] = "paid";
                }
            } else if (newPaid > 0) {
                row.fields[8] = "partial";
            } else {
                row.fields[8] = "unpaid";
            }
            found = true;
        }
        tempFile << formatRowAsCSV(row) << "\n";
    }
    file.close();
    tempFile.close();

    if (!found) {
        std::remove("temp_fees.txt");
        std::cout << "Fee record not found for student " << rollNo << " in semester " << semester << ".\n";
        return false;
    }

    std::remove("fees.txt");
    std::rename("temp_fees.txt", "fees.txt");
    std::cout << "Payment recorded successfully.\n";
    return true;
}

void generateReceipt(const std::string& rollNo, int semester) {
    std::ifstream file("fees.txt");
    if (!file.is_open()) {
        std::cout << "No fee records found.\n";
        return;
    }
    std::string line;
    bool isHeader = true;
    bool found = false;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        if (isHeader) {
            isHeader = false;
            continue;
        }
        Row row = parseCSVLine(line);
        
        // Debugging print statement showing the data being parsed while iterating
        if (row.size >= 9) {
            std::cout << "[DEBUG] Parsing fee record - Roll: " << row.fields[1]
                      << ", Sem: " << row.fields[2]
                      << ", Total: " << row.fields[3]
                      << ", Paid: " << row.fields[4]
                      << ", Status: " << row.fields[8] << "\n";
        }

        if (row.size >= 9 && row.fields[1] == rollNo && row.fields[2] == std::to_string(semester)) {
            found = true;
            double totalFee = std::stod(row.fields[3]);
            double amountPaid = std::stod(row.fields[4]);
            double fine = computeLateFine(rollNo, semester);
            std::cout << "\n====================================\n";
            std::cout << "             FEE RECEIPT             \n";
            std::cout << "====================================\n";
            std::cout << "Roll Number:    " << row.fields[1] << "\n";
            std::cout << "Semester:       " << row.fields[2] << "\n";
            std::cout << "Total Fee:      Rs. " << (long long)totalFee << "\n";
            std::cout << "Amount Paid:    Rs. " << (long long)amountPaid << "\n";
            std::cout << "Late Fine:      Rs. " << (long long)fine << "\n";
            std::cout << "Remaining:      Rs. " << (long long)(totalFee + fine - amountPaid) << "\n";
            std::cout << "Due Date:       " << row.fields[5] << "\n";
            std::cout << "Payment Date:   " << row.fields[6] << "\n";
            std::cout << "Method:         " << row.fields[7] << "\n";
            std::cout << "Status:         " << row.fields[8] << "\n";
            std::cout << "====================================\n";
            break;
        }
    }
    file.close();
    if (!found) {
        std::cout << "Fee record not found for student " << rollNo << " in semester " << semester << ".\n";
    }
}

Table getDefaulters() {
    Table results;
    results.size = 0;
    std::ifstream file("fees.txt");
    if (!file.is_open()) return results;
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
        if (row.size >= 9) {
            std::cout << "[DEBUG] Parsing fee record - Roll: " << row.fields[1]
                      << ", Sem: " << row.fields[2]
                      << ", Total: " << row.fields[3]
                      << ", Paid: " << row.fields[4]
                      << ", Status: " << row.fields[8] << "\n";
        }

        if (row.size >= 9) {
            double totalFee = std::stod(row.fields[3]);
            double amountPaid = std::stod(row.fields[4]);
            std::string status = row.fields[8];
            if (status == "unpaid" || status == "partial" || amountPaid < totalFee) {
                if (results.size < MAX_ROWS) {
                    results.rows[results.size++] = row;
                }
            }
        }
    }
    file.close();
    return results;
}

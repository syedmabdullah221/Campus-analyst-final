#include <string>
#include "filehandler.h"
#include <fstream>
#include <iostream>
#include <sstream>

Row parseCSVLine(const std::string& line) {
    Row r;
    r.size = 0;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) {
        // If field starts with a double quote but doesn't end with one,
        // it means there was a comma inside quotes. Continue reading until closing quote.
        if (!field.empty() && field.front() == '"' && (field.back() != '"' || field.length() == 1)) {
            std::string temp;
            while (std::getline(ss, temp, ',')) {
                field += "," + temp;
                if (!temp.empty() && temp.back() == '"') {
                    break;
                }
            }
        }
        // Remove surrounding quotes if present
        if (!field.empty() && field.front() == '"' && field.back() == '"') {
            field = field.substr(1, field.length() - 2);
        }
        if (r.size < MAX_FIELDS) {
            r.fields[r.size++] = field;
        }
    }
    return r;
}

std::string formatRowAsCSV(const Row& row) {
    std::string line = "";
    for (int j = 0; j < row.size; ++j) {
        const std::string& field = row.fields[j];
        bool needsQuotes = false;
        for (int k = 0; k < (int)field.length(); ++k) {
            char ch = field[k];
            if (ch == ',' || ch == '"' || ch == '\n' || ch == '\r') {
                needsQuotes = true;
                break;
            }
        }
        if (needsQuotes) {
            line += '"';
            for (int k = 0; k < (int)field.length(); ++k) {
                char ch = field[k];
                if (ch == '"') {
                    line += "\"\"";
                } else {
                    line += ch;
                }
            }
            line += '"';
        } else {
            line += field;
        }
        if (j < row.size - 1) {
            line += ",";
        }
    }
    return line;
}

bool readTXT(const std::string& filename, Table& outTable) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    outTable.size = 0;
    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        // Remove carriage return if present (Windows CRLF line endings)
        if (line.back() == '\r') {
            line.pop_back();
        }
        if (isHeader) {
            isHeader = false; // Skip the header row
            continue;
        }

        Row row = parseCSVLine(line);

        // Check if the row has any non-empty fields to avoid adding completely empty rows
        bool hasData = false;
        for (int i = 0; i < row.size; ++i) {
            if (!row.fields[i].empty()) {
                hasData = true;
                break;
            }
        }
        if (hasData && outTable.size < MAX_ROWS) {
            outTable.rows[outTable.size++] = row;
        }
    }

    file.close();
    return true;
}

bool writeTXT(const std::string& filename, const std::string& header, const Table& table) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // Write header
    file << header << "\n";

    // Write each row
    for (int i = 0; i < table.size; ++i) {
        const Row& row = table.rows[i];
        for (int j = 0; j < row.size; ++j) {
            const std::string& field = row.fields[j];
            bool needsQuotes = false;
            for (int k = 0; k < (int)field.length(); ++k) {
                char ch = field[k];
                if (ch == ',' || ch == '"' || ch == '\n' || ch == '\r') {
                    needsQuotes = true;
                    break;
                }
            }

            if (needsQuotes) {
                file << '"';
                for (int k = 0; k < (int)field.length(); ++k) {
                    char ch = field[k];
                    if (ch == '"') {
                        file << "\"\""; // escape quote
                    } else {
                        file << ch;
                    }
                }
                file << '"';
            } else {
                file << field;
            }

            if (j < row.size - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
    return true;
}

bool appendTXT(const std::string& filename, const Row& row) {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        return false;
    }

    for (int j = 0; j < row.size; ++j) {
        const std::string& field = row.fields[j];
        bool needsQuotes = false;
        for (int k = 0; k < (int)field.length(); ++k) {
            char ch = field[k];
            if (ch == ',' || ch == '"' || ch == '\n' || ch == '\r') {
                needsQuotes = true;
                break;
            }
        }

        if (needsQuotes) {
            file << '"';
            for (int k = 0; k < (int)field.length(); ++k) {
                char ch = field[k];
                if (ch == '"') {
                    file << "\"\"";
                } else {
                    file << ch;
                }
            }
            file << '"';
        } else {
            file << field;
        }

        if (j < row.size - 1) {
            file << ",";
        }
    }
    file << "\n";
    file.close();
    return true;
}

Row findRow(const std::string& filename, const std::string& query, int colIndex) {
    Table* table = new Table();
    Row foundRow;
    foundRow.size = 0;
    if (!readTXT(filename, *table)) {
        delete table;
        return foundRow;
    }
    for (int i = 0; i < table->size; ++i) {
        if (colIndex < table->rows[i].size && table->rows[i].fields[colIndex] == query) {
            foundRow = table->rows[i];
            break;
        }
    }
    delete table;
    return foundRow;
}

bool rowExists(const std::string& filename, const std::string& query, int colIndex) {
    Table* table = new Table();
    if (!readTXT(filename, *table)) {
        delete table;
        return false;
    }
    for (int i = 0; i < table->size; ++i) {
        if (colIndex < table->rows[i].size && table->rows[i].fields[colIndex] == query) {
            delete table;
            return true;
        }
    }
    delete table;
    return false;
}

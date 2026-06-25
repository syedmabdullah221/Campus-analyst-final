#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include "common.h"

// Read the TXT file (skipping the header) into the given Table.
// Parses character-by-character to correctly handle commas inside quotes.
bool readTXT(const std::string& filename, Table& outTable);

// Overwrites the file with the given header line and all rows in the Table.
// Fields containing commas or quotes are appropriately wrapped in quotes.
bool writeTXT(const std::string& filename, const std::string& header, const Table& table);

// Appends a single Row to the file without loading the entire file into memory.
// Appropriately wraps fields in quotes if necessary.
bool appendTXT(const std::string& filename, const Row& row);

// Performs a linear search on rows. Returns the first matching Row, or an empty Row if not found.
Row findRow(const std::string& filename, const std::string& query, int colIndex);

// Returns true if a row exists with the specified value at the given column index.
bool rowExists(const std::string& filename, const std::string& query, int colIndex);

// Parses a single CSV line into a Row struct, handling commas in double quotes correctly.
Row parseCSVLine(const std::string& line);

// Converts a Row struct back into a comma-separated CSV string.
std::string formatRowAsCSV(const Row& row);

#endif // FILEHANDLER_H

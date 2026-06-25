# Campus-analyst-final
A C++ console-based system for managing student, course, and fee records, built strictly without utilizing Object-Oriented Programming (OOP) classes, Standard Template Library (STL) container/algorithm headers (such as `<vector>`, `<map>`, `<algorithm>`), or `<ctime>`. The system uses custom structure representations and manual, character-by-character parsing to correctly handle comma-separated files and quoted fields.

## Features

The Campus Analytics Engine contains the following core modules:

*   **Student Management Module** (`student_ops`):
    *   Add new student profiles.
    *   Search student records by Roll Number or Name (with case-insensitive substring matching).
    *   Update student details (Name, Department, Semester, CGPA, Status).
    *   Perform soft deletions (marking profiles inactive).
    *   List all active students.
*   **Course Management Module** (`course_ops`):
    *   Enroll students in courses and track capacity.
    *   Drop courses and adjust enrollment counts.
    *   Check credit hour loads and verify course prerequisites.
*   **Attendance Management Module** (`attendance`):
    *   Mark daily attendance sessions (Present, Absent, Late).
    *   Calculate attendance percentages.
    *   Generate attendance shortage lists (students below 75% threshold).
    *   Undo the last attendance session.
*   **Grade Management Module** (`grades`):
    *   Enter quiz, assignment, and exam marks.
    *   Compute best 3-of-5 quizzes.
    *   Calculate weighted totals and assign letter grades.
    *   Apply attendance-based grade penalties.
*   **Fee Tracker Module** (`fee_tracker`):
    *   Record fee payments and track partial payments.
    *   Calculate late fines using a custom day-difference date algorithm (without `<ctime>`).
    *   Generate payment receipts.
*   **Report Generation Module** (`reports`):
    *   Print merit lists sorted by CGPA.
    *   Print lists of attendance defaulters.
    *   Print departmental summaries (average GPA, enrollment statistics).

## Compilation Instructions

To compile the application, use any standard C++ compiler supporting C++11 or later. Open your command terminal, navigate to the project directory, and run:

```bash
g++ *.cpp -o campus_analytics.exe
```

## How to Run

After compilation, launch the menu-driven command-line interface:

*   **On Windows:**
    ```cmd
    campus_analytics.exe
    ```
*   **On macOS/Linux:**
    ```bash
    ./campus_analytics.exe
    ```

Once started, navigate through the 3-level nested interactive menus to access and perform operations across all modules

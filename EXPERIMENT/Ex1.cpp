// Experiment 1: UNIX Permission and umask Calculator
// permcalc_simple.cpp
//
// Goal:
//  - Take MODE and UMASK from the user (both required)
//  - Compute effective permission: effective = mode & (~umask) & 0777
//  - Print result in 4-digit octal and symbolic (rwxrwxrwx) form
//
// Build: g++ -std=c++17 -O2 -Wall permcalc_simple.cpp -o permcalc
// Run  : ./permcalc

#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>

using namespace std;

// Print error messages in a consistent format.
// We separate this into a function so error printing is the same everywhere.
static void error_out(const string& code, const string& msg) {
    cout << "ERROR: " << code << ": " << msg << "\n";
}

// Check if a string is exactly 4-digit octal.
// Why 4 digits?
//  - In UNIX permission examples we typically write permissions like 0644, 0755, 0022.
//  - Each digit must be 0..7 because octal (base-8) uses digits 0..7 only.
static bool is_valid_octal4(const string& s) {
    if (s.size() != 4) return false;
    for (char c : s) {
        if (c < '0' || c > '7') return false;
    }
    return true;
}

// Convert 4-digit octal string (e.g., "0644") to integer.
// Why multiply by 8?
//  - Octal is base-8, so each step shifts left by 3 bits (same as multiply by 8).
static int octal_to_int(const string& s) {
    int value = 0;
    for (char c : s) {
        value = value * 8 + (c - '0');
    }
    return value;
}

// Convert integer permission value to 4-digit octal string.
// Why "& 0777"?
//  - We only care about the lower 9 permission bits (rwx for user/group/other).
//  - 0777 in octal = binary 111111111 (9 bits).
static string int_to_octal4(int value) {
    stringstream ss;
    ss << setfill('0') << setw(4) << oct << (value & 0777);
    return ss.str();
}

// Convert one permission triad (0..7) into "rwx" style string.
// Example:
//  - 7 (111b) => rwx
//  - 5 (101b) => r-x
//  - 4 (100b) => r--
//  - 0 (000b) => ---
static string triad_to_rwx(int bits) {
    string out;
    out += (bits & 4) ? 'r' : '-'; // 4 means read bit
    out += (bits & 2) ? 'w' : '-'; // 2 means write bit
    out += (bits & 1) ? 'x' : '-'; // 1 means execute bit
    return out;
}

// Convert full mode (lower 9 bits) to symbolic permissions.
// Why shifts?
//  - Permission bits are grouped as:
//      user  (bits 8..6), group (bits 5..3), other (bits 2..0)
//  - Shifting extracts each group as a number 0..7.
static string mode_to_symbolic(int mode) {
    int user  = (mode >> 6) & 7;
    int group = (mode >> 3) & 7;
    int other = (mode >> 0) & 7;

    return triad_to_rwx(user) + triad_to_rwx(group) + triad_to_rwx(other);
}

int main() {
    string modeStr, umaskStr;

    // 1) Take MODE from user (required)
    cout << "Enter file mode (4-digit octal, e.g., 0644): ";
    cin >> modeStr;

    // Validate MODE format
    if (!is_valid_octal4(modeStr)) {
        error_out("E_OCTAL", "mode must be 4-digit octal (0000-0777)");
        return 1;
    }

    // 2) Take UMASK from user (required)
    cout << "Enter umask (4-digit octal, e.g., 0022): ";
    cin >> umaskStr;

    // Validate UMASK format
    if (!is_valid_octal4(umaskStr)) {
        error_out("E_OCTAL", "umask must be 4-digit octal (0000-0777)");
        return 1;
    }

    // 3) Convert both inputs to integers
    int mode  = octal_to_int(modeStr);
    int umask = octal_to_int(umaskStr);

    // Extra safety range check (should already be fine if input is octal4)
    if (mode > 0777) {
        error_out("E_RANGE", "mode out of range (0000-0777)");
        return 1;
    }
    if (umask > 0777) {
        error_out("E_RANGE", "umask out of range (0000-0777)");
        return 1;
    }

    // 4) Compute effective permissions
    //
    // UNIX rule:
    //  - requested mode says what permissions we WANT
    //  - umask says which permissions must be REMOVED/blocked
    //
    // So:
    //  effective = mode & (~umask)
    //
    // Finally "& 0777" keeps only the last 9 bits (rwxrwxrwx)
    int effective = (mode & (~umask)) & 0777;

    // 5) Print outputs
    cout << "\nOK: EFFECTIVE " << int_to_octal4(effective) << "\n";
    cout << "OK: SYMBOLIC " << mode_to_symbolic(effective) << "\n";

    return 0;
}

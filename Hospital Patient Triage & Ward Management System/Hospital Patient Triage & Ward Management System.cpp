#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <ctime>

using namespace std;

struct Patient {
    int id;
    string name;
    int age;
    string symptomCode;
    int priority;         
    string wardAssigned;
};

struct Ward {
    string name;
    int maxBeds;
    int occupiedBeds;
};

Ward wards[3] = {
    {"ICU", 5, 0},        
    {"Emergency", 10, 0},  
    {"General", 15, 0}     
};

bool comparePatients(const Patient& a, const Patient& b) {
    return a.priority < b.priority;
}

string getCurrentTimestamp() {
    time_t now = time(0);
    char dt[26];
#if defined(_MSC_VER)
    ctime_s(dt, sizeof(dt), &now);
#else
    snprintf(dt, sizeof(dt), "%s", ctime(&now));
#endif
    string timeStr(dt);
    if (!timeStr.empty() && timeStr.back() == '\n') {
        timeStr.pop_back();
    }
    return timeStr;
}

void logEvent(string message) {
    ofstream auditFile("hospital_audit.txt", ios::app);
    if (auditFile.is_open()) {
        auditFile << "[" << getCurrentTimestamp() << "] " << message << endl;
        auditFile.close();
    }
}

bool assignBed(string wardName) {
    for (int i = 0; i < 3; i++) {
        if (wards[i].name == wardName) {
            if (wards[i].occupiedBeds < wards[i].maxBeds) {
                wards[i].occupiedBeds++;
                return true;
            }
            else {
                return false; 
            }
        }
    }
    return false;
}

void freeBed(string wardName) {
    for (int i = 0; i < 3; i++) {
        if (wards[i].name == wardName && wards[i].occupiedBeds > 0) {
            wards[i].occupiedBeds--;
            break;
        }
    }
}

void admitPatient(vector<Patient>& patients) {
    Patient p;
    cout << "\n--- Patient Admission (Receptionist View) ---\n";
    cout << "Enter Patient ID: ";
    cin >> p.id;

    for (size_t i = 0; i < patients.size(); i++) {
        if (patients[i].id == p.id) {
            cout << "Error: Patient ID already exists!\n";
            return;
        }
    }

    cout << "Enter Patient Name: ";
    cin.ignore();
    getline(cin, p.name);

    cout << "Enter Age: ";
    cin >> p.age;

    cout << "Enter Symptom Code (e.g., F10, C20): ";
    cin >> p.symptomCode;

    cout << "Enter Priority Level (1 = Critical/Red, 2 = Urgent/Yellow, 3 = Stable/Green): ";
    cin >> p.priority;

    if (p.priority == 1) {
        p.wardAssigned = "ICU";
    }
    else if (p.priority == 2) {
        p.wardAssigned = "Emergency";
    }
    else {
        p.wardAssigned = "General";
    }

    if (!assignBed(p.wardAssigned)) {
        cout << "Admission Failed: No available beds in " << p.wardAssigned << " Ward!\n";
        logEvent("FAILED ADMISSION: No beds available in " + p.wardAssigned + " for Patient ID " + to_string(p.id));
        return;
    }

    patients.push_back(p);
    sort(patients.begin(), patients.end(), comparePatients);

    cout << "Patient " << p.name << " admitted successfully to " << p.wardAssigned << " Ward.\n";
    logEvent("ADMITTED: " + p.name + " (ID: " + to_string(p.id) + ") assigned to " + p.wardAssigned + " [Priority: " + to_string(p.priority) + "]");
}

void displayWardOccupancy() {
    cout << "\n--- Hospital Ward & Bed Occupancy Report ---\n";
    cout << left << setw(15) << "Ward Name" << setw(15) << "Max Capacity" << setw(15) << "Occupied" << setw(15) << "Available" << endl;
    cout << "------------------------------------------------------------\n";
    for (int i = 0; i < 3; i++) {
        cout << left << setw(15) << wards[i].name
            << setw(15) << wards[i].maxBeds
            << setw(15) << wards[i].occupiedBeds
            << setw(15) << (wards[i].maxBeds - wards[i].occupiedBeds) << endl;
    }
}

void displayPatientQueue(const vector<Patient>& patients) {
    cout << "\n--- Emergency Triage Queue (Sorted by Severity) ---\n";
    if (patients.empty()) {
        cout << "No active patients in the queue.\n";
        return;
    }
    cout << left << setw(8) << "Queue#" << setw(10) << "ID" << setw(20) << "Name" << setw(8) << "Age" << setw(12) << "Priority" << setw(12) << "Ward" << endl;
    cout << "----------------------------------------------------------------------\n";
    for (size_t i = 0; i < patients.size(); i++) {
        string priorityStr = (patients[i].priority == 1) ? "1-Critical" : (patients[i].priority == 2) ? "2-Urgent" : "3-Stable";
        cout << left << setw(8) << (i + 1)
            << setw(10) << patients[i].id
            << setw(20) << patients[i].name
            << setw(8) << patients[i].age
            << setw(12) << priorityStr
            << setw(12) << patients[i].wardAssigned << endl;
    }
}

void dischargePatient(vector<Patient>& patients) {
    if (patients.empty()) {
        cout << "No patients to discharge.\n";
        return;
    }
    int targetId;
    cout << "Enter Patient ID to discharge: ";
    cin >> targetId;

    for (auto it = patients.begin(); it != patients.end(); ++it) {
        if (it->id == targetId) {
            freeBed(it->wardAssigned); 

            cout << "Patient " << it->name << " discharged successfully. Bed freed in " << it->wardAssigned << " Ward.\n";
            logEvent("DISCHARGED: Patient " + it->name + " (ID: " + to_string(targetId) + ") freed bed in " + it->wardAssigned);

            patients.erase(it); 
            return;
        }
    }
    cout << "Patient ID " << targetId << " not found!\n";
}

void receptionistMenu(vector<Patient>& patients) {
    int choice;
    do {
        cout << "\n=== RECEPTIONIST MENU ===\n";
        cout << "1. Admit New Patient\n";
        cout << "2. View Ward Bed Occupancy Report\n";
        cout << "3. Back to Main Menu\n";
        cout << "Choice: ";
        cin >> choice;

        switch (choice) {
        case 1: admitPatient(patients); break;
        case 2: displayWardOccupancy(); break;
        case 3: cout << "Returning to Main Menu...\n"; break;
        default: cout << "Invalid choice!\n";
        }
    } while (choice != 3);
}

void doctorMenu(vector<Patient>& patients) {
    int choice;
    do {
        cout << "\n=== DOCTOR / TRIAGE MENU ===\n";
        cout << "1. View Active Patient Queue (Triage Order)\n";
        cout << "2. Discharge Patient\n";
        cout << "3. View Ward Bed Occupancy Report\n";
        cout << "4. Back to Main Menu\n";
        cout << "Choice: ";
        cin >> choice;

        switch (choice) {
        case 1: displayPatientQueue(patients); break;
        case 2: dischargePatient(patients); break;
        case 3: displayWardOccupancy(); break;
        case 4: cout << "Returning to Main Menu...\n"; break;
        default: cout << "Invalid choice!\n";
        }
    } while (choice != 4);
}

int main() {
    vector<Patient> patients;
    int roleChoice;

    do {
        cout << "\n=============================================\n";
        cout << " HOSPITAL PATIENT TRIAGE & WARD SYSTEM \n";
        cout << "=============================================\n";
        cout << "Select Access Role:\n";
        cout << "1. Receptionist View\n";
        cout << "2. Doctor / Triage View\n";
        cout << "3. Exit System\n";
        cout << "Enter your choice (1-3): ";
        cin >> roleChoice;

        switch (roleChoice) {
        case 1: receptionistMenu(patients); break;
        case 2: doctorMenu(patients); break;
        case 3: cout << "Exiting System. Good Bye!\n"; break;
        default: cout << "Invalid choice! Enter 1, 2, or 3.\n";
        }
    } while (roleChoice != 3);

    return 0;
}
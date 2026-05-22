#include <iostream>
#include <fstream>
#include <iomanip>
#include<string>
#include <cstring>
#include <limits> // For numeric_limits

using namespace std;

// --- Structures ---

struct Refugee {
    int refugeeID;
    char name[50];
    int age;
    char gender[10];
    char location[50];
    int familyID;
    char status[20]; // "separated" or "reunited"
};
struct FamilyCase {
    int familyID;
    int memberIDs[10];   // IDs of family members
    char lastContact[20];
    bool reunited;
};
struct Aid {
    int aidID;
    char type[20];                  // food, water, medicine
    int quantity;
    char distributionLocation[50];
    char description[50];
    int* recipientIDs;              // dynamic array for recipient IDs
    int numRecipients;              // number of recipients entered
};
struct Incident {
    int incidentID;
    char type[50];       // e.g., GBV, theft, safety
    int victimID;
    char location[50];
    char severity[10];   // low, medium, high
    char date[20];       // e.g., 2025-12-26
    char time[10];       // e.g., 14:30
    char status[20];     // resolved/unresolved
};

// --- Helper Functions ---

// functions to make sure we get proper Date and Time
// Checks if a portion of a string contains only digits
bool isDigitString(const char* s, int start, int end) {
    for (int i = start; i <= end; i++) {
        if (!isdigit(s[i])) 
        return false;
    }
    return true;
}
// Validates a date string in YYYY-MM-DD format
bool isValidDate(const char* date) {
    // Check length YYYY-MM-DD
    if (strlen(date) != 10) return false;
    // Check format
    if (date[4] != '-' || date[7] != '-') return false;
    // Check digits
    if (!isDigitString(date, 0, 3) ||
        !isDigitString(date, 5, 6) ||
        !isDigitString(date, 8, 9))
        return false;

    int year  = atoi(date);
    int month = atoi(date + 5);
    int day   = atoi(date + 8);

    if (month < 1 || month > 12) return false;

    int maxDays=30;
  //  if (month == 2) maxDays = 28;
    // else if (month == 4 || month == 6 || month == 9 || month == 11)
    //     maxDays = 30;
    // else
    //     maxDays = 31;

    return (day >= 1 && day <= maxDays);
}
// Validates a time string in HH:MM format
bool isValidTime(const char* time) {
    // Check length HH:MM
    if (strlen(time) != 5) return false;

    if (time[2] != ':') return false;

    if (!isDigitString(time, 0, 1) ||
        !isDigitString(time, 3, 4))
        return false;

    int hour   = atoi(time);
    int minute = atoi(time + 3);

    return (hour >= 0 && hour <= 23 &&
            minute >= 0 && minute <= 59);
}

// Prompts the user to select and store a refugee location
bool getRefugeeLocation(char* location) {
    int attempts = 0;
    bool valid = false;

    while (attempts < 3) {
        cout << "Select Refugee Location:\n";
        cout << "1. Tent Zone\n";
        cout << "2. Shelter Block\n";
        cout << "3. Housing Unit\n";
        cout << "4. Family Shelter\n";
        cout << "5. Temporary Shelter\n";
        cout << "Enter choice (1–5): ";

        string input;
        cin >> input;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        // Check if input is numeric
        bool isNumber = true;
        for (char c : input) {
            if (!isdigit(c)) {
                isNumber = false;
                break;
            }
        }

        if (!isNumber) {
            attempts++;
            cout << "Invalid input (not a number). Attempts left: "
                 << (3 - attempts) << "\n";
            continue;
        }

        int choice = stoi(input);

        switch (choice) {
            case 1: strcpy(location, "Tent Zone"); valid = true; break;
            case 2: strcpy(location, "Shelter Block"); valid = true; break;
            case 3: strcpy(location, "Housing Unit"); valid = true; break;
            case 4: strcpy(location, "Family Shelter"); valid = true; break;
            case 5: strcpy(location, "Temporary Shelter"); valid = true; break;
            default:
                attempts++;
                cout << "Invalid choice. Attempts left: "
                     << (3 - attempts) << "\n";
        }

        if (valid)
            return true;
    }

    cout << "You have used all 3 attempts. Returning to previous menu.\n";
    return false;
}
// Generates the next ID based on file size and record size
int getNextID(const char* filename, int startID, int recSize) {
    ifstream file(filename, ios::binary | ios::ate);
    if (!file) return startID;

    long size = file.tellg();
    if (size < recSize) return startID;

    file.seekg(size - recSize);
    int lastID;
    file.read(reinterpret_cast<char*>(&lastID), sizeof(int));
    file.close();

    // ensure the ID is at least startID
    if (lastID < startID) return startID;
    return lastID + 1;
}
// Generates the next Aid ID by scanning all aid records

int getNextAidID(const char* filename, int startID) {
    ifstream file(filename, ios::binary);
    if (!file) return startID;

    int lastID = startID - 1;
    Aid a; // temporary struct to read full record

    while (file.read(reinterpret_cast<char*>(&a.aidID), sizeof(int))) {
        file.read(reinterpret_cast<char*>(a.type), sizeof(a.type));
        file.read(reinterpret_cast<char*>(&a.quantity), sizeof(int));
        file.read(reinterpret_cast<char*>(a.distributionLocation), sizeof(a.distributionLocation));
        file.read(reinterpret_cast<char*>(a.description), sizeof(a.description));
        file.read(reinterpret_cast<char*>(&a.numRecipients), sizeof(int));

        // skip recipients
        int* recs = new int[a.numRecipients];
        file.read(reinterpret_cast<char*>(recs), sizeof(int) * a.numRecipients);
        delete[] recs;

        lastID = a.aidID; // store last ID
    }

    file.close();
    return lastID + 1;
}

// Safely reads an integer input from the user
int getIntInput() {
    int value;
    while (!(cin >> value )|| value < 0) {
        cin.clear(); // clear error flag
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard invalid input
        cout << "Invalid input. Please enter a number: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear buffer after valid input
    return value;
}

// Authenticates users based on role and password
void showProgramInfo() {
    cout << "\n=====================================================\n";
    cout << "   Post-Conflict Humaniatarian Aid Distribution and Family Reunification System   \n";
    cout << "=====================================================\n\n";

    cout << "This system is designed to support humanitarian\n";
    cout << "operations in post-conflict and displacement settings in Refugee camps.\n\n";

    cout << "Our main services include:\n";
    cout << "• Registration and management of refugees\n";
    cout << "• Family reunification and case tracking\n";
    cout << "• Recording and distribution of humanitarian aid\n";
    cout << "• Incident reporting and safety monitoring\n";
    cout << "• Analytical reporting for decision-makers\n\n";

    cout << "If you are not an authorized system user,\n";
    cout << "you may still contact us for assistance or information.\n\n";

    cout << "📍 Office Address:\n";
    cout << "Humanitarian Coordination Office\n";
    cout << "Camp Administration Zone, Sector B\n";
    cout << "Mekelle, Tigray , Ethiopia\n\n";

    cout << "📞 Contact Information:\n";
    cout << "Phone: +251 900 000 000\n";
    cout << "Email: supportrefugees@Idpcamps.org\n\n";

    cout << "👤 Who to Contact:\n";
    cout << "• Camp Administrator\n";
    cout << "• Protection & Safety Officer\n";
    cout << "• Family Reunification Officer\n\n";

    cout << "Thank you for your interest.\n";
    cout << "For security and privacy reasons, system access\n";
    cout << "is limited to authorized personnel only.\n";
    cout << "=====================================================\n";
}


// --- Authentication Function with 3 attempts ---
bool authenticateUser() {
    cout << "=== User Authentication ===\n";

    int attempts = 0;
    const int maxAttempts = 3;

    while (attempts < maxAttempts) {
        cout << "Select your role from the numbers given:\n";
        cout << "1. Admin\n";
        cout << "2. Staff\n";
        cout << "3. Volunteer\n";
        cout<< " 4. Other\n";
        cout << "Enter choice (1-4): ";

        int roleChoice = getIntInput();
        string password;

        switch(roleChoice) {
            case 1: // Admin
                cout << "Enter Admin password: ";
                cin >> password;
                if(password == "admin123") {
                    cout << "Access granted. Welcome Admin!\n";
                    return true;
                }
                break;

            case 2: // Staff
                cout << "Enter Staff password: ";
                cin >> password;
                if(password == "staff456") {
                    cout << "Access granted. Welcome Staff!\n";
                    return true;
                }
                break;

            case 3: // Volunteer
                cout << "Enter Volunteer password: ";
                cin >> password;
                if(password == "vol789") {
                    cout << "Access granted. Welcome Volunteer!\n";
                    return true;
                }
                break;
            case 4:
                cout<<"\nYou are not registered as an Admin, Staff, or Volunteer.\n.However, you can still get all the information needed here.";
                showProgramInfo();
                return false;

            default:
                cout << "Invalid role selection!\n";
                attempts++;
                continue; // skip the rest of loop, count as a failed attempt
        }

        // Password incorrect
        attempts++;
        if(attempts < maxAttempts)
            cout << "Incorrect password. You have " << (maxAttempts - attempts) << " attempt(s) left.\n";
        else
            cout << "You have used all attempts. Access denied.\n";
    }

    return false; // after 3 failed attempts
}


// Pauses program execution until Enter is pressed
void pauseSystem() {
    cout << "\nPress Enter to continue...";
    cin.get();
}

// Get validated age 
int getValidatedAge() {
    int age;
    while (true) {
        cout << "Enter Age (0–100): ";
        age = getIntInput();
        if (age < 0 || age > 100) {
            cout << "Invalid age. Try again.\n";
        } else {
            return age;
        }
    }
}

//Get gender choice 
void getGender(char* gender) {
    char choice;
    while (true) {
        cout << "Choose Gender (M/F): ";
        cin >> choice;
  cin.ignore(1000, '\n');  // clear input buffer

        choice = toupper(choice);
        if (choice == 'M') {
            strcpy(gender, "Male");
            return;
        }
        else if (choice == 'F') {
            strcpy(gender, "Female");
            return;
        }
        else {
            cout << "Invalid choice. Try again.\n";
        }
    }
}

// Get status choice 
void getStatus(char* status) {
    char choice;
    while (true) {
        cout << "Status (S = Separated, R = Reunited): ";
        cin >> choice;
  cin.ignore(1000, '\n');  // clear input buffer

        choice = toupper(choice);
        if (choice == 'S') {
            strcpy(status, "Separated");
            return;
        }
        else if (choice == 'R') {
            strcpy(status, "Reunited");
            return;
        }
        else {
            cout << "Invalid choice. Try again.\n";
        }
    }
}

// Make first letter of each word uppercase, others lowercase 
void formatName(char* name) {
    bool newWord = true;  // true at the start and after each space

    for (int i = 0; name[i] != '\0'; ++i) {
        unsigned char c = static_cast<unsigned char>(name[i]);

        if (c == ' ') {
            newWord = true;           // next non-space starts a new word
        } else {
            if (newWord) {
                // first character of this word
                name[i] = static_cast<char>(toupper(c));
                newWord = false;
            } else {
                // inside a word
                name[i] = static_cast<char>(tolower(c));
            }
        }
    }
}

/* Check if string contains only letters and spaces */
bool isValidName(const char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        if (!isalpha(c) && c != ' ')
            return false;
    }
    return true;
}

/* Get validated string input */
void getValidatedString(const char* prompt, char* buffer, int size) {
    while (true) {
        cout << prompt;
        cin.getline(buffer, size);

        if (strlen(buffer) == 0) {
            cout << "Input cannot be empty. Try again.\n";
        } else if (!isValidName(buffer)) {
            cout << "Invalid input. Letters only.\n";
        } else {
            formatName(buffer);  // will capitalize each word
            return;
        }
    }
}
// Adds a refugee ID to an existing family record
bool addMemberToFamily(int familyID, int refugeeID) {
    fstream file("family.dat", ios::in | ios::out | ios::binary);
    if (!file) return false;

    FamilyCase f;
    while (file.read(reinterpret_cast<char*>(&f), sizeof(FamilyCase))) {
        if (f.familyID == familyID) {

            // check if already exists
            for (int i = 0; i < 10; i++) {
                if (f.memberIDs[i] == refugeeID) {
                    file.close();
                    return true; // already added
                }
            }

            // add to first empty slot
            for (int i = 0; i < 10; i++) {
                if (f.memberIDs[i] == 0) {
                    f.memberIDs[i] = refugeeID;
                    file.seekp(-static_cast<long>(sizeof(FamilyCase)), ios::cur);
                    file.write(reinterpret_cast<char*>(&f), sizeof(FamilyCase));
                    file.close();
                    return true;
                }
            }

            file.close();
            return false; // family full
        }
    }

    file.close();
    return false;
}

// search refugee using overload function
// search by id
void searchRefugee(int id) {
    ifstream file("refugees.dat", ios::binary);
    Refugee r;
    bool found = false;

    while (file.read(reinterpret_cast<char*>(&r), sizeof(r))) {
        if (r.refugeeID == id) {
            cout << "\nRefugee Found:\n";
            cout << "ID: " << r.refugeeID
                 << "\nName: " << r.name
                 << "\nAge: " << r.age
                 << "\nGender: " << r.gender
                 << "\nLocation: " << r.location
                 << "\nFamily ID: " << r.familyID
                 << "\nStatus: " << r.status << endl;
            found = true;
            break;
        }
    }

    if (!found)
        cout << "Refugee not found.\n";

    file.close();
}

//search by name
void searchRefugee(const char* name) {
    ifstream file("refugees.dat", ios::binary);
    Refugee r;
    bool found = false;

    while (file.read(reinterpret_cast<char*>(&r), sizeof(r))) {
        if (strstr(r.name, name) != nullptr) {
            cout << r.refugeeID << " | "
                 << r.name << " | "
                 << r.age << " | "
                 << r.gender << " | "
                 << r.location << endl;
            found = true;
        }
    }

    if (!found)
        cout << "No refugee found with that name.\n";

    file.close();
}

// search by age range
void searchRefugee(int minAge, int maxAge) {
    ifstream file("refugees.dat", ios::binary);
    Refugee r;
    bool found = false;

    while (file.read(reinterpret_cast<char*>(&r), sizeof(r))) {
        if (r.age >= minAge && r.age <= maxAge) {
            cout << r.refugeeID << " | "
                 << r.name << " | "
                 << r.age << " | "
                 << r.gender << " | "
                 << r.location << endl;
            found = true;
        }
    }

    if (!found)
        cout << "No refugees found in this age range.\n";

    file.close();
}

//search by gender
void searchRefugee(char gender) {
    ifstream file("refugees.dat", ios::binary);
    Refugee r;
    bool found = false;

    gender = toupper(gender);

    while (file.read(reinterpret_cast<char*>(&r), sizeof(r))) {
        if (toupper(r.gender[0]) == gender) {
            cout << r.refugeeID << " | "
                 << r.name << " | "
                 << r.age << " | "
                 << r.gender << " | "
                 << r.location << endl;
            found = true;
        }
    }

    if (!found)
        cout << "No refugees found for this gender.\n";

    file.close();
}

//search menu

void refugeeSearchMenu() {
    int choice;
    do {
        cout << "\n=== Search Refugee ===\n";
        cout << "1. By ID\n";
        cout << "2. By Name\n";
        cout << "3. By Age Group\n";
        cout << "4. By Gender\n";
        cout << "5. Back\n";
        cout << "Enter choice: ";
        choice = getIntInput();

        if (choice == 1) {
            int id;
            cout << "Enter Refugee ID: ";
            id = getIntInput();
            searchRefugee(id);
        }
        else if (choice == 2) {
            char name[50];
            cout << "Enter Name: ";
            cin.getline(name, 50);
            formatName( name);
            searchRefugee(name);
        }
        else if (choice == 3) {
            int minAge, maxAge;
            cout << "Enter minimum age: ";
            minAge = getIntInput();
            cout << "Enter maximum age: ";
            maxAge = getIntInput();
            searchRefugee(minAge, maxAge);
        }
        else if (choice == 4) {
            char g;
            cout << "Enter Gender (M/F): ";
            cin >> g;
            cin.ignore();
            searchRefugee(g);
        }

        if (choice != 5)
            pauseSystem();

    } while (choice != 5);
}
// ------------------------------------------------------------------------------

// checks if the family id given exists
bool familyIDExists(int id) {
    ifstream file("family.dat", ios::binary);
    if (!file) return false;

    FamilyCase f;
    while (file.read(reinterpret_cast<char*>(&f), sizeof(FamilyCase))) {
        if (f.familyID == id) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

//refugee managment system

void manageRefugees() {
    int choice;
    Refugee r;
    do { // --- Refugee Management Menu ---
        cout << "\n=== Refugee Management ===\n";
        cout << "1. Add Refugee\n";
        cout << "2. View All Refugees\n";
        cout<< "3. Search refugee \n";
        cout << "4. Update Refugee\n";
        cout << "5. Delete Refugee by ID\n";
        cout << "6. Back to Main Menu\n";
        cout << "Enter your choice: ";
        choice = getIntInput();

        fstream file;

        switch(choice) {
            case 1:{ // ================= ADD REFUGEE =================
                fstream refugeeFile;
                fstream familyFile;
                char repeatChoice;   // controls repeated refugee entry

                 do {
                    refugeeFile.open("refugees.dat",   ios::binary | ios::out | ios::app);
                    if (!refugeeFile) {
                    cout << "Error opening file!\n";
                    break;
                }
                // Generate unique refugee ID
                r.refugeeID = getNextID("refugees.dat",1001, sizeof(Refugee));
                cout << "Generated Refugee ID: " << r.refugeeID << "\n";
                // Collect refugee personal information
                getValidatedString("Enter Name: ", r.name, 50);
                r.age = getValidatedAge();

                getGender(r.gender);
                if (!getRefugeeLocation(r.location)) { // Get refugee location using predefined options
                    refugeeFile.close();
                    break;   // return to refugee menu
                    }

                bool familyAssigned = false;// ensures family ID is set

                // Ask if refugee already has a registered family
                while (!familyAssigned) {
                                    char ch;
                cout << "Do you have a registered family in this camp? (Y/N): ";
                cin >> ch;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                ch = toupper(ch);

                if (ch == 'Y') {
                    int attempts = 0;
                    bool validFamily = false;

                    while (attempts < 3) {// Allow up to 3 attempts to enter valid family ID
                        cout << "Enter Family ID: ";
                        int fid = getIntInput();

                        if (familyIDExists(fid)) {
                            r.familyID = fid;
                            familyAssigned = true;
                            validFamily = true;
                            break;
                        } else {
                            attempts++;
                            cout << " Family ID does not exist. Attempts left: "
                                << (3 - attempts) << "\n";
                        }
                    }

                if (!validFamily) {
                    cout << "\nYou have used all attempts.\n";
                    cout << "Please enter N in order to be given a new Family ID.\n";
                    continue; // goes back to question if he has family ID
                }
            }
            else if (ch == 'N') { 
                FamilyCase f{}; // create new family case
                familyFile.open("family.dat", ios::binary | ios::out | ios::app);

                f.familyID = getNextID("family.dat", 2001, sizeof(FamilyCase));
                r.familyID = f.familyID;

                cout << "Generated Family ID: " << r.familyID << "\n";

                familyFile.write(reinterpret_cast<char*>(&f), sizeof(FamilyCase));
                familyFile.close();
                familyAssigned = true;
            }
            else {
                cout << "Invalid choice. Please enter Y or N.\n";
                
            }}
            // Get refugee status (Separated / Reunited)
            getStatus(r.status);
            // Write refugee record to file
            refugeeFile.write(reinterpret_cast<char*>(&r), sizeof(r));
            refugeeFile.close();
            // Add refugee ID to family member list
           if (!addMemberToFamily(r.familyID, r.refugeeID)) {
               cout << " Warning: Could not add refugee to family member list.\n";
           }


                cout << "Refugee added successfully!\n";
            // Ask user what to do next
            cout << "\nWhat do you want to do now?\n";
            cout << "1. Add another refugee\n";
            cout << "2. Back to Refugee Menu\n";
            cout << "3. Exit Program\n";
            cout << "Enter choice (1-3): ";
            
            int next;
            next = getIntInput();


            if (next == 1) {
                repeatChoice = 'Y';          // repeat this case 1 loop
            } else if (next == 2) {
                repeatChoice = 'N';          // stop this loop, return to refugee menu
            } else if (next == 3) {
                choice = 6;                  // force main refugee menu to exit
                repeatChoice = 'N';
            } else {
                cout << "Invalid option. Returning to Refugee Menu...\n";
                repeatChoice = 'N';
            }

        } while (repeatChoice == 'Y');       // repeat adding refugees only if user chose 1
            break;
            }

        case 2: {
            // ================= VIEW ALL REFUGEES =================
            char viewAgain = 'Y';
            do {// Open refugees file for reading
                file.open("refugees.dat", ios::in | ios::binary);
                if (!file) {
                    cout << "No records found.\n";
                    break;
                }
                // Count total number of refugee records
                file.seekg(0, ios::end);
                int numRecords = file.tellg() / sizeof(Refugee);
                file.seekg(0, ios::beg);

                if (numRecords == 0) {  // If file exists but has no data
                    cout << "No refugees registered yet.\n";
                    file.close();
                    pauseSystem();
                    break;
                }

                // Load all refugees into dynamic array
                Refugee* refugees = new Refugee[numRecords];
                int count = 0;
                while (file.read(reinterpret_cast<char*>(&refugees[count]), sizeof(Refugee))) {
                    count++;
                }
                file.close();

                // Ask user how they want to view the data
                cout << "\nHow do you want to view the refugees?\n";
                cout << "1. Original order (as registered)\n";
                cout << "2. Sorted by Name (A to Z)\n";
                cout << "3. Sorted by Age (Youngest to Oldest)\n";
                cout << "Enter choice (1-3): ";
                int viewChoice = getIntInput();

                if (viewChoice == 2) { // Sort refugees by name
                    for (int i = 0; i < count - 1; i++) {
                        for (int j = i + 1; j < count; j++) {
                            if (strcmp(refugees[i].name, refugees[j].name) > 0) {
                                Refugee temp = refugees[i];
                                refugees[i] = refugees[j];
                                refugees[j] = temp;
                            }
                        }
                    }
                    cout << "\nRefugees sorted by Name (A to Z):\n";
                }
        else if (viewChoice == 3) {// Sort refugees by age
            for (int i = 0; i < count - 1; i++) {
                for (int j = i + 1; j < count; j++) {
                    if (refugees[i].age > refugees[j].age) {
                        Refugee temp = refugees[i];
                        refugees[i] = refugees[j];
                        refugees[j] = temp;
                    }
                }
            }

            cout << "\nRefugees sorted by Age (Youngest first):\n";
        }
        else if (viewChoice == 1) {   // Original registration order
            cout << "\nAll Refugees (in registration order):\n";
        }
        else {  // Invalid sort option
            cout << "\nInvalid choice! Showing refugees in registration order.\n";
        }

        // Display table header
        cout << left
             << setw(10) << "ID"
             << setw(20) << "Name"
             << setw(8)  << "Age"
             << setw(10) << "Gender"
             << setw(20) << "Location"
             << setw(12) << "FamilyID"
             << setw(15) << "Status"
             << endl;
        cout << string(95, '-') << endl;

        // Display all refugees
        for (int i = 0; i < count; i++) {
            cout << left
                 << setw(10) << refugees[i].refugeeID
                 << setw(20) << refugees[i].name
                 << setw(8)  << refugees[i].age
                 << setw(10) << refugees[i].gender
                 << setw(20) << refugees[i].location
                 << setw(12) << refugees[i].familyID
                 << setw(15) << refugees[i].status
                 << endl;
        }

        cout << "\nTotal Refugees: " << count << endl;

        // Free dynamically allocated memory
        delete[] refugees;

        // Ask what to do next ===
        cout << "\nWhat do you want to do now?\n";
        cout << "1. View refugees again (with different sorting)\n";
        cout << "2. Back to Refugee Management Menu\n";
        cout << "3. Exit Program\n";
        cout << "Enter choice (1-3): ";
        int nextChoice = getIntInput();

        if (nextChoice == 1) {
            viewAgain = 'Y';  // Repeat viewing
        }
        else if (nextChoice == 2) {
            viewAgain = 'N';  // Return to main refugee menu
        }
        else if (nextChoice == 3) {
            cout << "Exiting program...\n";
            exit(0);     // Terminate program
        }
        else {
            cout << "Invalid option. Returning to Refugee Management Menu...\n";
            viewAgain = 'N';
        }
        
        // Pause before repeating view
        if (viewAgain == 'Y') {
            pauseSystem();  
        }

    } while (viewAgain == 'Y');

    break;
}

    // ================= SEARCH REFUGEE MENU ===============
        case 3:{
                 refugeeSearchMenu();
                 break;
                        }
                // ================= UPDATE REFUGEE DETAILS =================
        case 4: {//// update section
        char updateMore = 'Y';
        fstream file;
        Refugee r;
        int id;
        char choiceField;

        while (updateMore == 'Y') {  
            // Open file for updating
            file.open("refugees.dat", ios::in | ios::out | ios::binary);
            if (!file) {
                cout << "File not found!\n";
                break;
            }
            bool found = false;
            int attempts = 0;

            // Allow user 3 attempts to enter valid refugee ID
            while (attempts < 3 && !found) {
                cout << "\nEnter Refugee ID to update: ";
                id = getIntInput();

                file.clear();
                file.seekg(0, ios::beg); 
                // Search for refugee ID
                while (file.read(reinterpret_cast<char*>(&r), sizeof(Refugee))) {
                    if (r.refugeeID == id) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    attempts++;
                    if (attempts < 3) {
                        cout << "Refugee ID not found. You have " << (3 - attempts) << " chance(s) left.\n";
                    
                    } else {
                        cout << "You have used all 3 chances. Returning to main menu.\n";
                    }
                }
            }

            if (!found) {
                file.close();
                break; // back to main menu
            }

            cout << "\nRefugee found: " << r.name << endl;

        // Loop to update selected fields
            do {
                cout << "\nWhat do you want to update?\n";
                cout << "N-Name  A-Age  G-Gender  L-Location  F-FamilyID  S-Status\n";
                cin >> choiceField;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                choiceField = toupper(choiceField);

                switch (choiceField) {
                    case 'N':
                        getValidatedString("Enter new Name: ", r.name, 50);
                        break;
                    case 'A':
                        r.age = getValidatedAge();
                        break;
                    case 'G':
                        getGender(r.gender);
                        break;
                    case 'L':
                    if (!getRefugeeLocation(r.location)) {
                        cout << "Location update failed.\n";
                            }
                        break;
                        
                    case 'F':
                        cout << "Enter new Family ID: ";
                        r.familyID = getIntInput();
                        break;
                    case 'S':
                        getStatus(r.status);
                        break;
                    default:
                        cout << "Invalid option!\n";
                }

                cout << "\nUpdate another field? (Y/N): ";
                cin >> choiceField;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                choiceField = toupper(choiceField);

            } while (choiceField == 'Y');

            // --- Write updated record back to file ---
            file.seekp(-static_cast<long>(sizeof(r)), ios::cur);
            file.write(reinterpret_cast<char*>(&r), sizeof(Refugee));
            file.close();

                cout << "\nRefugee updated successfully!\n";

                cout << "\nUpdate another refugee? (Y/N): ";
                cin >> updateMore;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                updateMore = toupper(updateMore);
        }

        break;
    }
  // ================= DELETE REFUGEE =================
        case 5: { 
            char deleteMore = 'Y';
            while (deleteMore == 'Y') {
                int attempts = 0;
                int id;
                bool found = false;
                while (attempts < 3 && !found) { // Allow up to 3 attempts to find refugee

                    cout << "Enter Refugee ID to delete: ";
                    id = getIntInput();
                    file.open("refugees.dat", ios::in | ios::binary);
                    if (!file) {
                        cout << "No records found.\n";
                        break;
                    }
                    Refugee tempR;
                    while (file.read(reinterpret_cast<char*>(&tempR), sizeof(Refugee))) {
                        if (tempR.refugeeID == id) {
                            found = true;
                            break;
                        }
                    }
                    file.close();
                    if (!found) {
                        attempts++;
                        if (attempts < 3) {
                            cout << "Refugee ID not found. You have " << (3 - attempts) << " chance(s) left.\n";
                        } else {
                            cout << "You have used all 3 chances. Returning to menu.\n";
                        }
                    }
                }
                if (!found) {
                    break; // exit delete loop if max attempts reached without finding
                }

                // Copy all records except deleted one
                ifstream inFile("refugees.dat", ios::in | ios::binary);
                ofstream outFile("temp.dat", ios::out | ios::binary);
                Refugee r;
                while (inFile.read(reinterpret_cast<char*>(&r), sizeof(Refugee))) {
                    if (r.refugeeID != id) {
                        outFile.write(reinterpret_cast<char*>(&r), sizeof(Refugee));
                    }
                }
                inFile.close();
                outFile.close();
                remove("refugees.dat");
                rename("temp.dat", "refugees.dat");
                cout << "Refugee deleted successfully!\n";
                cout << "\nDelete another refugee? (Y/N): ";
                cin >> deleteMore;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                deleteMore = toupper(deleteMore);
            }
            break;}
                    case 6:{
                        cout << "Returning to main menu...\n";
                        break;
                    }
                    default:{
                        cout << "Invalid choice!\n";
                        break;}
                }
                if(choice != 6) pauseSystem();

            } while(choice != 6);
        }



//=========Family management system=========
void manageFamilyReunification() {
    int choice;
    FamilyCase f; // Structure to hold one family record

    do { // --- Family Reunification Menu ---
        cout << "\n=== Family Reunification ===\n";
        cout << "1. Manage Family Case\n";
        cout << "2. View All Family Cases\n";
        cout << "3. Search Family Case by Family ID\n";
        cout << "4. Mark Family as Reunited\n";
        cout << "5. Delete Family case by ID\n";
        cout << "6. Back to Main Menu\n";
        cout << "Enter your choice: ";
        choice = getIntInput();

        fstream file;
        switch(choice) {
            case 1: { // Manage EXISTING family case (update last contact date only)
                int fid;
                cout << "Enter Family ID to manage: ";
                fid = getIntInput();

                // Open file and check if family exists
                fstream file("family.dat", ios::in | ios::out | ios::binary);
                if (!file) {
                    cout << "Error opening family file!\n";
                    break;
                }

                bool found = false;
            // Search for the family by ID
                while (file.read(reinterpret_cast<char*>(&f), sizeof(FamilyCase))) {
                    if (f.familyID == fid) {
                        found = true;
                        break;
                    }
                }
                // If family ID does not exist
                if (!found) {
                    cout << " Family ID not found!\n";
                    file.close();
                    break;
                }

               // --- Update last contact date (with 3-attempt validation) ---
                int attempts = 0;
                bool validDate = false;

                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                while (attempts < 3) {
                    cout << "Enter Last Contact Date (YYYY-MM-DD): ";
                    cin.getline(f.lastContact, 20);

                    if (isValidDate(f.lastContact)) {
                        validDate = true;
                        break;
                    } else {
                        attempts++;
                        cout << " Invalid date format or value.\n";
                        if (attempts < 3)
                            cout << "Attempts left: " << (3 - attempts) << "\n";
                    }
                }

                if (!validDate) {    // If user failed all attempts
                    cout << " Failed 3 attempts. Returning to menu.\n";
                    file.close();
                    break;
                }

                // Move file pointer back and overwrite same record
                file.seekp(-static_cast<long>(sizeof(FamilyCase)), ios::cur);
                file.write(reinterpret_cast<char*>(&f), sizeof(FamilyCase));
                file.close();

                cout << " Family case updated successfully!\n";
                break;
            }

            case 2: {  // View ALL family cases
                file.open("family.dat", ios::in | ios::binary);
                if (!file) {
                    cout << "No family records found.\n";
                    break;
                }
               // Table header
                cout << left
                    << setw(10) << "FamilyID"
                    << setw(15) << "LastContact"
                    << setw(10) << "Reunited"
                    << "Member IDs"
                    << endl;

                cout << string(70, '-') << endl;
                 // Read and display each family case
                while (file.read(reinterpret_cast<char*>(&f), sizeof(FamilyCase))) {

                    // Fixed columns FIRST
                    cout << left
                        << setw(10) << f.familyID
                        << setw(15) << f.lastContact
                        << setw(10) << (f.reunited ? "Yes" : "No");

                    // Member IDs 
                    bool first = true;
                    bool hasMembers = false;

                    for (int i = 0; i < 10; i++) {
                        if (f.memberIDs[i] != 0) {
                            if (!first) cout << ", ";
                            cout << f.memberIDs[i];
                            first = false;
                            hasMembers = true;
                        }
                    }

                    if (!hasMembers) {
                        cout << "—";
                    }

                    cout << endl;
                    cout << string(70, '-') << endl;
                }

                file.close();
                break;
            }


            case 3:  // Search for a family case by Family ID
                int id;
                cout << "Enter Family ID to search: ";
                id = getIntInput();
                file.open("family.dat", ios::in | ios::binary);
                bool found; found = false;
                while(file.read(reinterpret_cast<char*>(&f), sizeof(f))) {
                    if(f.familyID == id) {
                        cout << "Family Case Found:\n";
                        cout << "Family ID: " << f.familyID << "\nMembers: ";
                        for(int i=0;i<10;i++)
                            if(f.memberIDs[i] != 0)
                                cout << f.memberIDs[i] << " ";
                        cout << "\nLast Contact: " << f.lastContact
                             << "\nReunited: " << (f.reunited ? "Yes" : "No") << "\n";
                        found = true;
                        break;
                    }
                }
                if(!found) cout << "Family case not found!\n";
                file.close();
                break;

           case 4: { // Marka  family as Reunited
                cout << "Enter Family ID to mark reunited: ";
                int id = getIntInput();
                bool found = false;

                fstream file("family.dat", ios::in | ios::out | ios::binary);
                if (!file) {
                    cout << "Error opening family file!\n";
                    break;
                }

                while (file.read(reinterpret_cast<char*>(&f), sizeof(f))) {
                    if (f.familyID == id) {
                        found = true;

                        if (f.reunited) {
                            cout << " Family ID " << id << " is already marked as reunited.\n";
                        } else {
                            f.reunited = true;
                            // Overwrite same record
                            file.seekp(-static_cast<int>(sizeof(f)), ios::cur); // move back to overwrite
                            file.write(reinterpret_cast<char*>(&f), sizeof(f));
                            cout << " Family ID " << id << " has been marked as reunited!\n";
                        }

                        break; // stop looping once found
                    }
                }

                if (!found) {
                    cout << " Family case not found!\n";
                }

                file.close();
                break;
            }
            case 5: {   // Delete family case by ID (with 3 attempts)
                char deleteMore = 'Y';
                while (deleteMore == 'Y') {
                    int attempts = 0;
                    int id;
                    bool found = false;
                    while (attempts < 3 && !found) {
                        cout << "Enter Family ID to delete: ";
                        id = getIntInput();
                        file.open("family.dat", ios::in | ios::binary);
                        if (!file) {
                            cout << "No records found.\n";
                            break;
                        }
                        FamilyCase tempF;
                        while (file.read(reinterpret_cast<char*>(&tempF), sizeof(FamilyCase))) {
                            if (tempF.familyID == id) {
                                found = true;
                                break;
                            }
                        }
                        file.close();
                        if (!found) {
                            attempts++;
                            if (attempts < 3) {
                                cout << "Family ID not found. You have " << (3 - attempts) << " chance(s) left.\n";
                            } else {
                                cout << "You have used all 3 chances. Returning to menu.\n";
                            }
                        }
                    }
                    if (!found) {
                        break;
                    }
                    // Rewrite file excluding deleted record
                    ifstream inFile("family.dat", ios::in | ios::binary);
                    ofstream outFile("temp.dat", ios::out | ios::binary);
                    FamilyCase f;
                    while (inFile.read(reinterpret_cast<char*>(&f), sizeof(FamilyCase))) {
                        if (f.familyID != id) {
                            outFile.write(reinterpret_cast<char*>(&f), sizeof(FamilyCase));
                        }
                    }
                    inFile.close();
                    outFile.close();
                    remove("family.dat");
                    rename("temp.dat", "family.dat");
                    cout << "Family case deleted successfully!\n";
                    cout << "\nDelete another family case? (Y/N): ";
                    cin >> deleteMore;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    deleteMore = toupper(deleteMore);
                }
                break;
            }

            case 6:
                cout << "Returning to main menu...\n";
                break;

            default:
                cout << "Invalid choice!\n";
        }
        if(choice != 6) pauseSystem();

    } while(choice != 6);
}

//=========Aid management system=========
void manageAid() {
    int choice;
    Aid a;  // Aid structure for reading/writing records

    do { // ===== Aid Management Menu =====
        cout << "\n=== Humanitarian Aid Management ===\n";
        cout << "1. Record Aid Distribution\n";
        cout << "2. View All Aid Records\n";
        cout << "3. Search Aid Record by Aid ID\n";
        cout << "4. Delete Aid Record by Aid ID\n";
        cout << "5. Back to Main Menu\n";
        cout << "Enter your choice: ";
        choice = getIntInput();

        fstream file;
        switch(choice) {
            
            case 1: { // -------- Record New Aid Distribution --------

                // Open file in append mode to add new aid record
                ofstream aidFile("aid.dat", ios::binary | ios::app);
                if (!aidFile) {
                    cout << "Error opening aid file.\n";
                    continue;
                
                }
                Aid a{}; 
                
                        // Generate unique Aid ID
                a.aidID = getNextAidID("aid.dat", 3001); 

                cout << "Generated Aid ID: " << a.aidID << "\n";

                // -------- Aid Type Selection (3 attempts) --------
                char aidChoice;
                int attempts = 0;
                bool validAid = false;

            while (attempts < 3) {
                cout << "Enter Aid Type (F = Food / W = Water / M = Medicine): ";
                cin >> aidChoice;
                cin.ignore(1000, '\n');
                aidChoice = toupper(aidChoice);

                if (aidChoice == 'F') {
                    strcpy(a.type, "Food");

                    cout << "Enter food description(Rice only, Flour only, Rice and Flour..)";
                    cin.getline(a.description, 50);

                    if (strlen(a.description) == 0) {
                        cout << "Description cannot be empty.\n";
                        attempts++;
                        continue;
                    }

                    validAid = true;
                    break;
                }
                else if (aidChoice == 'W') {
                    strcpy(a.type, "Water");
                    strcpy(a.description, "water");
                    validAid = true;
                    break;
                }
                else if (aidChoice == 'M') {
                    strcpy(a.type, "Medicine");
                    strcpy(a.description, "Medicine");
                    validAid = true;
                    break;
                }
                else {
                    attempts++;
                    cout << "Invalid aid type. Attempts left: "
                        << (3 - attempts) << "\n";
                }
            }

            if (!validAid) { // Abort if aid type was never valid
                cout << "All attempts used. Returning to menu.\n";
                aidFile.close();
                break;
            }

                    // -------- Quantity Validation (must be > 0) --------
            attempts = 0;  

        while (attempts < 3) {
            cout << "Enter Quantity (Food in KG / Water in Liters / Medicine in Boxes): ";
            a.quantity = getIntInput();

            if (a.quantity <= 0) {  
                attempts++;
                cout << "Invalid quantity. Must be greater than zero. Attempts left: "
                    << (3 - attempts) << "\n";
                continue;  // go for the next attempt
            }

            break;  // valid input, exit the loop
        }

        if (attempts == 3) {
            cout << "Failed to enter a valid quantity after 3 attempts. Returning to menu.\n";
            aidFile.close();
            break;  // go back to previous menu
        }
                    // -------- Distribution Location Selection --------
            int locChoice;
            int attemptsLocation = 0;
        bool validLocation = false;

        while (attemptsLocation < 3) {
            cout << "Select location:\n";
            cout << "1. Tent Zone\n";
            cout << "2. Shelter Block\n";
            cout << "3. Housing Unit\n";
            cout << "4. Family Shelter\n";
            cout << "5. Temporary Shelter\n";
            cout << "Enter your choice (1-5): ";

            string input;
            cin >> input;
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear buffer

            // Check if input is a number
            bool isNumber = true;
            for (size_t i = 0; i < input.length(); i++) {
                if (!isdigit(input[i])) {
                    isNumber = false;
                    break;
                }
            }

            if (!isNumber) {
                attemptsLocation++;
                cout << "Invalid input. Attempts left: " << (3 - attemptsLocation) << "\n";
                continue; // count as a wrong attempt
            }

            // Convert to integer
            locChoice = stoi(input);

    // Check if number is valid
    switch (locChoice) {
        case 1:
            strcpy(a.distributionLocation, "Tent Zone");
            validLocation = true;
            break;
        case 2:
            strcpy(a.distributionLocation, "Shelter Block");
            validLocation = true;
            break;
        case 3:
            strcpy(a.distributionLocation, "Housing Unit");
            validLocation = true;
            break;
        case 4:
            strcpy(a.distributionLocation, "Family Shelter");
            validLocation = true;
            break;
        case 5:
                strcpy(a.distributionLocation, "Temporary Shelter");
                validLocation = true;
                break;
                default:
                attemptsLocation++;
                cout << "Invalid choice. Attempts left: " << (3 - attemptsLocation) << "\n";
                }

                    if (validLocation)
                        break;
                }

                if (!validLocation) {
                    cout << "Location selection failed. Returning to menu.\n";
                    aidFile.close();
                    break; // exit back to previous menu
                }

                // ---------- Number of Recipients ----------
                while (true) {
                    cout << "How many recipients do you want to enter? ";
                    a.numRecipients = getIntInput();

                    if (a.numRecipients <= 0)
                        cout << "Number must be greater than zero.\n";
                    else
                        break;
                }

                // Allocate dynamic array for recipient IDs
                a.recipientIDs = new int[a.numRecipients];

                // -------- Validate Recipient IDs --------
                int added = 0;
                while (added < a.numRecipients) {
                    cout << "Enter Refugee ID (" << added + 1
                        << "/" << a.numRecipients << "): ";
                    int rid = getIntInput();

                    // Verify refugee exists
                    ifstream refFile("refugees.dat", ios::binary);
                    if (!refFile) {
                        cout << "Error opening refugees file! Cannot verify IDs.\n";
                        continue;
                    }

                    bool exists = false;
                    Refugee r;
                    while (refFile.read(reinterpret_cast<char*>(&r), sizeof(Refugee))) {
                        if (r.refugeeID == rid) {
                            exists = true;
                            break;
                        }
                    }
                    refFile.close();

                    if (!exists) {
                        cout << " Refugee ID does not exist.\n";
                        continue;
                    }
                    // Prevent duplicate IDs
                    bool duplicate = false;
                    for (int i = 0; i < added; i++) {
                        if (a.recipientIDs[i] == rid) {
                            duplicate = true;
                            break;
                        }
                    }

                    if (duplicate) {
                        cout << " Duplicate ID. Try again.\n";
                        continue;
                    }

                    a.recipientIDs[added++] = rid;
                }

            // -------- Write Aid Record to File --------
                aidFile.write(reinterpret_cast<char*>(&a.aidID), sizeof(int));
                aidFile.write(reinterpret_cast<char*>(a.type), sizeof(a.type));
                aidFile.write(reinterpret_cast<char*>(&a.quantity), sizeof(int));
                aidFile.write(reinterpret_cast<char*>(a.distributionLocation), sizeof(a.distributionLocation));
                aidFile.write(reinterpret_cast<char*>(a.description), sizeof(a.description));
                aidFile.write(reinterpret_cast<char*>(&a.numRecipients), sizeof(int));
                aidFile.write(reinterpret_cast<char*>(a.recipientIDs), sizeof(int) * a.numRecipients);

                aidFile.close();
                delete[] a.recipientIDs;  // prevent memory leak because it is dynamic

                cout << "Aid distribution recorded successfully!\n";
                break;
            }

            break;

        case 2: // View ALL aid records (sequential read)
            file.open("aid.dat", ios::in | ios::binary);
            if(!file) {
                cout << "No aid records found.\n";
                break;
            }

            cout << left
            << setw(6) << "AidID"
            << setw(12) << "Type"
            << setw(10) << "Quantity"
            << setw(18) << "Location"
            << "Recipients\n";
                        
        while (!file.eof()) {
            file.read(reinterpret_cast<char*>(&a.aidID), sizeof(int));
            if (file.eof()) break;
            file.read(reinterpret_cast<char*>(a.type), sizeof(a.type));
            file.read(reinterpret_cast<char*>(&a.quantity), sizeof(int));
            file.read(reinterpret_cast<char*>(a.distributionLocation), sizeof(a.distributionLocation));
            file.read(reinterpret_cast<char*>(a.description), sizeof(a.description));
            file.read(reinterpret_cast<char*>(&a.numRecipients), sizeof(int));

            a.recipientIDs = new int[a.numRecipients];
            file.read(reinterpret_cast<char*>(a.recipientIDs), sizeof(int) * a.numRecipients);

            // Print formatted
            cout << left
            << setw(6) << a.aidID
            << setw(12) << a.type
            << setw(10) << a.quantity
            << setw(18) << a.distributionLocation;

        for (int i = 0; i < a.numRecipients; i++) {
            cout << a.recipientIDs[i] << " ";
        }
        cout << "\n";

            delete[] a.recipientIDs;
        }

                        file.close();
                        break;

        case 3: // Search Aid Record
            int id;
            cout << "Enter Aid ID to search: ";
            id = getIntInput();
            file.open("aid.dat", ios::in | ios::binary);
            bool found; found = false;
        
        while (!file.eof()) {
            file.read(reinterpret_cast<char*>(&a.aidID), sizeof(int));
            if (file.eof()) break;
            file.read(reinterpret_cast<char*>(a.type), sizeof(a.type));
            file.read(reinterpret_cast<char*>(&a.quantity), sizeof(int));
            file.read(reinterpret_cast<char*>(a.distributionLocation), sizeof(a.distributionLocation));
            file.read(reinterpret_cast<char*>(a.description), sizeof(a.description));
            file.read(reinterpret_cast<char*>(&a.numRecipients), sizeof(int));

            a.recipientIDs = new int[a.numRecipients];
            file.read(reinterpret_cast<char*>(a.recipientIDs), sizeof(int) * a.numRecipients);

            if (a.aidID == id) {  // only print if it matches
                cout << "\nAid Record Found:\n";
                cout << left
                    << setw(6) << "AidID"
                    << setw(12) << "Type"
                    << setw(10) << "Quantity"
                    << setw(18) << "Location"
                    << "Recipients\n";

                cout << left
                    << setw(6) << a.aidID
                    << setw(12) << a.type
                    << setw(10) << a.quantity
                    << setw(18) << a.distributionLocation;

                for (int i = 0; i < a.numRecipients; i++) {
                    cout << a.recipientIDs[i] << " ";
                }
                cout << "\n";

                found = true;
            }

            delete[] a.recipientIDs;
        }

        if (!found) cout << "Aid record not found!\n";
        break;

        case 4: { // Delete Aid Record by ID
            char deleteMore = 'Y';
            while (deleteMore == 'Y') {
                int attempts = 0;
                int id;
                bool found = false;
                while (attempts < 3 && !found) {
                    cout << "Enter Aid ID to delete: ";
                    id = getIntInput();
                    file.open("aid.dat", ios::in | ios::binary);
                    if (!file) {
                        cout << "No records found.\n";
                        break;
                    }
                    while (true) {
                        int aidID;
                        file.read(reinterpret_cast<char*>(&aidID), sizeof(int));
                        if (file.eof()) break;
                        char type[20];
                        file.read(type, 20);
                        int quantity;
                        file.read(reinterpret_cast<char*>(&quantity), sizeof(int));
                        char loc[50];
                        file.read(loc, 50);
                        char desc[50];
                        file.read(desc, 50);
                        int numRec;
                        file.read(reinterpret_cast<char*>(&numRec), sizeof(int));
                        int* recs = new int[numRec];
                        file.read(reinterpret_cast<char*>(recs), sizeof(int) * numRec);
                        if (aidID == id) {
                            found = true;
                            delete[] recs;
                            break;
                        }
                        delete[] recs;
                    }
                    file.close();
                    if (!found) {
                        attempts++;
                        if (attempts < 3) {
                            cout << "Aid ID not found. You have " << (3 - attempts) << " chance(s) left.\n";
                        } else {
                            cout << "You have used all 3 chances. Returning to menu.\n";
                        }
                    }
                }
                if (!found) {
                    break;
                }
                // Perform deletion
                ifstream inFile("aid.dat", ios::in | ios::binary);
                ofstream outFile("temp.dat", ios::out | ios::binary);
                while (true) {
                    int aidID;
                    inFile.read(reinterpret_cast<char*>(&aidID), sizeof(int));
                    if (inFile.eof()) break;
                    char type[20];
                    inFile.read(type, 20);
                    int quantity;
                    inFile.read(reinterpret_cast<char*>(&quantity), sizeof(int));
                    char loc[50];
                    inFile.read(loc, 50);
                    char desc[50];
                    inFile.read(desc, 50);
                    int numRec;
                    inFile.read(reinterpret_cast<char*>(&numRec), sizeof(int));
                    int* recs = new int[numRec];
                    inFile.read(reinterpret_cast<char*>(recs), sizeof(int) * numRec);
                    if (aidID != id) {
                        outFile.write(reinterpret_cast<char*>(&aidID), sizeof(int));
                        outFile.write(type, 20);
                        outFile.write(reinterpret_cast<char*>(&quantity), sizeof(int));
                        outFile.write(loc, 50);
                        outFile.write(desc, 50);
                        outFile.write(reinterpret_cast<char*>(&numRec), sizeof(int));
                        outFile.write(reinterpret_cast<char*>(recs), sizeof(int) * numRec);
                    }
                    delete[] recs;
                }
                inFile.close();
                outFile.close();
                remove("aid.dat");
                rename("temp.dat", "aid.dat");
                cout << "Aid record deleted successfully!\n";
                cout << "\nDelete another aid record? (Y/N): ";
                cin >> deleteMore;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                deleteMore = toupper(deleteMore);
            }
            break;
            }
            case 5:
                cout << "Returning to main menu...\n";
                break;

            default:
                cout << "Invalid choice!\n";
        }
        if(choice != 5) pauseSystem();

    } while(choice != 5);
}




//======Incident management system=========
void manageIncidents() {
    int choice;
    Incident inc;

    do {// Incident menu
        cout << "\n=== Incident Reporting ===\n";
        cout << "1. Add Incident Report\n";
        cout << "2. View All Incidents\n";
        cout << "3. Search Incident by ID\n";
        cout << "4. Mark Incident as Resolved\n";
        cout << "5. Delete Incident by ID\n";
        cout << "6. Back to Main Menu\n";
        cout << "Enter your choice: ";
        choice = getIntInput();

        fstream file;
        switch(choice) {

            case 1: 
                {// Add Incident
             char addMore = 'Y';
             while(addMore == 'Y') {
            
            // --- Incident Type Entry & Validation ---
            char incidentType;
            bool validType = false;
            int attempts = 0;

                inc.incidentID = getNextID("incidents.dat", 4001, sizeof(Incident));
                cout << "Generated Incident ID: " << inc.incidentID << "\n";

                while(attempts< 3 && !validType){
               string typeInput;
               cout << "Enter Type (G = GBV / T = Theft / M = Murder / O = Other): ";
               getline(cin, typeInput);        // read entire line
               if (typeInput.length() != 1) {
               attempts++;
               cout << "Invalid input. Enter only G, T, or M. Attempts left: " << (3 - attempts) << "\n";
               continue; // retry
}
               incidentType = toupper(typeInput[0]);

                if(incidentType == 'G')
                {  strcpy(inc.type, "GBV");
                    validType= true;
                    }
                else if(incidentType == 'T')
                {  strcpy(inc.type, "Theft");
                    validType= true;
                }
                else if(incidentType == 'M')
                {  strcpy(inc.type, "Murder");
                    validType= true;
                }
                  else if(incidentType == 'O')
                {  strcpy(inc.type, "Other");
                    validType= true;
                }
                else {
                    attempts++;
                    if(attempts < 3)
                    cout<<"You entered wrong incident type. Try again("<<3-attempts<<"attempts left).\n";
                }

            }
                if (!validType){
                    cout <<"Failed after 3 attempts. \n";
                    file.close();
                    break;// exit if invalid type after 3 attempts
                }

      bool validVictim = false;
int victimAttempts = 0;
while (victimAttempts < 3 && !validVictim) {
    cout << "Enter Victim ID: ";
    int vid = getIntInput();

    // Check if victim ID exists in refugees.dat
    ifstream refFile("refugees.dat", ios::binary);
    if (!refFile) {
        cout << "Error opening refugees file!\n";
        break; // exit if refugees file cannot be opened
    }

    Refugee r;
    bool found = false;
    while (refFile.read(reinterpret_cast<char*>(&r), sizeof(Refugee))) {
        if (r.refugeeID == vid) {
            found = true;
            break;
        }
    }
    refFile.close();

    if (found) {
        inc.victimID = vid; // assign valid victim ID
        validVictim = true;
    } else {
        victimAttempts++;
        if (victimAttempts < 3) {
            cout << "Victim ID not found. Please register the refugee first. Attempts left: "
                 << (3 - victimAttempts) << "\n";
        } else {
            cout << "Failed to enter valid Victim ID after 3 attempts. Returning to menu.\n";
            break; // exit victim ID entry after 3 failed attempts
        }
    }
            }

            if (!validVictim) {
                break; // exit the current incident entry completely if no valid victim
            }
                int locationChoice;
                bool validLocation = false;
                attempts = 0;
              // --- Incident location selection ---
                cout << "Select incident location :\n";
                cout << "1. Shelter(residence) Area\n";
                cout << "2. Water Point\n";
                cout << "3. Food Distribution Center\n";
                cout << "4. School / Learning Center\n";
                cout << "5. Latrine area\n";
                cout << "6. Camp Entrance\n";
                cout << "7. Out side the camp"<<endl;
                 while (attempts < 3 && !validLocation) {
                cout << "Enter choice by their number(1-7): ";
                locationChoice = getIntInput();

                if (locationChoice == 1)
                     strcpy(inc.location, "Shelter(residence) Area");
                else if (locationChoice == 2)
                     strcpy(inc.location, "Water Point");
                else if (locationChoice == 3)
                     strcpy(inc.location, "Food Distribution Center");
                else if (locationChoice == 4)
                     strcpy(inc.location, "School / Learning Center");
                else if (locationChoice == 5)
                     strcpy(inc.location, "Latrine area");
                else if (locationChoice == 6)
                     strcpy(inc.location, "Camp Entrance");
                else if (locationChoice == 7)
                     strcpy(inc.location, "Out side the camp");
                else {
                    attempts++;
                    if (attempts< 3)
                cout <<"You entered wrong location choice. please Try again("<<3-attempts<<"attempts left).\n";
               continue;//retry
                }
                validLocation = true;
                }
            if (!validLocation) {
                 cout << "Failed after 3 attempts.\n";
                 file.close();
                 break; // exit if invalid location
            }
             // --- Severity selection ---
                int severityLevel;
                bool validSeverity = false;
                attempts = 0;

                cout << "Enter Severity Level \n";
                cout<<"1.Low \n";
                cout<<"2.Medium \n";
                cout<<"3.High \n";
                 while (attempts < 3 && !validSeverity) {
                    cout<<"Enter choice by their number: ";
                severityLevel = getIntInput();

                if(severityLevel == 1)
                {  strcpy(inc.severity, "Low");
                    validSeverity = true;
                    }
                else if( severityLevel== 2)
                {  strcpy(inc.severity, "Medium");
                    validSeverity= true;
                }
                else if(severityLevel == 3)
                {  strcpy(inc.severity, "High");
                     validSeverity= true;
                }
                else {
                    attempts++;
                    if (attempts < 3)
                       cout<<"You entered wrong severity choice. Try again("<<3-attempts<<"attempts left).\n";
             
                }
            }
                 if (!validSeverity) { // exit if severity not valid
                   cout << "Failed after 3 attempts.\n";
                   file.close();
                   break;
               }
                  // ---- Date input & validation ----
                while (true) {
                    cout << "Enter Date (YYYY-MM-DD): ";
                    cin.getline(inc.date, 20);

                    if (isValidDate(inc.date))
                        break;
                    else
                        cout << " Invalid date format. Try again.\n";
                }
                // ---- Time input & validation ----
                while (true) {
                    cout << "Enter Time (HH:MM): ";
                    cin.getline(inc.time, 10);

                    if (isValidTime(inc.time))
                        break;
                    else
                        cout << " Invalid time format. Try again.\n";
                }
                  // --- Status selection ---
            char statusChoice;
            bool validStatus = false;
            attempts = 0;

             while (attempts < 3 && !validStatus) {
                cout<<"Is this incident resolved or unresolved yet? ( R= Resolved / U = Unresolved) ";
                cin>>statusChoice;
                statusChoice = toupper(statusChoice);

                if(statusChoice == 'R')
                {  strcpy(inc.status, "Resolved");
                    validStatus = true;
                    }
                else if(statusChoice == 'U')
                {  strcpy(inc.status, "Unresolved");
                    validStatus = true;
                }
                else {
                     attempts++;
                    if (attempts < 3)
                    cout<<"You entered wrong status chocie . Try again("<<3-attempts<<"attempts left).\n";
                       cin.clear();              //  RESET the input stream
                       cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // flush buffer
                        }
            }
              if (!validStatus) {
              cout << "Failed after 3 attempts.\n";
              file.close();
              break; // exit if status invalid
            }
            
               ofstream outFile("incidents.dat", ios::app | ios::binary);
               outFile.write(reinterpret_cast<char*>(&inc), sizeof(inc));
               outFile.close();

                cout << "Incident added successfully!\n";
                while (true) {      // --- Ask user if they want to add another incident ---
           
                cout << "Do you want to add another incident? (Y/N): ";
                cin >> addMore;
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // flush leftover input
                addMore = toupper(addMore);

                if (addMore == 'Y' || addMore == 'N') {
                    break; // valid input, exit loop
                } else {
                    cout << " Invalid input. Please enter Y for Yes or N for No.\n";
                }
            }          }
                            break;
                        }
                case 2: // View All Incidents
                    {
                    file.open("incidents.dat", ios::in | ios::binary);
                    if (!file) {
                        cout << "No incident records found.\n";
                        break;
                    }

                    cout << left   // --- Print table header ---
                        << setw(8)  << "ID"
                        << setw(10) << "Type"
                        << setw(12) << "VictimID"
                        << setw(30) << "Location"
                        << setw(10) << "Severity"
                        << setw(12) << "Date"
                        << setw(8)  << "Time"
                        << setw(12) << "Status" << endl;

                    cout << string(102, '-') << endl;

                      // --- Read & display incidents ---
                    while (file.read(reinterpret_cast<char*>(&inc), sizeof(inc))) {
                        cout << left
                            << setw(8)  << inc.incidentID
                            << setw(10) << inc.type
                            << setw(12) << inc.victimID
                            << setw(30) << inc.location
                            << setw(10) << inc.severity
                            << setw(12) << inc.date
                            << setw(8)  << inc.time
                            << setw(12) << inc.status
                            << endl;
                    }

                file.close();
                break;
            }

            case 3: // Search Incident by ID
                int id;
                cout << "Enter Incident ID to search: ";
                id = getIntInput();
                file.open("incidents.dat", ios::in | ios::binary);
                bool found; found = false;
                while(file.read(reinterpret_cast<char*>(&inc), sizeof(inc))) {
                    if(inc.incidentID == id) {
                        cout << "Incident Found:\n";
                        cout << "ID: " << inc.incidentID << "\nType: " << inc.type
                             << "\nVictim ID: " << inc.victimID
                             << "\nLocation: " << inc.location
                             << "\nSeverity: " << inc.severity
                             << "\nDate: " << inc.date
                             << "\nTime: " << inc.time
                             << "\nStatus: " << inc.status << "\n";
                        found = true;
                        break;
                    }
                }
                if(!found) cout << "Incident not found!\n";
                file.close();
                break;

            case 4: // Mark as Resolved
                 {
                cout << "Enter Incident ID to mark resolved: ";
                id = getIntInput();

                file.open("incidents.dat", ios::in | ios::out | ios::binary);
                if (!file) {
                    cout << "Error opening file.\n";
                    break;
                }

                found = false;

                while (file.read(reinterpret_cast<char*>(&inc), sizeof(inc))) {
                    if (inc.incidentID == id) {
                        strcpy(inc.status, "Resolved");   // keep consistent case

                        file.seekp(-static_cast<int>(sizeof(inc)), ios::cur);
                        file.write(reinterpret_cast<char*>(&inc), sizeof(inc));

                        cout << "Incident marked as resolved!\n";
                        found = true;
                        break;
                    }
                }

                if (!found)
                    cout << "Incident not found!\n";

                file.close();
                break;
            }
                    
            case 5: { // Delete Incident by ID
                 char deleteMore = 'Y';
                while (deleteMore == 'Y') {
                    int attempts = 0;
                    int id;
                    bool found = false;
                    while (attempts < 3 && !found) {
                        cout << "Enter Incident ID to delete: ";
                        id = getIntInput();
                        file.open("incidents.dat", ios::in | ios::binary);
                        if (!file) {
                            cout << "No records found.\n";
                            break;
                        }
                        Incident tempInc;
                        while (file.read(reinterpret_cast<char*>(&tempInc), sizeof(Incident))) {
                            if (tempInc.incidentID == id) {
                                found = true;
                                break;
                            }
                        }
                        file.close();
                        if (!found) {
                            attempts++;
                            if (attempts < 3) {
                                cout << "Incident ID not found. You have " << (3 - attempts) << " chance(s) left.\n";
                            } else {
                                cout << "You have used all 3 chances. Returning to menu.\n";
                            }
                        }
                    }
                    if (!found) {
                        break;
                    }
                    // Perform deletion
                    ifstream inFile("incidents.dat", ios::in | ios::binary);
                    ofstream outFile("temp.dat", ios::out | ios::binary);
                    Incident inc;
                    while (inFile.read(reinterpret_cast<char*>(&inc), sizeof(Incident))) {
                        if (inc.incidentID != id) {
                            outFile.write(reinterpret_cast<char*>(&inc), sizeof(Incident));
                        }
                    }
                    inFile.close();
                    outFile.close();
                    remove("incidents.dat");
                    rename("temp.dat", "incidents.dat");
                    cout << "Incident deleted successfully!\n";
                    cout << "\nDelete another incident? (Y/N): ";
                    cin >> deleteMore;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    deleteMore = toupper(deleteMore);
                }
                break;
            }
            case 6:
                cout << "Returning to main menu...\n";
                break;

            default:
                cout << "Invalid choice!\n";
        }
        if(choice != 6) pauseSystem();

    } while(choice != 6);
}
//======= Report generating system=========
void generateReports() {
    int choice;
    do {
        cout << "\n=== Analytics ===\n";
        cout << "1. Number of Refugees by Location\n";
        cout << "2. Aid Distribution Summary\n";
        cout << "3. High-Risk Areas Sorted by Incident Reports\n";
        cout << "4. Family Reunification Statistics\n";
        cout << "5. Back to Main Menu\n";
        cout << "Enter your choice: ";
        choice = getIntInput();

        switch(choice) {

            case 1:  { // Number of Refugees by Location
                Refugee r; 

                ifstream inFile("refugees.dat", ios::in | ios::binary);
                if(!inFile) {
                    cout << "No refugee records found.\n";
                     break; }

                     int shelterCount = 0;
                     int tentCount = 0;
                     int housingCount = 0;
                     int familyCount = 0;
                     int tempCount = 0;

             while(inFile.read(reinterpret_cast<char*>(&r), sizeof(r))) {
              if(strcmp(r.location, "Shelter Block") == 0)
               shelterCount++;
              else if(strcmp(r.location, "Tent Zone") == 0)
               tentCount++;
              else if(strcmp(r.location, "Housing Unit") == 0)
               housingCount++;
              else if(strcmp(r.location, "Family Shelter") == 0)
               familyCount++;
              else if(strcmp(r.location, "Temporary Shelter") == 0)
               tempCount++;
             }

    cout << "Number of Refugees by Living Location:\n";
    cout << "Shelter Block: " << shelterCount << "\n";
    cout << "Tent Zone: " << tentCount << "\n";
    cout << "Housing Unit: " << housingCount << "\n";
    cout << "Family Shelter: " << familyCount << "\n";
    cout << "Temporary Shelter: " << tempCount << "\n";

    inFile.close();
    break;
}case 2: { // Aid Distribution Summary
                Aid a;

                ifstream inFile("aid.dat", ios::in | ios::binary);
                if(!inFile) {
                    cout << "No aid records found.\n";
                    break;
                 }

                int totalFood=0, totalWater=0, totalMedicine=0;
               
                 while (true) {
    // Read Aid ID
    inFile.read(reinterpret_cast<char*>(&a.aidID), sizeof(int));
    if (inFile.eof()) break;

    // Read fixed fields
    inFile.read(reinterpret_cast<char*>(a.type), sizeof(a.type));
    inFile.read(reinterpret_cast<char*>(&a.quantity), sizeof(int));
    inFile.read(reinterpret_cast<char*>(a.distributionLocation), sizeof(a.distributionLocation));
    inFile.read(reinterpret_cast<char*>(a.description), sizeof(a.description));

    // Read dynamic part
    inFile.read(reinterpret_cast<char*>(&a.numRecipients), sizeof(int));
    a.recipientIDs = new int[a.numRecipients];
    inFile.read(reinterpret_cast<char*>(a.recipientIDs),
                sizeof(int) * a.numRecipients);

    // count
    if (strcmp(a.type, "Food") == 0)
        totalFood += a.quantity;
    else if (strcmp(a.type, "Water") == 0)
        totalWater += a.quantity;
    else if (strcmp(a.type, "Medicine") == 0)
        totalMedicine += a.quantity;

    delete[] a.recipientIDs;
}
                cout << "Here is the Aid Distribution Summary:\n";
                cout << "Food: " << totalFood << "\n";
                cout << "Water: " << totalWater << "\n";
                cout << "Medicine: " << totalMedicine << "\n";

                inFile.close();
                break;
            }

            case 3: {
                // High-Risk Areas by Incident Reports
    Incident inc;

    //Open the file
    ifstream inFile("incidents.dat", ios::in | ios::binary);
    if(!inFile) {
        cout << "No incident records found.\n";
        break;
    }

    //  Initialize counters for all fixed locations
    int shelterCount = 0;
    int waterCount = 0;
    int foodCount = 0;
    int schoolCount = 0;
    int latrineCount = 0;
    int entranceCount = 0;
    int outsideCount = 0;

    //  Count incidents per location
    while(inFile.read(reinterpret_cast<char*>(&inc), sizeof(inc))) {
        if(strcmp(inc.location, "Shelter(residence) Area") == 0)
             shelterCount++;
        else if(strcmp(inc.location, "Water Point") == 0)
             waterCount++;
        else if(strcmp(inc.location, "Food Distribution Center") == 0)
             foodCount++;
        else if(strcmp(inc.location, "School / Learning Center") == 0)
             schoolCount++;
        else if(strcmp(inc.location, "Latrine area") == 0)
             latrineCount++;
        else if(strcmp(inc.location, "Camp Entrance") == 0)
             entranceCount++;
        else if(strcmp(inc.location, "Out side the camp") == 0)
             outsideCount++;
    }

    inFile.close();

    //  Store names and counts in parallel arrays
    const int LOC_NUM = 7;
   
       string areaNames[LOC_NUM] = {
        "Shelter(residence) Area",
        "Water Point",
        "Food Distribution Center",
        "School / Learning Center",
        "Latrine area",
        "Camp Entrance",
        "Out side the camp"
    };


    int counts[LOC_NUM] = {shelterCount, waterCount, foodCount, schoolCount,
                           latrineCount, entranceCount, outsideCount};

    // Sort areas by counts (descending)
    for(int i = 0; i < LOC_NUM - 1; i++) {
        for(int j = 0; j < LOC_NUM - i - 1; j++) {
            if(counts[j] < counts[j+1]) {  // descending order
                // Swap counts
                int tempCount = counts[j];
                counts[j] = counts[j+1];
                counts[j+1] = tempCount;

                // Swap corresponding names
                string tempName = areaNames[j];
                areaNames[j] = areaNames[j+1];
                areaNames[j+1] = tempName;
            }
        }
    }

    //  Print sorted results
    cout << "\nHigh-Risk Areas Sorted by Incident Counts:\n";
    for(int i = 0; i < LOC_NUM; i++) {
        cout << areaNames[i] << ": " << counts[i] << " incidents\n";
    }

    break;
}       case 4:{ // Family Reunification Statistics
                FamilyCase f; 

                ifstream inFile("family.dat", ios::in | ios::binary);
                if(!inFile) {
                    cout << "No family records found.\n";
                    break; }

                int totalFamilies=0, reunitedCount=0;
                while(inFile.read(reinterpret_cast<char*>(&f), sizeof(f))) {
                    totalFamilies++;
                    if(f.reunited)
                      reunitedCount++;
                }

                cout << "Family Reunification Statistics:\n";
                cout << "Total Families: " << totalFamilies << "\n";
                cout << "Reunited: " << reunitedCount << "\n";
                cout << "Not Reunited: " << (totalFamilies-reunitedCount) << "\n";

                inFile.close();
                break;
            }
            case 5:
                cout << "Returning to main menu...\n";
                break;

            default:
                cout << "Invalid choice!\n";
        }
        if(choice != 5) pauseSystem();

    } while(choice != 5);
}
int main() {
     // Authenticate user first
    if(!authenticateUser()) {
        cout << "Exiting program.\n";
        return 0; // exit if authentication fails
    }
    int mainChoice;
    do {
        //program description

        cout << "\n\n\n";
        cout << "======================================================\n";
        cout << "   Post Conflict Humanitarian Aid & Family Reunification System in refugee camps.  \n";
        cout << "======================================================\n";
        cout << "1. Refugee Management\n";
        cout << "2. Family Reunification\n";
        cout << "3. Humanitarian Aid Management\n";
        cout << "4. Incident Reporting\n";
        cout << "5. Analytics / Reporting\n";
        cout << "6. Exit\n";
        cout << "Enter your choice: ";
        mainChoice = getIntInput();

        switch(mainChoice) {
            case 1: manageRefugees(); break;
            case 2: manageFamilyReunification(); break;
            case 3: manageAid(); break;
            case 4: manageIncidents(); break;
            case 5: generateReports(); break;
            case 6: cout << "Exiting...\n"; break;
            default: cout << "Invalid choice!\n";
        }
    } while(mainChoice != 6);

    return 0;
}




# Refugee Management System

A console-based C++ application built to support post-conflict humanitarian operations in refugee camps. It helps NGOs and camp staff manage refugee registration, family reunification, humanitarian aid distribution, and incident reporting — all from a single, role-based terminal interface.

## Features

**🔐 Role-Based Authentication**
- Login system with three roles: Admin, Staff, and Volunteer
- Limited password attempts for basic security

**🧍 Refugee Management**
- Add, view, update, and delete refugee records
- Search refugees by ID, name, age range, or gender

**👪 Family Reunification**
- Track family cases and members
- Search family cases by Family ID
- Mark families as reunited
- Update last contact information

**📦 Humanitarian Aid Management**
- Record aid distribution (food, water, medicine, etc.)
- View, search, and delete aid records
- Track recipients and distribution locations

**🚨 Incident Reporting**
- Log incidents with type, severity, date, time, and status
- Search incidents by ID
- Mark incidents as resolved

**📊 Analytics / Reporting**
- Number of refugees by location
- Aid distribution summary
- High-risk areas sorted by incident reports
- Family reunification statistics

## Tech Stack

- **Language:** C++
- **Data storage:** Binary `.dat` files (`refugees.dat`, `family.dat`, `aid.dat`, `incidents.dat`)
- **I/O:** Standard C++ file streams (`fstream`)

## Getting Started

### Prerequisites
- A C++ compiler (e.g., `g++`)
- Git installed on your machine

### Clone the repository

```bash
git clone https://github.com/Betty-Gm/RefugeeManagementSystem.git
cd RefugeeManagementSystem
```

### Compile and Run

```bash
g++ refugeemanagementProject.cpp -o refugeeSystem
./refugeeSystem
```

On Windows (using g++ via MinGW):

```bash
g++ refugeemanagementProject.cpp -o refugeeSystem.exe
refugeeSystem.exe
```

### Usage
1. Run the program and log in with a role (Admin / Staff / Volunteer)
2. Choose a module from the main menu:
   - Refugee Management
   - Family Reunification
   - Humanitarian Aid Management
   - Incident Reporting
   - Analytics / Reporting
3. Follow the on-screen prompts to add, view, search, update, or delete records

## Project Structure

```
RefugeeManagementSystem/
├── refugeemanagementProject.cpp   # Main application source code
├── refugees.dat                   # Refugee records (binary)
├── family.dat                     # Family case records (binary)
├── aid.dat                        # Aid distribution records (binary)
├── incidents.dat                  # Incident reports (binary)
└── README.md
```

## Future Improvements
- Migrate from binary flat files to a proper database (e.g., SQLite)
- Add input validation and stronger error handling throughout
- Build a GUI or web-based front end
- Add data export (CSV/PDF) for reports

## Author
Built by [Beti](https://github.com/Betty-Gm) as a university project focused on C++ OOP and file-based data management.

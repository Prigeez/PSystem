#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <conio.h>  
#include <cstdlib> 
#include <cstdio>
#include <ctime> 
#include <windows.h> 
#include <sstream>
#include <map>
using namespace std;               //BENAVIDEZ, PRINCE ERL ORIGINATED
enum ConsoleColor {
    C_RESET = 7,
    C_BLUE = 9,
    C_RED = 12,
    C_GREEN = 10,
    C_YELLOW = 6,
    C_CYAN = 11,
};
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color);
}
const string USERS_HEADER        = "# PRINCEB USERS v1";
const string SUBJECTS_HEADER     = "# PRINCEB SUBJECTS v1";
const string EXAMS_HEADER        = "# PRINCEB EXAMS v1";
const string GRADES_HEADER       = "# PRINCEB GRADES v1";
const string DELETED_USERS_HDR   = "# PRINCEB DELETED_USERS v1";
const string MODULES_HEADER      = "# PRINCEB MODULES v1";
bool isWeakPassword(const string& pwd) {
    if (pwd.length() < 6) return true;
    bool hasDigit = false, hasAlpha = false;
    for (size_t i = 0; i < pwd.length(); ++i) {
        if (isdigit((unsigned char)pwd[i])) hasDigit = true;
        if (isalpha((unsigned char)pwd[i])) hasAlpha = true;
    }
    return !(hasDigit && hasAlpha);
}
int getMenuChoice(int minOpt, int maxOpt) {
    int ch;
    while (true) {
        setColor(C_RESET);
        cout << "\nEnter choice: ";
        cin >> ch;
        if (!cin.fail() && ch >= minOpt && ch <= maxOpt) return ch;
        cin.clear();
        cin.ignore(10000, '\n');
        setColor(C_RED);
        cout << "Invalid input! Please enter a number between " << minOpt << " and " << maxOpt << ".\n";
        setColor(C_RESET);
    }
}
struct User {
    string username;
    string password;
    string role;
};
struct Subject {
    string code;
    string name;
};
struct Question {
    string questionText;
    vector<string> choices;
    int correctIndex;
};
struct Exam {
    string subject;
    vector<Question> questions;
};
struct Grade {
    string studentUsername;
    string subject;
    int score;
    double average;
    string remark;
};
struct Module {
    string title;
    vector<string> contentLines;
};
vector<User> users;
vector<Subject> subjects;
vector<Exam> exams;
vector<Grade> grades;
vector<Module> modules;
struct LoginAttempt {
    int count;
    time_t lastTry;
};
vector<LoginAttempt> loginAttempts; 
void pause() {
    setColor(C_YELLOW);
    cout << "\nPress ENTER to continue...";
    cin.ignore(10000, '\n');
    cin.get();
}
void clearScreen() {
    system("cls");
}
string lower(const string& s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(),
        static_cast<int(*)(int)>(tolower)
    );
    return r;
}
bool ciStringLess(const string &a, const string &b) {
    return lower(a) < lower(b);
}
bool gradeStudentLess(const Grade &a, const Grade &b) {
    return lower(a.studentUsername) < lower(b.studentUsername);
}
bool userExists(const string& uname) {
    for (size_t i = 0; i < users.size(); i++)
        if (lower(users[i].username) == lower(uname)) return true;
    return false;
}
User* findUser(const string& uname) {
    for (size_t i = 0; i < users.size(); i++)
        if (lower(users[i].username) == lower(uname)) return &users[i];
    return NULL;
}
Subject* findSubjectByCode(const string& code) {
    for (size_t i = 0; i < subjects.size(); i++)
        if (lower(subjects[i].code) == lower(code)) return &subjects[i];
    return NULL;
}
Subject* findSubjectByName(const string& name) {
    for (size_t i = 0; i < subjects.size(); i++)
        if (lower(subjects[i].name) == lower(name)) return &subjects[i];
    return NULL;
}
Subject* findSubject(const string& s) {
    Subject* byCode = findSubjectByCode(s);
    if (byCode) return byCode;
    return findSubjectByName(s);
}
Exam* findExam(const string& subjCode) {
    for (size_t i = 0; i < exams.size(); i++)
        if (lower(exams[i].subject) == lower(subjCode)) return &exams[i];
    return NULL;
}
Module* findModule(const string& title) {
    for (size_t i = 0; i < modules.size(); ++i)
        if (lower(modules[i].title) == lower(title)) return &modules[i];
    return NULL;
}
string inputPassword(const char* prompt = "\nPassword: ", bool mask = true) {
    if (mask) {
        string pwd = "";
        int ch;
        printf("%s", prompt);
        while ((ch = getch()) != 13) { 
            if (ch == 8) { 
                if (!pwd.empty()) {
                    pwd.resize(pwd.size() - 1);
                    printf("\b \b");
                }
            } else if ((size_t)pwd.size() < 32 && ch >= 32 && ch <= 126) {
                pwd.push_back((char)ch);
                printf("*");
            }
        }
        printf("\n");
        return pwd;
    } else {
        
        cout << prompt;
        string pwd;
        getline(cin >> ws, pwd);
        return pwd;
    }
}
void backupFile(const char* filename) {
    char backupname[256];
    std::sprintf(backupname, "%s.bak", filename);
    FILE *src = fopen(filename, "rb");
    FILE *dst = fopen(backupname, "wb");
    if (!src || !dst) { setColor(C_RED); puts("Backup failed."); setColor(C_RESET); if (src) fclose(src); if (dst) fclose(dst); return; }
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0)
        fwrite(buf, 1, n, dst);
    fclose(src); fclose(dst);
    setColor(C_GREEN);
    printf("Backup saved as %s\n", backupname);
    setColor(C_RESET);
    pause();
}
void restoreFile(const char* filename) {
    char backupname[256];
    std::sprintf(backupname, "%s.bak", filename);
    FILE *src = fopen(backupname, "rb");
    FILE *dst = fopen(filename, "wb");
    if (!src || !dst) { setColor(C_RED); puts("Restore failed."); setColor(C_RESET); if (src) fclose(src); if (dst) fclose(dst); return; }
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0)
        fwrite(buf, 1, n, dst);
    fclose(src); fclose(dst);
    setColor(C_GREEN);
    printf("Restore completed from %s\n", backupname);
    setColor(C_RESET);
    pause();
}
void showHelp() {
    setColor(C_GREEN);
    puts("\n======================== PSYS HELP OPTION =================================");
    setColor(C_RESET);
    puts("Welcome to the PRINCE SYSTEM (PSYS).");
    puts("This is a Console-based Exam & Learning System.");
    puts("");
    puts("Quick Start:");
    puts("- Register as TEACHER or STUDENT (or ask ADMIN to create an account).");
    puts("- Login and use the role menu (ADMIN/TEACHER/STUDENT) to access features.");
    puts("");
    puts("Contacts:");
    setColor(C_YELLOW);
    puts("- System Administrator: adm1npisys@gmail.com");
    setColor(C_RESET);
    puts("");
    puts("Important Notes:");
    puts("- Always remember if you want to cancel or exit just type zero.");
    puts("- Data files are stored in the program folder: users.txt, subjects.txt, exams.txt, grades.txt, modules.txt.");
    puts("- Backups use .bak (e.g., users.txt.bak). Deleted users get recorded in deleted_users.txt.");
    puts("");
    puts("Exam Rules:");
    puts("- Exams are timed (default: 15 minutes) and contain 10 questions.");
    puts("- Questions and choices are shuffled per attempt to reduce cheating.");
    puts("- Average is calculated as score * 10 (max 100).");
    puts("- Retakes may overwrite previous results if you confirm.");
    puts("");
    puts("Account & Passwords:");
    puts("- Passwords are currently stored in plaintext in users.txt (admin action required for hashing).");
    puts("- Weak password warning: at least 6 characters with letters and digits is recommended.");
    puts("- 3 failed login attempts locks the account for the running session.");
    puts("");
    puts("Backups & Restore:");
    puts("- Use the Backup/Restore menu to create/restore .bak copies.");
    puts("- When an admin deletes a user, the system backs up that user's grades to grades_<username>.bak for possible restore.");
    puts("");
    puts("Admin Responsibilities:");
    puts("- Keep backups safe, review exam definitions, and manage user restores when needed.");
    puts("");
    setColor(C_YELLOW);
    puts("Security Recommendations (maintainers):");
    setColor(C_RESET);
    puts("- Do not keep plaintext passwords in production and implement hashing like (bcrypt/Argon2).");
    puts("- Consider encrypting backups and adding persistent lockout windows.");
    puts("");
    setColor(C_GREEN);
    puts("==========================================================================\n");
    setColor(C_RESET);
    pause();
}
void initLoginAttempts() {
    loginAttempts.clear();
    for (size_t i = 0; i < users.size(); ++i) {
        LoginAttempt la;
        la.count = 0;
        la.lastTry = 0;
        loginAttempts.push_back(la);
    }
}
string sanitizeFilename(const string& s) {
    string out = s;
    for (size_t i = 0; i < out.size(); ++i) {
        if (!isalnum((unsigned char)out[i])) out[i] = '_';
    }
    return out;
}
string makeCodeFromName(const string& name) {
    string out;
    for (size_t i = 0; i < name.size(); ++i) {
        char c = name[i];
        if (isalnum((unsigned char)c)) out.push_back((char)toupper(c));
        else if (c == ' ' || c == '-' || c == '_') out.push_back('_');
    }
    if (out.empty()) out = "SUBJ";
    if (out.size() > 12) out = out.substr(0,12);
    return out;
}
void showStartupLoading() {
    clearScreen();
    setColor(C_RESET);
    cout << "WELCOME TO PRINCE SYSTEM (PSYS)\n";
    setColor(C_GREEN);
    cout << "Initializing";
    cout.flush();
    setColor(C_BLUE);
    for (int i = 0; i < 3; ++i) { cout << "."; cout.flush(); Sleep(300); }
    cout << "\n\n";
    const int barWidth = 40;
    for (int pct = 0; pct <= 100; pct += 2) {
        int pos = (pct * barWidth) / 100;
        cout << "\r[";
        for (int i = 0; i < pos; ++i) cout << "=";
        for (int i = pos; i < barWidth; ++i) cout << " ";
        cout << "] " << setw(3) << pct << "%";
        cout.flush();
        Sleep(35);
    }
    cout << "\n\n";
    setColor(C_RESET);
    Sleep(250);
}
void showLoginLoading() {
    setColor(C_GREEN);
    cout << "\nAuthenticating and preparing your workspace";
    cout.flush();
    for (int i = 0; i < 3; ++i) { cout << "."; cout.flush(); Sleep(300); }
    cout << "\n";
    setColor(C_BLUE);
    const char spinner[] = "|/-\\";
    int cycles = 14;
    for (int i = 0; i < cycles; ++i) {
        int pct = (i * 100) / cycles;
        cout << "\r" << spinner[i % 4] << " Loading " << setw(3) << pct << "%";
        cout.flush();
        Sleep(90);
    }
    cout << "\r" << "                         " << "\r";
    setColor(C_RESET);
    Sleep(200);
}
void showLogoutLoading(bool autoClose = false) {
    clearScreen();
    setColor(C_GREEN);
    cout << "\nProcessing logout and closing application";
    cout.flush();
    for (int i = 0; i < 3; ++i) { cout << "."; cout.flush(); Sleep(300); }
    cout << "\n";
    setColor(C_BLUE);
    const char spinner[] = "|/-\\";
    int cycles = 20;
    for (int i = 0; i < cycles; ++i) {
        int pct = (i * 100) / cycles;
        cout << "\r" << spinner[i % 4] << " Finalizing " << setw(3) << pct << "%";
        cout.flush();
        Sleep(70);
    }
    cout << "\n\n";
    const int barWidth = 40;
    for (int pct = 0; pct <= 100; pct += 5) {
        int pos = (pct * barWidth) / 100;
        cout << "\r[";
        for (int i = 0; i < pos; ++i) cout << "=";
        for (int i = pos; i < barWidth; ++i) cout << " ";
        cout << "] " << setw(3) << pct << "%";
        cout.flush();
        Sleep(25);
    }
    cout << "\n\n";
    setColor(C_YELLOW);
    cout << "\nThank you for using PSYS, I hope you like it!\nMade by: PRINCE BENAVIDEZ PSYS ADMIN\n";
    setColor(C_RESET);
    Sleep(400);
    if (autoClose) {
        cout.flush();
        exit(0);
    }
}
void loadUsers() {
    users.clear();
    ifstream fin("users.txt");
    if (!fin.is_open()) {
        initLoginAttempts();
        return;
    }
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') continue; 
        istringstream iss(line);
        User u;
        if (!(iss >> u.username >> u.password >> u.role)) continue; 
        users.push_back(u);
    }
    fin.close();
    initLoginAttempts();
}
void saveUsers() {
    ofstream fout("users.txt");
    if (!fout.is_open()) {
        setColor(C_RED);
        cout << "Error: failed to open users.txt for writing. Changes may not be saved.\n";
        setColor(C_RESET);
        return;
    }
    fout << USERS_HEADER << endl;
    for (size_t i = 0; i < users.size(); i++)
        fout << users[i].username << " " << users[i].password << " " << users[i].role << endl;
    fout.flush();
    fout.close();
}
void loadSubjects() {
    subjects.clear();
    ifstream fin("subjects.txt");
    if (!fin.is_open()) return;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        Subject s;
        size_t pos = line.find('|');
        if (pos != string::npos) {
            s.code = line.substr(0, pos);
            s.name = line.substr(pos+1);
            if (!s.code.empty() && s.code[s.code.size()-1] == ' ') s.code.erase(s.code.size()-1);
        } else {
            s.name = line;
            s.code = makeCodeFromName(s.name);
            int suffix = 1;
            string base = s.code;
            while (findSubjectByCode(s.code) != NULL) {
                char buf[32];
                sprintf(buf, "%d", suffix++);
                s.code = base + "_" + buf;
            }
        }
        subjects.push_back(s);
    }
    fin.close();
}
void saveSubjects() {
    ofstream fout("subjects.txt");
    if (!fout.is_open()) {
        setColor(C_RED);
        cout << "Error: failed to open subjects.txt for writing. Changes may not be saved.\n";
        setColor(C_RESET);
        return;
    }
    fout << SUBJECTS_HEADER << endl;
    for (size_t i = 0; i < subjects.size(); i++)
        fout << subjects[i].code << "|" << subjects[i].name << endl;
    fout.flush();
    fout.close();
}
void loadExams() {
    exams.clear();
    ifstream fin("exams.txt");
    if (!fin.is_open()) return;
    string subj;
    while (getline(fin, subj)) {
        if (subj.empty()) continue;
        if (subj[0] == '#') continue; 
        Exam e;
        Subject* s = findSubjectByCode(subj);
        if (s) e.subject = s->code;
        else {
            Subject* s2 = findSubjectByName(subj);
            if (s2) e.subject = s2->code;
            else e.subject = subj;
        }
        for (int i = 0; i < 15; ++i) {
            Question q;
            if (!getline(fin, q.questionText)) break;
            if (q.questionText.empty()) break;
            string nchoicesLine;
            if (!getline(fin, nchoicesLine)) break;
            int nchoices = atoi(nchoicesLine.c_str());
            q.choices.clear();
            for (int j = 0; j < nchoices; ++j) {
                string choice;
                if (!getline(fin, choice)) break;
                q.choices.push_back(choice);
            }
            string correctLine;
            if (!getline(fin, correctLine)) break;
            q.correctIndex = atoi(correctLine.c_str());
            e.questions.push_back(q);
        }
        exams.push_back(e);
        string blank;
        getline(fin, blank); 
    }
    fin.close();
}
void saveExams() {
    ofstream fout("exams.txt");
    if (!fout.is_open()) {
        setColor(C_RED);
        cout << "Error: failed to open exams.txt for writing. Changes may not be saved.\n";
        setColor(C_RESET);
        return;
    }
    fout << EXAMS_HEADER << endl;
    for (size_t i = 0; i < exams.size(); i++) {
        fout << exams[i].subject << endl;
        for (size_t k = 0; k < exams[i].questions.size(); k++) {
            fout << exams[i].questions[k].questionText << endl;
            fout << exams[i].questions[k].choices.size() << endl;
            for (size_t j = 0; j < exams[i].questions[k].choices.size(); j++)
                fout << exams[i].questions[k].choices[j] << endl;
            fout << exams[i].questions[k].correctIndex << endl;
        }
        fout << endl;
    }
    fout.flush();
    fout.close();
}
void loadGrades() {
    grades.clear();
    ifstream fin("grades.txt");
    if (!fin.is_open()) return;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        istringstream iss(line);
        Grade g;
        string subjToken;
        if (!(iss >> g.studentUsername >> subjToken >> g.score >> g.average >> g.remark)) continue;
        Subject* s = findSubjectByCode(subjToken);
        if (s) g.subject = s->code;
        else {
            Subject* s2 = findSubjectByName(subjToken);
            if (s2) g.subject = s2->code;
            else g.subject = subjToken;
        }
        grades.push_back(g);
    }
    fin.close();
}
void saveGrades() {
    ofstream fout("grades.txt");
    if (!fout.is_open()) {
        setColor(C_RED);
        cout << "Error: failed to open grades.txt for writing. Changes may not be saved.\n";
        setColor(C_RESET);
        return;
    }
    fout << GRADES_HEADER << endl;
    for (size_t i = 0; i < grades.size(); i++)
        fout << grades[i].studentUsername << " " << grades[i].subject << " " << grades[i].score << " " << grades[i].average << " " << grades[i].remark << endl;
    fout.flush();
    fout.close();
}
void loadModules() {
    modules.clear();
    ifstream fin("modules.txt");
    if (!fin.is_open()) return;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        Module m;
        m.title = line;
        string nlinesStr;
        if (!getline(fin, nlinesStr)) break;
        int nlines = atoi(nlinesStr.c_str());
        for (int i = 0; i < nlines; ++i) {
            string contentLine;
            if (!getline(fin, contentLine)) break;
            m.contentLines.push_back(contentLine);
        }
        modules.push_back(m);
        string blank;
        getline(fin, blank);
    }
    fin.close();
}
void saveModules() {
    ofstream fout("modules.txt");
    if (!fout.is_open()) {
        setColor(C_RED);
        cout << "Error: failed to open modules.txt for writing. Changes may not be saved.\n";
        setColor(C_RESET);
        return;
    }
    fout << MODULES_HEADER << endl;
    for (size_t i = 0; i < modules.size(); ++i) {
        fout << modules[i].title << endl;
        fout << modules[i].contentLines.size() << endl;
        for (size_t j = 0; j < modules[i].contentLines.size(); ++j) {
            fout << modules[i].contentLines[j] << endl;
        }
        fout << endl;
    }
    fout.flush();
    fout.close();
}
void saveAll() {
    saveUsers();
    saveSubjects();
    saveExams();
    saveGrades();
    saveModules();
}
void searchModules() {
    string part;
    cout << "Enter part of module title to search (0 to cancel): ";
    cin.ignore();
    getline(cin, part);
    if (part == "0") { setColor(C_BLUE); cout << "Cancelled.\n"; setColor(C_RESET); pause(); return; }
    setColor(C_GREEN);
    cout << "\nMatching modules:\n";
    setColor(C_BLUE);
    for (size_t i = 0; i < modules.size(); ++i) {
        if (modules[i].title.find(part) != string::npos) {
            cout << (i+1) << ". " << modules[i].title << endl;
        }
    }
    setColor(C_RESET);
    pause();
}
void adminManageModules() {
    while (1) {
        clearScreen();
        setColor(C_RESET);
        cout << "\n1. View modules\n2. Add module\n3. Delete module\n4. Search modules\n0. Back\n";
        int ch = getMenuChoice(0, 4);
        if (ch == 0) break;
        if (ch == 1) {
            clearScreen();
            if (modules.empty()) {
                setColor(C_BLUE); cout << "No modules available.\n"; setColor(C_RESET); pause(); continue;
            }
            cout << "\nModules:\n";
            for (size_t i = 0; i < modules.size(); ++i) {
                cout << (i+1) << ". " << modules[i].title << endl;
            }
            cout << "\nChoose module number to view (0 to cancel): ";
            int sel; cin >> sel;
            setColor(C_BLUE);
            if (sel <= 0 || sel > (int)modules.size()) { cout << "Cancelled.\n"; pause(); continue; }
            Module &m = modules[sel-1];
            clearScreen();
            setColor(C_GREEN);
            cout << "\n=== " << m.title << " ===\n";
            setColor(C_RESET);
            for (size_t li = 0; li < m.contentLines.size(); ++li)
                cout << m.contentLines[li] << endl;
            pause();
        } else if (ch == 2) {
            cout << "Module Title (type 0 to cancel): ";
            cin.ignore();
            string title;
            getline(cin, title);
            if (title == "0") { setColor(C_BLUE); cout << "Cancelled.\n"; setColor(C_RESET); pause(); continue; }
            if (title.empty()) { setColor(C_RED); cout << "Title cannot be empty.\n"; setColor(C_RESET); pause(); continue; }
            if (findModule(title)) { setColor(C_RED); cout << "Module title already exists.\n"; setColor(C_RESET); pause(); continue; }
            Module m;
            m.title = title;
            cout << "Enter module content. Type a single line containing just a dot (.) to finish.\n";
            string line;
            while (true) {
                getline(cin, line);
                if (line == ".") break;
                m.contentLines.push_back(line);
            }
            modules.push_back(m);
            saveModules();
            setColor(C_GREEN); cout << "Module added.\n"; setColor(C_RESET);
            pause();
        } else if (ch == 3) {
            clearScreen();
            if (modules.empty()) { setColor(C_BLUE); cout << "No modules to delete.\n"; setColor(C_RESET); pause(); continue; }
            cout << "\nModules:\n";
            for (size_t i = 0; i < modules.size(); ++i) cout << (i+1) << ". " << modules[i].title << endl;
            cout << "Choose module number to delete (0 to cancel): ";
            int idx; cin >> idx;
            setColor(C_BLUE);
            if (idx <= 0 || idx > (int)modules.size()) { cout << "Cancelled.\n"; pause(); continue; }
            string t = modules[idx-1].title;
            modules.erase(modules.begin() + (idx-1));
            saveModules();
            setColor(C_GREEN); cout << "Deleted module: " << t << endl; setColor(C_RESET);
            pause();
        } else if (ch == 4) {
            searchModules();
        }
    }
}
void appendDeletedUserRecord(const User& u) {
    ofstream fout("deleted_users.txt", ios::app);
    if (!fout.is_open()) return;
    static bool headerChecked = false;
    if (!headerChecked) {
        headerChecked = true;
        ifstream fin("deleted_users.txt");
        bool emptyFile = true;
        if (fin.is_open()) {
            string tmp;
            if (getline(fin, tmp)) emptyFile = false;
            fin.close();
        }
        if (emptyFile) {
            fout << DELETED_USERS_HDR << endl;
        }
    }
    fout << u.username << " " << u.password << " " << u.role << endl;
    fout.close();
}
vector<User> loadDeletedUsers() {
    vector<User> res;
    ifstream fin("deleted_users.txt");
    if (!fin.is_open()) return res;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        istringstream iss(line);
        User u;
        if (!(iss >> u.username >> u.password >> u.role)) continue;
        res.push_back(u);
    }
    fin.close();
    return res;
}
void saveDeletedUsers(const vector<User>& list) {
    ofstream fout("deleted_users.txt");
    if (!fout.is_open()) {
        setColor(C_RED);
        cout << "Error: failed to open deleted_users.txt for writing. Changes may not be saved.\n";
        setColor(C_RESET);
        return;
    }
    fout << DELETED_USERS_HDR << endl;
    for (size_t i = 0; i < list.size(); ++i)
        fout << list[i].username << " " << list[i].password << " " << list[i].role << endl;
    fout.flush();
    fout.close();
}
void backupGradesForUser(const string& uname) {
    string san = sanitizeFilename(uname);
    string fname = "grades_" + san + ".bak";
    ofstream fout(fname.c_str());
    if (!fout.is_open()) return;
    fout << GRADES_HEADER << endl;
    for (size_t i = 0; i < grades.size(); ++i) {
        if (grades[i].studentUsername == uname) {
            fout << grades[i].studentUsername << " " << grades[i].subject << " " << grades[i].score << " " << grades[i].average << " " << grades[i].remark << endl;
        }
    }
    fout.close();
}
void removeGradesForUser(const string& uname) {
    for (vector<Grade>::iterator git = grades.begin(); git != grades.end();) {
        if (git->studentUsername == uname) git = grades.erase(git);
        else ++git;
    }
    saveGrades();
}
bool restoreGradesForUser(const string& uname) {
    string san = sanitizeFilename(uname);
    string fname = "grades_" + san + ".bak";
    ifstream fin(fname.c_str());
    if (!fin.is_open()) return false;
    string line;
    vector<Grade> toAdd;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        istringstream iss(line);
        Grade g;
        if (!(iss >> g.studentUsername >> g.subject >> g.score >> g.average >> g.remark)) continue;
        bool exists = false;
        for (size_t i = 0; i < grades.size(); ++i) {
            if (grades[i].studentUsername == g.studentUsername && lower(grades[i].subject) == lower(g.subject)) {
                exists = true;
                grades[i] = g;
                break;
            }
        }
        if (!exists) toAdd.push_back(g);
    }
    fin.close();
    for (size_t i = 0; i < toAdd.size(); ++i) grades.push_back(toAdd[i]);
    saveGrades();
    ::remove(fname.c_str());
    return true;
}
void removeDeletedUserRecord(const string& uname) {
    vector<User> list = loadDeletedUsers();
    vector<User> out;
    for (size_t i = 0; i < list.size(); ++i) {
        if (lower(list[i].username) != lower(uname)) out.push_back(list[i]);
    }
    saveDeletedUsers(out);
}
void searchUsers() {
    clearScreen();
    string part;
    cout << "Enter part of username to search (0 to cancel): ";
    cin >> part;
    if (part == "0") { setColor(C_BLUE); cout << "Cancelled.\n"; setColor(C_RESET); pause(); return; }
    setColor(C_GREEN);
    cout << left << setw(15) << "\nUsername " << setw(12) << " Role" << endl;
    cout << "=======================\n";
    for (size_t i = 0; i < users.size(); i++)
        if (users[i].username.find(part) != string::npos)
            cout << setw(15) << users[i].username << setw(12) << users[i].role << endl;
    setColor(C_RESET);
    pause();
}
void searchSubjects() {
    clearScreen();
    string part;
    cout << "Enter part of subject code or name to search (0 to cancel): ";
    cin.ignore();
    getline(cin, part);
    if (part == "0") { setColor(C_BLUE); cout << "Cancelled.\n"; setColor(C_RESET); pause(); return; }
    setColor(C_GREEN);
    for (size_t i = 0; i < subjects.size(); i++) {
        if (subjects[i].code.find(part) != string::npos || subjects[i].name.find(part) != string::npos) {
            cout << "- " << subjects[i].code << " - " << subjects[i].name << endl;
        }
    }
    setColor(C_RESET);
    pause();
}
void showWelcomeBanner(const string& uname, const string& role) {
    setColor(C_GREEN);
    cout << "\n=========================================\n";
    cout << " Welcome, " << uname << " (" << role << ")!\n";
    time_t now = time(0);
    cout << " Login time: " << ctime(&now);
    cout << "=========================================\n";
    setColor(C_RESET);
}
void createDefaultAdmin() {
    if (!userExists("admin")) {
        User u;
        u.username = "admin";
        u.password = "admin123";
        u.role = "ADMIN";
        users.push_back(u);
        saveUsers();
        initLoginAttempts();
    }
}
void registerAccount() {
    clearScreen();
    int ch;
    cout << "\nCreating Account as?\n0. Cancel\n1. TEACHER\n2. STUDENT \n";
    ch = getMenuChoice(0, 2);
    if (ch == 0) {
        setColor(C_BLUE);
        cout << "Registration cancelled.\n";
        setColor(C_RESET);
        pause();
        return;
    }
    string role = (ch == 1) ? "TEACHER" : "STUDENT";
    string uname, pwd;
    cout << "\n\n\nCreate Username: ";
    cin.ignore();
    getline(cin, uname);
    if (uname == "0") { setColor(C_BLUE); cout << "Cancelled.\n"; setColor(C_RESET); pause(); return; }
    if (userExists(uname)) { setColor(C_RED); cout << "Username already exists!\n"; setColor(C_RESET); pause(); return; }
    pwd = inputPassword("Create Password: ", true);
    if (isWeakPassword(pwd)) {
        setColor(C_RED);
        cout << "Warning: Weak password! Use at least 6 chars with letters and numbers.\n";
        setColor(C_RESET);
    }
    User u;
    u.username = uname;
    u.password = pwd;
    u.role = role;
    users.push_back(u);
    saveUsers();
    initLoginAttempts();
    setColor(C_GREEN);
    cout << "===========================================\n";
    cout << "Successfully Created! Thank for joining\n";
    cout << "===========================================\n";
    setColor(C_RESET);
    showLogoutLoading(); 
    pause();
}
bool login(string& role, string& uname) {
    clearScreen();
    setColor(C_RESET);
    cout << "\n\n\n\n\nUsername: ";
    cin >> uname;
    if (uname == "0") { setColor(C_BLUE); cout << "Login cancelled.\n"; setColor(C_RESET); pause(); return false; }
    int idx = -1;
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i].username == uname) { idx = i; break; }
    }
    if (idx == -1) { setColor(C_RED); cout << "Login failed! User Not Found!\n"; setColor(C_RESET); pause(); return false; }
    if (loginAttempts[idx].count >= 3) {
        setColor(C_RED); cout << "Account locked due to too many failed attempts. Try again later.\n"; setColor(C_RESET);
        pause(); return false;
    }
    string pwd = inputPassword("\nPassword: ", true);
    if (users[idx].password == pwd) {
        role = users[idx].role;
        loginAttempts[idx].count = 0;
        return true;
    } else {
        loginAttempts[idx].count++;
        loginAttempts[idx].lastTry = std::time(0);
        setColor(C_RED);
        cout << "Login failed! Incorrect password!\n";
        setColor(C_RESET);
        pause();
        return false;
    }
}
void viewAllAccounts() {
    clearScreen();
    setColor(C_GREEN);
    cout << left << setw(15) << "\nUsername" << setw(12) << "Role" << endl;
    setColor(C_BLUE);
    cout << "=======================\n";
    setColor(C_RESET);
    for (size_t i = 0; i < users.size(); i++)
        cout << setw(15) << users[i].username << setw(12) << users[i].role << endl;
    pause();
}
void createAccountByAdmin() {
    int ch;
    cout << "\n0. Cancel\n1. TEACHER\n2. STUDENT\n";
    ch = getMenuChoice(0, 2);
    if (ch == 0) {
        setColor(C_BLUE);
        cout << "Create account cancelled.\n";
        setColor(C_RESET);
        pause();
        return;
    }
    string role = (ch == 1) ? "TEACHER" : "STUDENT";
    string uname, pwd;
    cout << "Create Username: ";
    cin.ignore();
    getline(cin, uname);
    if (uname == "0") { setColor(C_BLUE); cout << "Cancelled.\n"; setColor(C_RESET); pause(); return; }
    if (userExists(uname)) { setColor(C_RED); cout << "Username exists!\n"; setColor(C_RESET); pause(); return; }
    pwd = inputPassword("Create Password: ");
    if (isWeakPassword(pwd)) {
        setColor(C_BLUE);
        cout << "Warning: Weak password! Use at least 6 chars with letters and numbers.\n";
        setColor(C_RESET);
    }
    User u;
    u.username = uname;
    u.password = pwd;
    u.role = role;
    users.push_back(u);
    saveUsers();
    initLoginAttempts();
    setColor(C_GREEN); cout << "Account created!\n"; setColor(C_RESET);
    pause();
}
void editAccount() {
    string uname;
    cout << "Enter username to edit (0 to cancel): ";
    cin >> uname;
    if (uname == "0") { setColor(C_BLUE); cout << "Cancelled.\n"; setColor(C_RESET); pause(); return; }
    User* u = findUser(uname);
    if (u == NULL || u->role == "ADMIN") { setColor(C_RED); cout << "User not found!\n"; setColor(C_RESET); pause(); return; }
    u->password = inputPassword();
    if (isWeakPassword(u->password)) {
        setColor(C_BLUE);
        cout << "Warning: Weak password! Use at least 6 chars with letters and numbers.\n";
        setColor(C_RESET);
    }
    cout << "Role (TEACHER/STUDENT): ";
    cin >> u->role;
    saveUsers();
    setColor(C_GREEN); cout << "Account updated!\n"; setColor(C_RESET);
    pause();
}
void deleteAccount() {
    string uname;
    cout << "Enter username to delete (0 to cancel): ";
    cin >> uname;
    if (uname == "0") { setColor(C_BLUE); cout << "Cancelled.\n"; setColor(C_RESET); pause(); return; }
    for (vector<User>::iterator it = users.begin(); it != users.end(); ++it) {
        if (it->username == uname && it->role != "ADMIN") {
            appendDeletedUserRecord(*it);
            backupGradesForUser(uname);
            removeGradesForUser(uname);
            users.erase(it);
            saveUsers();
            initLoginAttempts();
            setColor(C_GREEN); cout << "Account deleted and grades backed up to file. You can restore them from Admin -> Restore account.\n"; setColor(C_RESET);
            pause();
            return;
        }
    }
    setColor(C_RED); cout << "User not found and doesn't exist!\n"; setColor(C_RESET);
    pause();
}
void adminRestoreAccount() {
    vector<User> deleted = loadDeletedUsers();
    if (deleted.empty()) {
        setColor(C_BLUE);
        cout << "No deleted accounts to restore.\n";
        setColor(C_RESET);
        pause();
        return;
    }
    cout << "\nDeleted accounts:\n";
    for (size_t i = 0; i < deleted.size(); ++i) {
        cout << (i+1) << ". " << deleted[i].username << " (" << deleted[i].role << ")\n";
    }
    cout << "Choose account number to restore (0 to cancel): ";
    int ch;
    cin >> ch;
    if (ch <= 0 || ch > (int)deleted.size()) { cout << "Cancelled.\n"; pause(); return; }
    User u = deleted[ch-1];
    if (userExists(u.username)) {
        setColor(C_RED);
        cout << "A user with that username already exists in the system. Cannot restore.\n";
        setColor(C_RESET);
        pause();
        return;
    }
    users.push_back(u);
    saveUsers();
    initLoginAttempts();
    bool restored = restoreGradesForUser(u.username);
    removeDeletedUserRecord(u.username);
    setColor(C_GREEN);
    cout << "Account restored for " << u.username << ". ";
    if (restored) cout << "Grades restored from backup.\n";
    else cout << "No grade backup found for this user.\n";
    setColor(C_RESET);
    pause();
}
void adminViewOrResetPassword() {
    while (1) {
        clearScreen();
        setColor(C_GREEN);
        cout << "\n===== View/Reset User Password =====\n";
        setColor(C_RESET);
        cout << "1. View User Password\n2. Reset User Password\n0. Back\n";
        int ch = getMenuChoice(0, 2);
        if (ch == 0) break;
        string uname;
        cout << "Enter username (0 to cancel): ";
        cin >> uname;
        if (uname == "0") { setColor(C_BLUE); cout << "Cancelled.\n"; setColor(C_RESET); pause(); continue; }
        User* u = findUser(uname);
        if (!u || u->role == "ADMIN") {
            setColor(C_RED);
            cout << "User not found and doesn't exist!\n";
            setColor(C_RESET);
            pause();
            continue;
        }
        if (ch == 1) {
            setColor(C_BLUE);
            cout << "\nUser: " << u->username << "\nRole: " << u->role << "\nPassword: " << u->password << endl;
            setColor(C_RESET);
            pause();
        } else if (ch == 2) {
            string newp = inputPassword("Enter new password: ");
            if (isWeakPassword(newp)) {
                setColor(C_BLUE);
                cout << "Warning: Weak password! Use at least 6 chars with letters and numbers.\n";
                setColor(C_RESET);
            }
            u->password = newp;
            saveUsers();
            initLoginAttempts();
            setColor(C_GREEN);
            cout << "Password reset successfully for user " << u->username << ".\n";
            setColor(C_RESET);
            pause();
        }
    }
}
void adminManageAccounts() {
    while (1) {
        clearScreen();
        setColor(C_GREEN);
        cout << "\n1. View all accounts\n2. Create account\n3. Edit account\n4. Delete account\n5. Search accounts\n6. Restore account\n7. View/Reset User Password\n0. Back\n";
        int ch = getMenuChoice(0, 7);
        if (ch == 0) break;
        if (ch == 1) viewAllAccounts();
        else if (ch == 2) createAccountByAdmin();
        else if (ch == 3) editAccount();
        else if (ch == 4) deleteAccount();
        else if (ch == 5) searchUsers();
        else if (ch == 6) adminRestoreAccount();
        else if (ch == 7) adminViewOrResetPassword();
    }
}
void adminSubjects() {
    while (1) {
        clearScreen();
        setColor(C_RESET);
        cout << "\n1. View subjects\n2. Add subject\n3. Delete subject\n4. Search subject\n0. Back\n";
        int ch = getMenuChoice(0, 4);
        if (ch == 0) break;
        if (ch == 1) {
            clearScreen();
            for (size_t i = 0; i < subjects.size(); i++) cout << "- " << subjects[i].code << " - " << subjects[i].name << endl;
            pause();
        } else if (ch == 2) {
            string code, sname;
            cout << "Enter subject code: ";
            cin.ignore();
            getline(cin, code);
            if (code == "0") { setColor(C_BLUE); cout << "Cancelled.\n"; setColor(C_RESET); pause(); continue; }
            cout << "Enter subject name: ";
            getline(cin, sname);
            if (sname == "0") { setColor(C_BLUE); cout << "Cancelled.\n"; setColor(C_RESET); pause(); continue; }
            if (code.empty() || sname.empty()) { setColor(C_RED); cout << "Code and name cannot be empty.\n"; setColor(C_RESET); pause(); continue; }
            if (findSubjectByCode(code) || findSubjectByName(sname)) { setColor(C_RED); cout << "Subject code or name already exists!\n"; setColor(C_RESET); pause(); continue; }
            Subject s;
            s.code = code;
            s.name = sname;
            subjects.push_back(s);
            saveSubjects();
            setColor(C_GREEN); cout << "Subject added!\n"; setColor(C_RESET);
            pause();
        } else if (ch == 3) {
            string input;
            cout << "Enter subject code or name to delete (0 to cancel): ";
            cin.ignore();
            getline(cin, input);
            if (input == "0") { setColor(C_BLUE); cout << "Cancelled.\n"; setColor(C_RESET); pause(); continue; }
            Subject* s = findSubject(input);
            if (!s) { setColor(C_RED); cout << "Not found!\n"; setColor(C_RESET); pause(); continue; }
            string codeToRemove = s->code;
            for (vector<Exam>::iterator eit = exams.begin(); eit != exams.end();) {
                if (lower(eit->subject) == lower(codeToRemove)) eit = exams.erase(eit);
                else ++eit;
            }
            for (vector<Grade>::iterator git = grades.begin(); git != grades.end();) {
                if (lower(git->subject) == lower(codeToRemove)) git = grades.erase(git);
                else ++git;
            }
            for (vector<Subject>::iterator it = subjects.begin(); it != subjects.end(); ++it) {
                if (lower(it->code) == lower(codeToRemove)) { subjects.erase(it); break; }
            }
            saveSubjects();
            saveExams();
            saveGrades();
            setColor(C_GREEN); cout << "Subject, related exams and grades deleted.\n"; setColor(C_RESET);
            pause();
        } else if (ch == 4) searchSubjects();
    }
}
void adminCreateExam() {
    if (subjects.empty()) { setColor(C_BLUE); cout << "No subjects available. Add subjects first.\n"; setColor(C_RESET); pause(); return; }
    cout << "Available subjects:\n";
    for (size_t i = 0; i < subjects.size(); i++) {
        cout << (i+1) << ". " << subjects[i].code << " - " << subjects[i].name << endl;
    }
    cout << "\nChoose subject number to create exam for (0 to cancel): ";
    int sel; cin >> sel;
    if (sel <= 0 || sel > (int)subjects.size()) { setColor(C_BLUE); cout << "Cancelled.\n"; setColor(C_RESET); pause(); return; }
    setColor(C_BLUE);
    string sCode = subjects[sel-1].code;
    bool willOverwrite = false;
    if (findExam(sCode)) {
        cout << "An exam for this subject code already exists. Overwrite? (Y/N): ";
        char c; cin >> c;
        setColor(C_BLUE);
        if (c == 'N' || c == 'n') { cout << "Cancelled.\n"; pause(); return; }
        willOverwrite = true;
    }
    Exam ex;
    ex.subject = sCode;
    cin.ignore();
    for (int i = 0; i < 15; ++i) {
        Question q;
        setColor(C_RESET);
        cout << "\nQ" << (i+1) << ": ";
        getline(cin, q.questionText);
        if (q.questionText == "0") {
            setColor(C_BLUE);
            cout << "Exam creation cancelled by user.\n";
            setColor(C_RESET);
            pause();
            return;
        }
        if (q.questionText.empty()) {
            setColor(C_RED);
            cout << "Question cannot be empty. Please re-enter or type 0 to cancel.\n";
            setColor(C_RESET);
            --i;
            continue;
        }
        int n;
        cout << "\nHow many choices?: ";
        if (!(cin >> n)) {
            cin.clear();
            cin.ignore(10000, '\n');
            setColor(C_RED);
            cout << "Invalid number. Cancelling exam creation.\n";
            setColor(C_RESET);
            pause();
            return;
        }
        if (n == 0) {
            setColor(C_BLUE);
            cout << "Exam creation cancelled by user.\n";
            setColor(C_RESET);
            cin.ignore(10000, '\n');
            pause();
            return;
        }
        if (n < 2) {
            setColor(C_RED);
            cout << "There must be at least 2 choices. Please re-enter this question.\n";
            setColor(C_RESET);
            cin.ignore(10000, '\n');
            --i;
            continue;
        }
        cin.ignore();
        q.choices.clear();
        bool cancelled = false;
        for (int j = 0; j < n; ++j) {
            string choice;
            cout << "Choice " << char('A'+j) << ": ";
            getline(cin, choice);
            if (choice == "0") {
                setColor(C_BLUE);
                cout << "Exam creation cancelled by user.\n";
                setColor(C_RESET);
                cancelled = true;
                break;
            }
            if (choice.empty()) {
                setColor(C_RED);
                cout << "Choice cannot be empty. Re-enter this choice.\n";
                setColor(C_RESET);
                --j;
                continue;
            }
            q.choices.push_back(choice);
        }
        if (cancelled) {
            pause();
            return;
        }
        int correctIdx;
        cout << "Correct answer index (0-" << (n-1) << ") : ";
        if (!(cin >> correctIdx)) {
            cin.clear();
            cin.ignore(10000, '\n');
            setColor(C_RED);
            cout << "Invalid index. Cancelling exam creation.\n";
            setColor(C_RESET);
            pause();
            return;
        }
        if (correctIdx < 0 || correctIdx >= n) {
            setColor(C_RED);
            cout << "Correct index out of range. Please re-enter this question.\n";
            setColor(C_RESET);
            cin.ignore(10000, '\n');
            --i;
            continue;
        }
        cin.ignore();
        q.correctIndex = correctIdx;
        ex.questions.push_back(q);
    }
    if (willOverwrite) {
        for (vector<Exam>::iterator it = exams.begin(); it != exams.end();) {
            if (lower(it->subject) == lower(sCode)) it = exams.erase(it);
            else ++it;
        }
    }
    exams.push_back(ex);
    saveExams();
    setColor(C_GREEN); cout << "Exam created!\n"; setColor(C_RESET);
    pause();
}
void deleteExam() {
    if (exams.empty()) {
        setColor(C_BLUE);
        cout << "No exams available to delete.\n";
        setColor(C_RESET);
        pause();
        return;
    }
    cout << "\nExisting Exams:\n";
    for (size_t i = 0; i < exams.size(); i++) {
        Subject* s = findSubjectByCode(exams[i].subject);
        if (s) cout << (i+1) << ". " << exams[i].subject << " - " << s->name << endl;
        else cout << (i+1) << ". " << exams[i].subject << endl;
    }
    cout << "\nChoose exam number to delete (0 to cancel): ";
    int ch;
    cin >> ch;
    if (ch <= 0 || ch > (int)exams.size()) {
    	setColor(C_BLUE);
        cout << "Cancelled.\n";
        pause();
        return;
    }
    string subj = exams[ch-1].subject;
    exams.erase(exams.begin() + (ch-1));
    saveExams();
    setColor(C_GREEN);
    cout << "Deleted exam for subject: " << subj << endl;
    setColor(C_RESET);
    pause();
}
void adminViewGrades() {
    clearScreen();
    if (grades.empty()) {
        setColor(C_BLUE);
        cout << "No grades recorded.\n";
        setColor(C_RESET);
        pause();
        return;
    }
    map<string, vector<Grade> > bySubject;
    for (size_t i = 0; i < grades.size(); ++i) {
        bySubject[grades[i].subject].push_back(grades[i]);
    }
    vector<string> subjCodes;
    for (map<string, vector<Grade> >::iterator it = bySubject.begin(); it != bySubject.end(); ++it) {
        subjCodes.push_back(it->first);
    }
    sort(subjCodes.begin(), subjCodes.end(), ciStringLess);
    for (size_t si = 0; si < subjCodes.size(); ++si) {
        string code = subjCodes[si];
        Subject* s = findSubjectByCode(code);
        string displayName = s ? (code + " - " + s->name) : code;
        setColor(C_GREEN);
        cout << "\n=== Subject: " << displayName << " ===\n";
        setColor(C_BLUE);
        cout << left << setw(15) << "Student" << setw(8) << "Score" << setw(10) << "Average" << setw(10) << "Remark" << endl;
        setColor(C_RESET);
        vector<Grade> &list = bySubject[code];
        sort(list.begin(), list.end(), gradeStudentLess);
        for (size_t j = 0; j < list.size(); ++j) {
            cout << setw(15) << list[j].studentUsername
                 << setw(8) << list[j].score
                 << setw(10) << list[j].average
                 << setw(10) << list[j].remark << endl;
        }
    }
    pause();
}
void adminBackupRestore() {
    while (1) {
        clearScreen();
        setColor(C_GREEN);
        cout << "\n1. Backup Users\n2. Restore Users\n3. Backup Subjects\n4. Restore Subjects\n5. Backup Exams\n6. Restore Exams\n7. Backup Grades\n8. Restore Grades\n0. Back\n";
        int ch = getMenuChoice(0, 8);
        if (ch == 0) break;
        if (ch == 1) backupFile("users.txt");
        else if (ch == 2) restoreFile("users.txt");
        else if (ch == 3) backupFile("subjects.txt");
        else if (ch == 4) restoreFile("subjects.txt");
        else if (ch == 5) backupFile("exams.txt");
        else if (ch == 6) restoreFile("exams.txt");
        else if (ch == 7) backupFile("grades.txt");
        else if (ch == 8) restoreFile("grades.txt");
    }
}
void changeOwnPassword(const string& uname) {
    User* u = findUser(uname);
    if (!u) {
        setColor(C_RED);
        cout << "User not found!\n";
        setColor(C_RESET);
        pause();
        return;
    }
    string current = inputPassword("Enter current password: ");
    if (current != u->password) {
        setColor(C_RED);
        cout << "Incorrect current password!\n";
        setColor(C_RESET);
        pause();
        return;
    }
    string newp = inputPassword("Enter new password: ");
    string confirm = inputPassword("Confirm new password: ");
    if (newp != confirm) {
        setColor(C_RED);
        cout << "Passwords do not match!\n";
        setColor(C_RESET);
        pause();
        return;
    }
    if (isWeakPassword(newp)) {
        setColor(C_BLUE);
        cout << "Warning: Weak password! Use at least 6 chars with letters and numbers.\n";
        setColor(C_RESET);
    }
    u->password = newp;
    saveUsers();
    initLoginAttempts(); 
    setColor(C_GREEN);
    cout << "Password changed successfully.\n";
    setColor(C_RESET);
    pause();
}
bool attemptSwitchAccount() {
    saveUsers(); saveSubjects(); saveExams(); saveGrades(); saveModules();
    string newRole, newUser;
    if (login(newRole, newUser)) {
        showLoginLoading();
        showWelcomeBanner(newUser, newRole);
        if (newRole == "ADMIN") {
            extern void adminMenu(const string&); 
            adminMenu(newUser);
        } else if (newRole == "TEACHER") {
            extern void teacherMenu(const string&);
            teacherMenu(newUser);
        } else if (newRole == "STUDENT") {
            extern void studentMenu(const string&);
            studentMenu(newUser);
        }
        return true;
    }
    return false;
}
void adminMenu(const string& uname) {
    while (1) {
        clearScreen();
        setColor(C_GREEN);
        cout << "========================================================================================================================\n";
        setColor(C_RESET);
        cout << "===========================================    PSYS ADMINISTRATOR     =================================================\n";
        setColor(C_BLUE);
        cout << "========================================================================================================================\n";
        cout << "\nADMIN OPTIONS:";
        setColor(C_GREEN);
        cout << "\n1. Manage Accounts\n2. Manage Subjects\n3. Create Exam\n4. Delete Exam\n5. View Grades\n6. Backup/Restore\n7. Help?\n8. Change Password\n9. Manage Modules\n10. Switch Account\n0. Logout\n";
        int ch = getMenuChoice(0, 10);
        if (ch == 0) {
            saveUsers(); saveSubjects(); saveExams(); saveGrades(); saveModules();
            showLogoutLoading(true);
            return; 
        }
        if (ch == 1) adminManageAccounts();
        else if (ch == 2) adminSubjects();
        else if (ch == 3) adminCreateExam();
        else if (ch == 4) deleteExam();
        else if (ch == 5) adminViewGrades();
        else if (ch == 6) adminBackupRestore();
        else if (ch == 7) showHelp();
        else if (ch == 8) changeOwnPassword(uname);
        else if (ch == 9) adminManageModules();
        else if (ch == 10) {
            if (attemptSwitchAccount()) {
                return;
            }
        }
        saveAll();
    }
}
void teacherViewStudents() {
    clearScreen();
    setColor(C_BLUE);
    cout << left << setw(15) << "\n=====  Students  =====" << endl;
    setColor(C_RESET);
    for (size_t i = 0; i < users.size(); i++)
        if (users[i].role == "STUDENT")
            cout << setw(15) << users[i].username << endl;
    pause();
}
void teacherCreateExam() {
    adminCreateExam();
}
void teacherDeleteExam() {
    deleteExam();
}
void teacherViewGrades() {
    adminViewGrades();
}
void teacherSubjects() {
    adminSubjects();
}
void teacherBackupRestore() {
    adminBackupRestore();
}
void teacherManageModules() {
    adminManageModules();
}
void teacherMenu(const string& uname) {
    while (1) {
        clearScreen();
         setColor(C_GREEN);
        cout << "========================================================================================================================\n";
        setColor(C_RESET);
        cout << "===============================================     TEACHER MENU     ===================================================\n";
        setColor(C_BLUE);
        cout << "========================================================================================================================\n";
        setColor(C_RESET);
        cout <<"\n1. View Students\n2. Create Exam\n3. Delete Exam\n4. View Grades\n5. Manage Subjects\n6. Backup/Restore\n7. Help?\n8. Change Password\n9. Manage Modules\n10. Switch Account\n0. Logout\n";
        int ch = getMenuChoice(0, 10);
        if (ch == 0) {
            saveUsers(); saveSubjects(); saveExams(); saveGrades(); saveModules();
            showLogoutLoading(true);
            return;
        }
        if (ch == 1) teacherViewStudents();
        else if (ch == 2) teacherCreateExam();
        else if (ch == 3) teacherDeleteExam();
        else if (ch == 4) teacherViewGrades();
        else if (ch == 5) teacherSubjects();
        else if (ch == 6) teacherBackupRestore();
        else if (ch == 7) showHelp();
        else if (ch == 8) changeOwnPassword(uname);
        else if (ch == 9) teacherManageModules();
        else if (ch == 10) {
            if (attemptSwitchAccount()) {
                return;
            }
        }
        saveAll();
    }
}
bool hasTakenAlready(const string& user, const string& subjCode) {
    for (size_t i = 0; i < grades.size(); ++i)
        if (grades[i].studentUsername == user && lower(grades[i].subject) == lower(subjCode))
            return true;
    return false;
}
int findGradeIndex(const string& user, const string& subjCode) {
    for (size_t i = 0; i < grades.size(); ++i)
        if (grades[i].studentUsername == user && lower(grades[i].subject) == lower(subjCode))
            return (int)i;
    return -1;
}
void shuffleExam(Exam& ex) {
    random_shuffle(ex.questions.begin(), ex.questions.end());
    for (size_t qi = 0; qi < ex.questions.size(); ++qi) {
        Question &q = ex.questions[qi];
        vector< pair<string, bool> > pairs;
        for (size_t ci = 0; ci < q.choices.size(); ++ci) {
            pairs.push_back(make_pair(q.choices[ci], (int)ci == q.correctIndex));
        }
        random_shuffle(pairs.begin(), pairs.end());
        q.choices.clear();
        q.correctIndex = -1;
        for (size_t ci = 0; ci < pairs.size(); ++ci) {
            q.choices.push_back(pairs[ci].first);
            if (pairs[ci].second) q.correctIndex = (int)ci;
        }
    }
}
const int EXAM_TIME_LIMIT_SECONDS = 15 * 60; 
void studentTakeExam(const string& uname) {
    clearScreen();
    setColor(C_GREEN);
    cout << "\nAvailable subjects:\n";
    setColor(C_RESET);
    for (size_t i = 0; i < subjects.size(); i++)
        cout << i+1 << ". " << subjects[i].code << " - " << subjects[i].name << endl;
    cout << "\nChoose Subject (TYPE 0 to Cancel): ";
    int subch;
    cin >> subch;
    if (subch <= 0 || subch > (int)subjects.size()) return;
    string subjCode = subjects[subch-1].code;
    Exam* exOrig = findExam(subjCode);
    if (!exOrig) { setColor(C_RED); cout << "No exam for this subject.\n"; setColor(C_RESET); pause(); return; }
    Exam ex = *exOrig;
    shuffleExam(ex);
    bool overwrite = false;
    if (hasTakenAlready(uname, subjCode)) {
        cout << "You have already taken this exam. Retake and overwrite previous result? (Y/N): ";
        char resp;
        cin >> resp;
        if (resp == 'N' || resp == 'n') {
        	setColor(C_BLUE);
            cout << "Exam cancelled.\n";
            pause();
            return;
        }
        overwrite = true;
    }
    int score = 0;
    vector<int> ans;
    cin.ignore();
    time_t startTime = time(NULL);
    for (size_t i = 0; i < ex.questions.size(); ++i) {
        time_t now = time(NULL);
        int elapsed = (int)difftime(now, startTime);
        int remaining = EXAM_TIME_LIMIT_SECONDS - elapsed;
        if (remaining <= 0) {
            setColor(C_RED);
            cout << "\nTime's up! The exam time limit has been reached.\n";
            setColor(C_RESET);
            break;
        }
        int remMin = remaining / 60;
        int remSec = remaining % 60;
        cout << "\nRemaining time: " << remMin << "m " << setw(2) << setfill('0') << remSec << "s" << setfill(' ') << endl;
        Question &q = ex.questions[i];
        cout << "\nQ" << (i+1) << ": " << q.questionText << endl;
        for (size_t j = 0; j < q.choices.size(); ++j)
            cout << "  " << char('A'+j) << ". " << q.choices[j] << endl;
        string respStr;
        int idx = -1;
        while (true) {
            now = time(NULL);
            elapsed = (int)difftime(now, startTime);
            remaining = EXAM_TIME_LIMIT_SECONDS - elapsed;
            if (remaining <= 0) {
                setColor(C_RED);
                cout << "\nTime's up! The exam time limit has been reached.\n";
                setColor(C_RESET);
                break;
            }
            cout << "\nYour answer (A-" << char('A'+q.choices.size()-1) << "): ";
            cin >> respStr;
            if (respStr == "0") {
                cout << "Cancel exam? All progress will be lost. Confirm (Y/N): ";
                char c; cin >> c;
                if (c == 'Y' || c == 'y') {
                    setColor(C_BLUE);
                    cout << "Exam cancelled. Returning to menu.\n";
                    setColor(C_RESET);
                    pause();
                    return; 
                } else {
                    cout << "Resuming exam...\n";
                    continue; 
                }
            }
            if (respStr.length() != 1) {
                setColor(C_RED);
                cout << "Invalid input. Enter a single letter between A and " << char('A' + q.choices.size() - 1) << " or type 0 to cancel.\n";
                setColor(C_RESET);
                continue;
            }
            char c = respStr[0];
            if (!isalpha((unsigned char)c)) {
                setColor(C_RED);
                cout << "Invalid input. Enter a letter.\n";
                setColor(C_RESET);
                continue;
            }
            idx = toupper(c) - 'A';
            if (idx < 0 || idx >= (int)q.choices.size()) {
                setColor(C_RED);
                cout << "Choice out of range. Try again.\n";
                setColor(C_RESET);
                continue;
            }
            break; 
        }
        now = time(NULL);
        int elapsedAfter = (int)difftime(now, startTime);
        if (elapsedAfter >= EXAM_TIME_LIMIT_SECONDS) {
            setColor(C_RED);
            cout << "Time's up — exam ended.\n";
            setColor(C_RESET);
            break;
        }
        ans.push_back(idx);
        if (idx == q.correctIndex) {
            setColor(C_GREEN); cout << "Correct!\n"; setColor(C_RESET);
            score++;
        } else {
            setColor(C_RED);
            if (q.correctIndex >= 0 && q.correctIndex < (int)q.choices.size()) {
                cout << "Wrong! The correct answer is "
                     << char('A' + q.correctIndex) << ". "
                     << q.choices[q.correctIndex] << endl;
            } else {
                cout << "Wrong! (correct answer index invalid in exam data)\n";
            }
            setColor(C_RESET);
        }
        cout << endl;
        now = time(NULL);
        elapsed = (int)difftime(now, startTime);
        if (elapsed >= EXAM_TIME_LIMIT_SECONDS) {
            setColor(C_RED);
            cout << "Time's up — exam ended.\n";
            setColor(C_RESET);
            break;
        }
    }
    double avg = score * 100 / 15;
    string remark = (avg >= 75) ? "Passed" : "Failed";
    Grade g;
    g.studentUsername = uname;
    g.subject = subjCode;
    g.score = score;
    g.average = avg;
    g.remark = remark;
    if (hasTakenAlready(uname, subjCode) && !overwrite) {
    } else {
        if (overwrite) {
            int gi = findGradeIndex(uname, subjCode);
            if (gi >= 0) {
                grades[gi] = g;
            } else {
                grades.push_back(g); 
            }
        } else {
            grades.push_back(g);
        }
        saveGrades();
    }
    setColor(C_GREEN);
    cout << "Exam done! Score: " << score << "/15, Average: " << avg << ", Remark: " << remark << endl;
    setColor(C_RESET);
    pause();
}
void studentViewGrades(const string& uname) {
    clearScreen();
    setColor(C_GREEN);
    cout << left << setw(60) << "\nSubject" << setw(8) << "Score" << setw(10) << "Average" << setw(12) << "Remarks" << endl;
    setColor(C_RESET);
    cout << string(90, '-') << endl;
    bool any = false;
    for (size_t i = 0; i < grades.size(); i++) {
        if (grades[i].studentUsername == uname) {
            any = true;
            Subject* s = findSubjectByCode(grades[i].subject);
            string subjDisplay = s ? (s->code + " - " + s->name) : grades[i].subject;
            int avgInt = (int)grades[i].average;
            cout << left << setw(60) << subjDisplay
                 << setw(8) << grades[i].score
                 << setw(10) << avgInt
                 << setw(12) << grades[i].remark << endl;
        }
    }
    if (!any) {
        setColor(C_BLUE);
        cout << "No grades available for user: " << uname << "\n";
        setColor(C_RESET);
    }
    pause();
}
void studentBackupRestore() {
    adminBackupRestore();
}
void studentViewModules(const string& uname) {
    clearScreen();
    if (modules.empty()) {
        setColor(C_BLUE); cout << "No modules available.\n"; setColor(C_RESET); pause(); return;
    }
    setColor(C_RESET);
    cout << "\nAvailable Modules:\n";
    for (size_t i = 0; i < modules.size(); ++i) {
        cout << (i+1) << ". " << modules[i].title << endl;
    }
    cout << "\nChoose module number to read (0 to cancel): ";
    int sel; cin >> sel;
    setColor(C_BLUE);
    if (sel <= 0 || sel > (int)modules.size()) { cout << "Cancelled.\n"; pause(); return; }
    Module &m = modules[sel-1];
    clearScreen();
    setColor(C_GREEN);
    cout << "\n=== " << m.title << " ===\n";
    setColor(C_RESET);
    for (size_t li = 0; li < m.contentLines.size(); ++li)
        cout << m.contentLines[li] << endl;
    pause();
}
void studentMenu(const string& uname) {
    while (1) {
        clearScreen();
        setColor(C_GREEN);
        cout << "\n=======================================";
        setColor(C_RESET);
        cout << "============   STUDENT   ============";
        setColor(C_BLUE);
        cout << "=======================================\n";
        setColor(C_RESET);
        cout << "\n1. Take Exam\n2. View Grades\n3. Help?\n4. Change Password\n5. View Modules\n6. Switch Account\n0. Logout\n";
        int ch = getMenuChoice(0, 6);
        if (ch == 0) {
            saveUsers(); saveSubjects(); saveExams(); saveGrades(); saveModules();
            showLogoutLoading(true);
            return;
        }
        if (ch == 1) studentTakeExam(uname);
        else if (ch == 2) studentViewGrades(uname);
        else if (ch == 3) showHelp();
        else if (ch == 4) changeOwnPassword(uname);
        else if (ch == 5) studentViewModules(uname);
        else if (ch == 6) {
            if (attemptSwitchAccount()) {
                return;
            }
        }
        saveAll();
    }
}
void mainMenu() {
    while (1) {
        clearScreen();                                          //PRINCE + SYSTEM = PSYSTEM
        setColor(C_GREEN);
        cout << "========================================================================================================================\n";
        setColor(C_RESET);
        cout << "===========================================     PSYS ASSESSMENT EXAMINATION     =======================================\n";
        setColor(C_BLUE);
        cout << "========================================================================================================================\n";
        setColor(C_RESET);
        cout << "\n1. REGISTER\n2. LOGIN\n3. HELP?\n0. EXIT\n";
        int ch = getMenuChoice(0, 3);
        if (ch == 0) break;
        if (ch == 1) registerAccount();
        else if (ch == 2) {
            string role, uname;
            if (login(role, uname)) {
                showLoginLoading();
                showWelcomeBanner(uname, role);
                if (role == "ADMIN") adminMenu(uname);
                else if (role == "TEACHER") teacherMenu(uname);
                else if (role == "STUDENT") studentMenu(uname);
            }
        } else if (ch == 3) showHelp();
        saveAll();
    }
}
int main() {
    loadUsers(); loadSubjects(); loadExams(); loadGrades(); loadModules();
    createDefaultAdmin();
    srand((unsigned)time(NULL));
    showStartupLoading();
    mainMenu();
    saveUsers(); saveSubjects(); saveExams(); saveGrades(); saveModules();
    showLogoutLoading();
    return 0;  //BENAVIDEZ, PRINCE ERL ORIGINATED          
}


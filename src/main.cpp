#include <algorithm>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include "../include/Candidate.h"
#include "../include/JobDescription.h"
#include "../include/Matcher.h"
#include "../include/ScreeningManager.h"

using namespace std;
using namespace TalentForge;

const char* COLOR_RESET = "\033[0m";
const char* COLOR_TITLE = "\033[1;36m"; // bright cyan
const char* COLOR_OPTION = "\033[1;33m"; // bright yellow
const char* COLOR_INPUT = "\033[1;32m"; // bright green
const char* COLOR_INFO = "\033[1;35m"; // bright magenta
const char* COLOR_ERROR = "\033[1;31m"; // bright red
const char* COLOR_BORDER = "\033[1;34m"; // bright blue

string colorText(const char* color, const string& text){
    return string(color) + text + COLOR_RESET;
}

string padRight(const string& text, int width){
    if (static_cast<int>(text.size()) >= width) {
        return text.substr(0, width);
    }
    return text + string(width - static_cast<int>(text.size()), ' ');
}

void clearScreen(){
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void waitForEnter(){
    cout << colorText(COLOR_INPUT, "Press Enter to continue...");
    string input;
    getline(cin, input);
}

bool isNumber(const string& text){
    if (text.empty()) {
        return false;
    }
    for (size_t i = 0; i < text.size(); ++i) {
        if (!isdigit(text[i]) && text[i] != '+' && text[i] != '-') {
            return false;
        }
    }
    return true;
}

void printBorder(int width = 70, char fill = '='){
    string border(width, fill);
    cout << colorText(COLOR_BORDER, border) << "\n";
}

void printBoxLine(const string& text, const char* color, int width = 70){
    string content = padRight(text, width - 4);
    cout << colorText(COLOR_BORDER, "| ")
         << colorText(color, content)
         << colorText(COLOR_BORDER, " |") << "\n";
}

void printHeadingBox(const string& heading){
    printBorder();
    printBoxLine(heading, COLOR_TITLE);
    printBoxLine("", COLOR_TITLE);
    printBorder();
}

void printOptionsBox(const vector<string>& options){
    printBorder();
    for (size_t i = 0; i < options.size(); ++i) {
        printBoxLine(options[i], COLOR_OPTION);
        if (i + 1 < options.size()) {
            printBoxLine("", COLOR_OPTION);
        }
    }
    printBorder();
}

void printDataBox(const vector<string>& items){
    printBorder();
    for (size_t i = 0; i < items.size(); ++i) {
        printBoxLine(items[i], COLOR_INFO);
        if (i + 1 < items.size()) {
            printBoxLine("", COLOR_INFO);
        }
    }
    printBorder();
}

void printInputBox(const string& prompt){
    printBorder(70, '-');
    printBoxLine(prompt, COLOR_INPUT);
    printBoxLine("", COLOR_INPUT);
    printBorder(70, '-');
}

string trim(const string& text){
    size_t start = 0;
    while (start < text.size() && isspace(text[start])) {
        ++start;
    }
    size_t end = text.size();
    while (end > start && isspace(text[end - 1])) {
        --end;
    }
    return text.substr(start, end - start);
}

string promptInput(const string& prompt){
    printInputBox(prompt);
    cout << colorText(COLOR_INPUT, "> ");
    string input;
    if (!getline(cin, input)) {
        return string();
    }
    return trim(input);
}

vector<string> splitCSV(const string& text){
    vector<string> parts;
    string token;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == ',') {
            parts.push_back(trim(token));
            token.clear();
        } else {
            token.push_back(text[i]);
        }
    }
    parts.push_back(trim(token));
    return parts;
}

bool isITSkill(const string& skill){
    static const char* itSkills[] = {
        "C++", "Java", "Python", "SQL", "JavaScript", "React", "Node.js",
        "AWS", "Docker", "Kubernetes", "Django", "Machine Learning",
        "Flask", "Spring", "Terraform", "CI/CD", "Microservices",
        "MongoDB", "Pandas", "TypeScript", "Hibernate", "REST",
        "Linux", "Embedded Systems", "Angular", "Vue.js"
    };

    for (size_t i = 0; i < sizeof(itSkills) / sizeof(itSkills[0]); ++i) {
        if (skill == itSkills[i]) {
            return true;
        }
    }
    return false;
}

Candidate* createCandidateFromData(const string& id,
                                   const string& name,
                                   int experience,
                                   const vector<string>& skills){
    bool hasIT = false;
    for (size_t i = 0; i < skills.size(); ++i) {
        if (isITSkill(skills[i])) {
            hasIT = true;
            break;
        }
    }

    Candidate* candidate = hasIT
        ? static_cast<Candidate*>(new ITCandidate(id, name, experience))
        : static_cast<Candidate*>(new NonITCandidate(id, name, experience));

    for (size_t i = 0; i < skills.size(); ++i) {
        if (!skills[i].empty()) {
            candidate->addSkill(skills[i]);
        }
    }
    return candidate;
}

Candidate* parseResumeFile(const string& path){
    ifstream file(path.c_str());
    if (!file.is_open()) {
        return 0;
    }

    string candidateID;
    string name;
    string skillsLine;
    int experience = 0;
    string line;

    while (getline(file, line)) {
        if (line.find("CandidateID:") == 0) {
            candidateID = trim(line.substr(12));
        } else if (line.find("Name:") == 0) {
            name = trim(line.substr(5));
        } else if (line.find("Experience:") == 0) {
            experience = atoi(trim(line.substr(11)).c_str());
        } else if (line.find("Skills:") == 0) {
            skillsLine = trim(line.substr(7));
        }
    }

    if (candidateID.empty() || name.empty()) {
        return 0;
    }
    return createCandidateFromData(candidateID, name, experience,
                                   splitCSV(skillsLine));
}

void saveCandidatesToCSV(const ScreeningManager& manager, const string& path){
    ofstream output(path.c_str());
    if (!output.is_open()) {
        cout << "Unable to write " << path << "\n";
        return;
    }
    output << "candidateID,name,experienceYears,skills\n";
    const vector<Candidate*>& candidates = manager.getCandidates();
    for (size_t i = 0; i < candidates.size(); ++i) {
        candidates[i]->saveToCSV(output);
    }
}

void saveJobsToCSV(const ScreeningManager& manager, const string& path){
    ofstream output(path.c_str());
    if (!output.is_open()) {
        cout << "Unable to write " << path << "\n";
        return;
    }
    output << "jobID,title,requiredSkills\n";
    const vector<JobDescription>& jobs = manager.getJobs();
    for (size_t i = 0; i < jobs.size(); ++i) {
        jobs[i].saveToCSV(output);
    }
}

bool candidateExists(const ScreeningManager& manager, const string& id){
    const vector<Candidate*>& candidates = manager.getCandidates();
    for (size_t i = 0; i < candidates.size(); ++i) {
        if (candidates[i]->getID() == id) {
            return true;
        }
    }
    return false;
}

bool jobExists(const ScreeningManager& manager, const string& id){
    const vector<JobDescription>& jobs = manager.getJobs();
    for (size_t i = 0; i < jobs.size(); ++i) {
        if (jobs[i].getID() == id) {
            return true;
        }
    }
    return false;
}

void loadCandidatesFromCSV(ScreeningManager& manager, const string& path){
    ifstream file(path.c_str());
    if (!file.is_open()) {
        return;
    }

    string line;
    getline(file, line);
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }
        vector<string> tokens = splitCSV(line);
        if (tokens.size() < 4) {
            continue;
        }
        string id = tokens[0];
        string name = tokens[1];
        int experience = atoi(tokens[2].c_str());
        vector<string> skills;
        for (size_t i = 3; i < tokens.size(); ++i) {
            if (!tokens[i].empty()) {
                skills.push_back(tokens[i]);
            }
        }
        Candidate* candidate = createCandidateFromData(id, name, experience, skills);
        if (candidate) {
            manager.addCandidate(candidate);
        }
    }
}

void loadJobsFromCSV(ScreeningManager& manager, const string& path){
    ifstream file(path.c_str());
    if (!file.is_open()) {
        return;
    }
    string line;
    getline(file, line);
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }
        vector<string> tokens = splitCSV(line);
        if (tokens.size() < 3) {
            continue;
        }
        JobDescription job(tokens[0], tokens[1]);
        for (size_t i = 2; i < tokens.size(); ++i) {
            string skillToken = trim(tokens[i]);
            size_t colon = skillToken.find(':');
            if (colon != string::npos) {
                string skill = trim(skillToken.substr(0, colon));
                int weight = atoi(trim(skillToken.substr(colon + 1)).c_str());
                job.addSkill(skill, weight);
            }
        }
        manager.addJob(job);
    }
}

int getChoice(int minValue, int maxValue){
    while (true) {
        ostringstream prompt;
        prompt << "Enter choice (" << minValue << "-" << maxValue << ")";
        string input = promptInput(prompt.str());
        if (input.empty()) {
            return -1;
        }
        int value = atoi(input.c_str());
        if (value >= minValue && value <= maxValue && isNumber(input)) {
            return value;
        }
        cout << colorText(COLOR_ERROR, "Invalid input.\n");
        waitForEnter();
        clearScreen();
    }
}

int getChoiceWithCancel(int minValue, int maxValue){
    while (true) {
        ostringstream prompt;
        prompt << "Enter choice (" << minValue << "-" << maxValue << ", x to go back)";
        string input = promptInput(prompt.str());
        if (input.empty()) {
            return -1;
        }
        if (input == "x" || input == "X") {
            return -1;
        }
        int value = atoi(input.c_str());
        if (value >= minValue && value <= maxValue && isNumber(input)) {
            return value;
        }
        cout << colorText(COLOR_ERROR, "Invalid input.\n");
        waitForEnter();
        clearScreen();
    }
}

void displayJobs(const ScreeningManager& manager){
    const vector<JobDescription>& jobs = manager.getJobs();
    if (jobs.empty()) {
        cout << colorText(COLOR_ERROR, "No job descriptions loaded.\n");
        return;
    }
    vector<string> items;
    for (size_t i = 0; i < jobs.size(); ++i) {
        ostringstream line;
        line << i + 1 << ". " << jobs[i].getID() << " - " << jobs[i].getTitle();
        items.push_back(line.str());
    }
    printDataBox(items);
}

void saveScreeningResults(const vector<MatchResult>& results, const string& jobID){
    const string path = "screening_results.csv";
    bool writeHeader = true;
    ifstream check(path.c_str());
    if (check.is_open() && check.peek() != EOF) {
        writeHeader = false;
    }
    check.close();

    ofstream output(path.c_str(), ios::app);
    if (!output.is_open()) {
        cout << "Unable to open " << path << "\n";
        return;
    }
    if (writeHeader) {
        output << "resultID,candidateID,jobID,score,rank\n";
    }
    for (size_t i = 0; i < results.size(); ++i) {
        output << "R" << (i + 1) << ","
               << results[i].candidate->getID() << ","
               << jobID << ","
               << results[i].score << ","
               << i + 1 << "\n";
    }
}

string nowTimestamp(){
    time_t now = time(0);
    struct tm* localTime = localtime(&now);
    char buffer[32];
    if (localTime != 0) {
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
        return string(buffer);
    }
    return string("0000-00-00 00:00:00");
}

void appendShortlistLog(const vector<MatchResult>& results, const string& jobID){
    ofstream output("shortlist.log", ios::app);
    if (!output.is_open()) {
        cout << "Unable to write shortlist.log\n";
        return;
    }
    output << "[" << nowTimestamp() << "] Shortlist for " << jobID << ": ";
    for (size_t i = 0; i < results.size(); ++i) {
        if (i > 0) {
            output << ", ";
        }
        output << results[i].candidate->getID() << " (" << results[i].score << ")";
    }
    output << "\n";
}

void displayJobDetails(const ScreeningManager& manager){
    const vector<JobDescription>& jobs = manager.getJobs();
    if (jobs.empty()) {
        cout << colorText(COLOR_ERROR, "No job descriptions loaded.\n");
        return;
    }
    vector<string> jobLines;
    for (size_t i = 0; i < jobs.size(); ++i) {
        ostringstream line;
        line << i + 1 << ". " << jobs[i].getID() << " - " << jobs[i].getTitle();
        jobLines.push_back(line.str());
    }
    printDataBox(jobLines);
    int choice = getChoice(1, static_cast<int>(jobs.size()));
    if (choice < 1) {
        return;
    }
    const JobDescription& job = jobs[choice - 1];
    vector<string> details;
    details.push_back(string("Job ID: ") + job.getID());
    details.push_back(string("Title: ") + job.getTitle());
    details.push_back("Required Skills:");
    const map<string, int>& skills = job.getRequiredSkills();
    for (map<string, int>::const_iterator it = skills.begin(); it != skills.end(); ++it) {
        ostringstream weightStream;
        weightStream << it->second;
        details.push_back(string("  - ") + it->first + ": " + weightStream.str());
    }
    printDataBox(details);
}

void addJobDescription(ScreeningManager& manager);
void viewShortlists();

void jobDescriptionManagement(ScreeningManager& manager){
    while (true) {
        clearScreen();
        printHeadingBox("Job Description Management");
        vector<string> menuLines;
        menuLines.push_back("1. View job details");
        menuLines.push_back("2. Add job description");
        menuLines.push_back("3. Back");
        printOptionsBox(menuLines);
        int choice = getChoice(1, 3);
        if (choice == 1) {
            clearScreen();
            printHeadingBox("View Job Details");
            displayJobDetails(manager);
            waitForEnter();
        } else if (choice == 2) {
            clearScreen();
            printHeadingBox("Add Job Description");
            addJobDescription(manager);
            waitForEnter();
        } else {
            break;
        }
    }
}

void displayMatchResults(const vector<MatchResult>& results){
    vector<string> items;
    for (size_t i = 0; i < results.size(); ++i) {
        ostringstream line;
        line << i + 1 << ". " << results[i].candidate->getID()
             << " - " << results[i].candidate->getName()
             << " | Score: " << results[i].score;
        items.push_back(line.str());
    }
    if (!items.empty()) {
        printDataBox(items);
    }
}

vector<MatchResult> shortlistByThreshold(const vector<MatchResult>& results,
                                       double threshold){
    vector<MatchResult> filtered;
    for (size_t i = 0; i < results.size(); ++i) {
        if (results[i].score >= threshold) {
            filtered.push_back(results[i]);
        }
    }
    return filtered;
}

vector<MatchResult> shortlistTopX(const vector<MatchResult>& results,
                                 int topX){
    vector<MatchResult> filtered;
    for (int i = 0; i < topX && i < static_cast<int>(results.size()); ++i) {
        filtered.push_back(results[i]);
    }
    return filtered;
}

void displayScreeningResultsForJob(const string& jobID){
    ifstream file("screening_results.csv");
    if (!file.is_open()) {
        cout << colorText(COLOR_ERROR, "No screening results found.\n");
        return;
    }
    string line;
    if (!getline(file, line)) {
        cout << colorText(COLOR_ERROR, "No screening results found.\n");
        return;
    }
    vector<string> items;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }
        vector<string> tokens = splitCSV(line);
        if (tokens.size() < 5) {
            continue;
        }
        if (tokens[2] == jobID) {
            ostringstream resultLine;
            resultLine << "Candidate " << tokens[1]
                       << " | Score: " << tokens[3]
                       << " | Rank: " << tokens[4];
            items.push_back(resultLine.str());
        }
    }
    if (items.empty()) {
        cout << colorText(COLOR_ERROR, string("No shortlisted candidates found for job ") + jobID + ".\n");
    } else {
        printHeadingBox(string("Shortlisted candidates for ") + jobID);
        printDataBox(items);
    }
}

bool removeCandidateFromScreeningResults(const string& jobID,
                                         const string& candidateID){
    const string path = "screening_results.csv";
    ifstream in(path.c_str());
    if (!in.is_open()) {
        return false;
    }
    string header;
    vector<string> keptLines;
    bool found = false;
    if (getline(in, header)) {
        keptLines.push_back(header);
    }
    string line;
    while (getline(in, line)) {
        string trimmed = trim(line);
        if (trimmed.empty()) {
            continue;
        }
        vector<string> tokens = splitCSV(trimmed);
        if (tokens.size() < 5) {
            keptLines.push_back(trimmed);
            continue;
        }
        if (tokens[2] == jobID && tokens[1] == candidateID) {
            found = true;
            continue;
        }
        keptLines.push_back(trimmed);
    }
    in.close();

    if (!found) {
        return false;
    }

    ofstream out(path.c_str());
    if (!out.is_open()) {
        return false;
    }
    for (size_t i = 0; i < keptLines.size(); ++i) {
        out << keptLines[i] << "\n";
    }
    out.close();

    ofstream logFile("shortlist.log", ios::app);
    if (logFile.is_open()) {
        logFile << "[" << nowTimestamp() << "] Removed candidate "
                << candidateID << " from shortlist for " << jobID << "\n";
    }
    return true;
}

void manageShortlists(const ScreeningManager& manager){
    clearScreen();
    const vector<JobDescription>& jobs = manager.getJobs();
    if (jobs.empty()) {
        cout << "No jobs available to manage.\n";
        return;
    }
    displayJobs(manager);
    int choice = getChoiceWithCancel(1, static_cast<int>(jobs.size()));
    if (choice < 1) {
        return;
    }
    const string jobID = jobs[choice - 1].getID();
    displayScreeningResultsForJob(jobID);
    string candidateID = promptInput("Enter candidate ID to remove from shortlist (x to cancel):");
    if (candidateID.empty() || candidateID == "x" || candidateID == "X") {
        return;
    }
    if (candidateID.empty() || candidateID == "x" || candidateID == "X") {
        return;
    }
    if (removeCandidateFromScreeningResults(jobID, candidateID)) {
        cout << "Candidate " << candidateID << " removed from shortlist for "
             << jobID << ".\n";
    } else {
        cout << "Candidate " << candidateID << " is not shortlisted for "
             << jobID << ".\n";
    }
}

void viewManageShortlists(const ScreeningManager& manager){
    while (true) {
        clearScreen();
        printHeadingBox("Shortlist View & Manage");
        vector<string> menuLines;
        menuLines.push_back("1. View shortlist history");
        menuLines.push_back("2. Manage shortlist entries");
        menuLines.push_back("3. Back");
        printOptionsBox(menuLines);
        int choice = getChoice(1, 3);
        if (choice == 1) {
            clearScreen();
            printHeadingBox("Shortlist History");
            viewShortlists();
            waitForEnter();
        } else if (choice == 2) {
            clearScreen();
            printHeadingBox("Manage Shortlist Entries");
            manageShortlists(manager);
            waitForEnter();
        } else {
            break;
        }
    }
}

void ingestResumes(ScreeningManager& manager){
    clearScreen();
    printHeadingBox("Resume Ingestion & Parsing");
    DIR* dir = opendir("resumes");
    if (dir == 0) {
        vector<string> errorLines;
        errorLines.push_back("Unable to open resumes directory.");
        printDataBox(errorLines);
        return;
    }

    struct dirent* entry;
    bool anyNew = false;
    vector<string> logLines;
    while ((entry = readdir(dir)) != 0) {
        string filename = entry->d_name;
        if (filename == "." || filename == "..") {
            continue;
        }
        if (filename.size() < 5 || filename.substr(filename.size() - 4) != ".txt") {
            continue;
        }
        string path = string("resumes/") + filename;
        Candidate* candidate = parseResumeFile(path);
        if (!candidate) {
            logLines.push_back(string("Skipped invalid resume: ") + filename);
            continue;
        }
        if (candidateExists(manager, candidate->getID())) {
            logLines.push_back(string("Candidate ") + candidate->getID() + " already exists; skipping.");
            delete candidate;
            continue;
        }
        manager.addCandidate(candidate);
        logLines.push_back(string("Imported ") + candidate->getName() + " from " + filename);
        anyNew = true;
    }
    closedir(dir);

    if (anyNew) {
        saveCandidatesToCSV(manager, "candidates.csv");
        logLines.push_back("Saved updated candidate list.");
    } else if (logLines.empty()) {
        logLines.push_back("No resumes found in ./resumes directory.");
    }
    printDataBox(logLines);
}

void addJobDescription(ScreeningManager& manager){
    string jobID = promptInput("Enter job ID:");
    if (jobID.empty()) {
        cout << colorText(COLOR_ERROR, "Job ID is required.\n");
        return;
    }
    string title = promptInput("Enter job title:");
    if (title.empty()) {
        cout << colorText(COLOR_ERROR, "Job title is required.\n");
        return;
    }
    if (jobExists(manager, jobID)) {
        cout << colorText(COLOR_ERROR, string("Job ") + jobID + " already exists.\n");
        return;
    }
    JobDescription job(jobID, title);
    while (true) {
        string line = promptInput("Enter skill and weight (Skill:Weight), blank to finish:");
        if (line.empty()) {
            break;
        }
        line = trim(line);
        if (line.empty()) {
            break;
        }
        size_t colon = line.find(':');
        if (colon == string::npos) {
            cout << "Invalid format.\n";
            continue;
        }
        string skill = trim(line.substr(0, colon));
        int weight = atoi(trim(line.substr(colon + 1)).c_str());
        if (skill.empty() || weight <= 0) {
            cout << "Invalid skill or weight.\n";
            continue;
        }
        job.addSkill(skill, weight);
    }
    manager.addJob(job);
    saveJobsToCSV(manager, "job_descriptions.csv");
    cout << "Saved job description " << jobID << ".\n";
}

void screenAndRankCandidates(ScreeningManager& manager){
    clearScreen();
    const vector<JobDescription>& jobs = manager.getJobs();
    if (jobs.empty()) {
        cout << "No jobs available.\n";
        return;
    }
    if (manager.getCandidates().empty()) {
        cout << "No candidates loaded.\n";
        return;
    }
    displayJobs(manager);
    int choice = getChoice(1, static_cast<int>(jobs.size()));
    if (choice < 1) {
        return;
    }
    const JobDescription& selectedJob = jobs[choice - 1];
    vector<MatchResult> allResults = manager.screenJob(selectedJob.getID());
    if (allResults.empty()) {
        cout << "No matches found for " << selectedJob.getID() << ".\n";
        return;
    }

    vector<string> screenLines;
    screenLines.push_back("Screen & Rank Candidates");
    screenLines.push_back("1. Shortlist based on score threshold");
    screenLines.push_back("2. Shortlist top X candidates");
    screenLines.push_back("3. Cancel");
    printOptionsBox(screenLines);
    int method = getChoice(1, 3);
    if (method == 3) {
        return;
    }

    vector<MatchResult> results;
    if (method == 1) {
        string input = promptInput("Enter threshold score:");
        if (input.empty()) {
            return;
        }
        double threshold = atof(input.c_str());
        results = shortlistByThreshold(allResults, threshold);
        if (results.empty()) {
            cout << "No candidates meet the threshold.\n";
            return;
        }
    } else {
        string input = promptInput("Enter number of top candidates:");
        if (input.empty()) {
            return;
        }
        int topX = atoi(input.c_str());
        if (topX <= 0) {
            cout << "Invalid number.\n";
            return;
        }
        if (topX > static_cast<int>(allResults.size())) {
            cout << "Candidate number exceed\n";
            return;
        }
        results = shortlistTopX(allResults, topX);
    }

    cout << "Shortlist for " << selectedJob.getTitle() << "\n";
    displayMatchResults(results);
    saveScreeningResults(results, selectedJob.getID());
    appendShortlistLog(results, selectedJob.getID());
    cout << "Saved screening results and shortlist.\n";
}

void viewShortlists(){
    ifstream file("shortlist.log");
    if (!file.is_open()) {
        cout << colorText(COLOR_ERROR, "No shortlist history.\n");
        return;
    }
    vector<string> items;
    string line;
    while (getline(file, line)) {
        items.push_back(line);
    }
    if (items.empty()) {
        cout << colorText(COLOR_ERROR, "No shortlist history.\n");
        return;
    }
    printHeadingBox("Shortlist history");
    printDataBox(items);
}

struct SkillCountGreater
{
    bool operator()(const pair<string, int>& a,
                    const pair<string, int>& b) const
    {
        return a.second > b.second;
    }
};

void analyticsAndReports(const ScreeningManager& manager){
    const vector<Candidate*>& candidates = manager.getCandidates();
    const vector<JobDescription>& jobs = manager.getJobs();
    cout << "Analytics & Reports\n";
    cout << "Candidates loaded: " << candidates.size() << "\n";
    cout << "Jobs loaded: " << jobs.size() << "\n";
    map<string, int> skillFrequency;
    for (size_t i = 0; i < candidates.size(); ++i) {
        const set<string>& skills = candidates[i]->getSkills();
        for (set<string>::const_iterator it = skills.begin(); it != skills.end(); ++it) {
            skillFrequency[*it] += 1;
        }
    }
    if (skillFrequency.empty()) {
        cout << "No skill data.\n";
        return;
    }
    vector<pair<string, int> > report;
    for (map<string, int>::const_iterator it = skillFrequency.begin();
         it != skillFrequency.end(); ++it) {
        report.push_back(*it);
    }
    sort(report.begin(), report.end(), SkillCountGreater());
    cout << "Top skills:\n";
    size_t limit = report.size() < 5 ? report.size() : 5;
    int total = 0;
    for (size_t i = 0; i < limit; ++i) {
        cout << report[i].first << ": " << report[i].second << "\n";
        total += report[i].second;
    }
    for (size_t i = limit; i < report.size(); ++i) {
        total += report[i].second;
    }
    cout << "Total skill mentions: " << total << "\n";
}

void backupSystem(){
    cout << "1. Create backup\n";
    cout << "2. Restore backup\n";
    cout << "3. Cancel\n";
    int choice = getChoice(1, 3);
    if (choice != 1 && choice != 2) {
        return;
    }
    if (choice == 1) {
        ifstream in1("candidates.csv");
        ifstream in2("job_descriptions.csv");
        if (!in1.is_open() || !in2.is_open()) {
            cout << "Missing data files.\n";
            return;
        }
        ofstream out("backup.dat");
        if (!out.is_open()) {
            cout << "Unable to write backup.dat\n";
            return;
        }
        string line;
        out << "---CANDIDATES---\n";
        while (getline(in1, line)) {
            out << line << "\n";
        }
        out << "---JOBS---\n";
        while (getline(in2, line)) {
            out << line << "\n";
        }
        cout << "Backup saved.\n";
        return;
    }
    ifstream in("backup.dat");
    if (!in.is_open()) {
        cout << "Backup not found.\n";
        return;
    }
    ofstream out1("candidates.csv");
    ofstream out2("job_descriptions.csv");
    if (!out1.is_open() || !out2.is_open()) {
        cout << "Unable to restore files.\n";
        return;
    }
    string line;
    bool writeCandidates = false;
    bool writeJobs = false;
    while (getline(in, line)) {
        if (line == "---CANDIDATES---") {
            writeCandidates = true;
            writeJobs = false;
            continue;
        }
        if (line == "---JOBS---") {
            writeCandidates = false;
            writeJobs = true;
            continue;
        }
        if (writeCandidates) {
            out1 << line << "\n";
        } else if (writeJobs) {
            out2 << line << "\n";
        }
    }
    cout << "Data restored. Restart to reload.\n";
}

int main(){
    ScreeningManager manager;
    cout << "TalentForge - Intelligent Resume Screening Platform\n";
    loadCandidatesFromCSV(manager, "candidates.csv");
    loadJobsFromCSV(manager, "job_descriptions.csv");
    while (true) {
        clearScreen();
        printHeadingBox("TalentForge - Intelligent Resume Screening Platform");
        vector<string> menuLines;
        menuLines.push_back("1. Resume Ingestion & Parsing");
        menuLines.push_back("2. Job Description Management");
        menuLines.push_back("3. Screen & Rank Candidates");
        menuLines.push_back("4. View & Manage Shortlists");
        menuLines.push_back("5. Analytics & Reports");
        menuLines.push_back("6. System Backup & Restore");
        menuLines.push_back("7. Exit");
        printOptionsBox(menuLines);
        int choice = getChoice(1, 7);
        if (choice < 0) {
            break;
        }
        if (choice == 1) {
            ingestResumes(manager);
            waitForEnter();
        } else if (choice == 2) {
            jobDescriptionManagement(manager);
        } else if (choice == 3) {
            screenAndRankCandidates(manager);
        } else if (choice == 4) {
            viewManageShortlists(manager);
        } else if (choice == 5) {
            analyticsAndReports(manager);
        } else if (choice == 6) {
            backupSystem();
        } else if (choice == 7) {
            cout << "Exiting TalentForge.\n";
            break;
        }
    }

    const vector<Candidate*>& candidates = manager.getCandidates();
    for (size_t i = 0; i < candidates.size(); ++i) {
        delete candidates[i];
    }
    return 0;
}

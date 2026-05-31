#include <algorithm>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstdlib>
#include "../include/Candidate.h"
#include "../include/JobDescription.h"
#include "../include/Matcher.h"
#include "../include/ScreeningManager.h"

using namespace std;
using namespace TalentForge;

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
    string input;
    while (true) {
        cout << "Enter choice (" << minValue << "-" << maxValue << "): ";
        if (!getline(cin, input)) {
            return -1;
        }
        int value = atoi(trim(input).c_str());
        if (value >= minValue && value <= maxValue) {
            return value;
        }
        cout << "Invalid choice.\n";
    }
}

void displayJobs(const ScreeningManager& manager){
    const vector<JobDescription>& jobs = manager.getJobs();
    if (jobs.empty()) {
        cout << "No job descriptions loaded.\n";
        return;
    }
    for (size_t i = 0; i < jobs.size(); ++i) {
        cout << i + 1 << ". " << jobs[i].getID() << " - " << jobs[i].getTitle() << "\n";
    }
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
    for (size_t i = 0; i < results.size() && i < 5; ++i) {
        if (i > 0) {
            output << ", ";
        }
        output << results[i].candidate->getID() << " (" << results[i].score << ")";
    }
    output << "\n";
}

void ingestResumes(ScreeningManager& manager){
    DIR* dir = opendir("resumes");
    if (dir == 0) {
        cout << "Unable to open resumes directory.\n";
        return;
    }
    struct dirent* entry;
    bool anyNew = false;
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
            cout << "Skipped invalid resume: " << filename << "\n";
            continue;
        }
        if (candidateExists(manager, candidate->getID())) {
            cout << "Candidate " << candidate->getID() << " already exists; skipping.\n";
            delete candidate;
            continue;
        }
        manager.addCandidate(candidate);
        cout << "Imported " << candidate->getName() << " from " << filename << "\n";
        anyNew = true;
    }
    closedir(dir);
    if (anyNew) {
        saveCandidatesToCSV(manager, "candidates.csv");
        cout << "Saved updated candidate list.\n";
    }
}

void addJobDescription(ScreeningManager& manager){
    string jobID;
    string title;
    cout << "Enter job ID: ";
    if (!getline(cin, jobID) || trim(jobID).empty()) {
        cout << "Job ID is required.\n";
        return;
    }
    cout << "Enter job title: ";
    if (!getline(cin, title) || trim(title).empty()) {
        cout << "Job title is required.\n";
        return;
    }
    if (jobExists(manager, jobID)) {
        cout << "Job " << jobID << " already exists.\n";
        return;
    }
    JobDescription job(jobID, title);
    while (true) {
        string line;
        cout << "Enter skill and weight (Skill:Weight), blank to finish: ";
        if (!getline(cin, line)) {
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
    vector<MatchResult> results = manager.screenJob(selectedJob.getID());
    if (results.empty()) {
        cout << "No matches found for " << selectedJob.getID() << ".\n";
        return;
    }
    cout << "Shortlist for " << selectedJob.getTitle() << "\n";
    for (size_t i = 0; i < results.size(); ++i) {
        cout << i + 1 << ". " << results[i].candidate->getID()
             << " - " << results[i].candidate->getName()
             << " | Score: " << results[i].score << "\n";
    }
    saveScreeningResults(results, selectedJob.getID());
    appendShortlistLog(results, selectedJob.getID());
    cout << "Saved screening results and shortlist.\n";
}

void viewShortlists(){
    ifstream file("shortlist.log");
    if (!file.is_open()) {
        cout << "No shortlist history.\n";
        return;
    }
    cout << "Shortlist history:\n";
    string line;
    while (getline(file, line)) {
        cout << line << "\n";
    }
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
        cout << "\n============================================================\n";
        cout << "1. Resume Ingestion & Parsing\n";
        cout << "2. Job Description Management\n";
        cout << "3. Screen & Rank Candidates\n";
        cout << "4. View & Manage Shortlists\n";
        cout << "5. Analytics & Reports\n";
        cout << "6. System Backup & Restore\n";
        cout << "7. Exit\n";
        int choice = getChoice(1, 7);
        if (choice < 0) {
            break;
        }
        if (choice == 1) {
            ingestResumes(manager);
        } else if (choice == 2) {
            addJobDescription(manager);
        } else if (choice == 3) {
            screenAndRankCandidates(manager);
        } else if (choice == 4) {
            viewShortlists();
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

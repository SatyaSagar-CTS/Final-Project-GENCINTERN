#include "../include/ScreeningManager.h"
#include <iostream>

using namespace std;
using namespace TalentForge;

ScreeningManager::ScreeningManager()
{
}

ScreeningManager::~ScreeningManager()
{
}

void ScreeningManager::addCandidate(Candidate* candidate)
{
    if (candidate != 0) {
        candidates.push_back(candidate);
    }
}

void ScreeningManager::addJob(const JobDescription& job)
{
    jobs.push_back(job);
}

vector<MatchResult> ScreeningManager::screenJob(const string& jobID) const
{
    for (vector<JobDescription>::const_iterator it = jobs.begin(); it != jobs.end(); ++it) {
        if (it->getID() == jobID) {
            return Matcher::rankCandidates(candidates, *it);
        }
    }

    return vector<MatchResult>();
}

void ScreeningManager::displayJobMatches(const string& jobID) const
{
    vector<MatchResult> results = screenJob(jobID);

    if (results.empty()) {
        cout << "No matches found for job ID: " << jobID << "\n";
        return;
    }

    cout << "Matches for job ID " << jobID << ":\n";
    for (vector<MatchResult>::const_iterator it = results.begin();
         it != results.end(); ++it) {
        cout << "Candidate " << it->candidate->getID()
             << " (" << it->candidate->getName() << ")"
             << " - Score: " << it->score << "\n";
    }
}

const vector<Candidate*>& ScreeningManager::getCandidates() const
{
    return candidates;
}

const vector<JobDescription>& ScreeningManager::getJobs() const
{
    return jobs;
}

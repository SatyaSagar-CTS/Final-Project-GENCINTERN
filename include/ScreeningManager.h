#ifndef SCREENINGMANAGER_H
#define SCREENINGMANAGER_H

#include <vector>
#include <string>
#include "Candidate.h"
#include "JobDescription.h"
#include "Matcher.h"
using namespace std;

namespace TalentForge {

class ScreeningManager
{
private:
    vector<Candidate*> candidates;
    vector<JobDescription> jobs;

public:
    ScreeningManager();
    ~ScreeningManager();

    void addCandidate(Candidate* candidate);
    void addJob(const JobDescription& job);

    vector<MatchResult> screenJob(const string& jobID) const;
    void displayJobMatches(const string& jobID) const;

    const vector<Candidate*>& getCandidates() const;
    const vector<JobDescription>& getJobs() const;
};

} // namespace TalentForge

#endif // SCREENINGMANAGER_H

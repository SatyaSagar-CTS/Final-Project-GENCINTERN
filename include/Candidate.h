#ifndef CANDIDATE_H
#define CANDIDATE_H

#include <string>
#include <set>
#include <iostream>
using namespace std;

namespace TalentForge {

class JobDescription;

class Candidate
{
protected:
    string candidateID;
    string name;
    set<string> skills;
    int experienceYears;
    double score;

public:
    Candidate(const string& id,
              const string& candidateName,
              int experience);
    virtual ~Candidate();

    const string& getID() const;
    const string& getName() const;
    int getExperience() const;
    double getScore() const;
    const set<string>& getSkills() const;

    void setScore(double newScore);
    void addSkill(const string& skill);

    virtual double calculateScore(const JobDescription& job) = 0;
    virtual void display() const;
    virtual void saveToCSV(ostream& output) const;
};

class ITCandidate : public Candidate
{
public:
    ITCandidate(const string& id,
                const string& candidateName,
                int experience);

    virtual double calculateScore(const JobDescription& job);
};

class NonITCandidate : public Candidate
{
public:
    NonITCandidate(const string& id,
                   const string& candidateName,
                   int experience);

    virtual double calculateScore(const JobDescription& job);
};

} // namespace TalentForge

#endif // CANDIDATE_H
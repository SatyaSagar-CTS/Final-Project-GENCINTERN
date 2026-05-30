#include "../include/Candidate.h"
#include "../include/JobDescription.h"

using namespace std;
using namespace TalentForge;

Candidate::Candidate(const string& id,
                         const string& candidateName,
                         int experience)
{
    candidateID = id;
    name = candidateName;
    experienceYears = experience;
    score = 0.0;
}

Candidate::~Candidate()
{
}

const string& Candidate::getID() const
{
    return candidateID;
}

const string& Candidate::getName() const
{
    return name;
}

int Candidate::getExperience() const
{
    return experienceYears;
}

double Candidate::getScore() const
{
    return score;
}

const set<string>& Candidate::getSkills() const
{
    return skills;
}

void Candidate::setScore(double newScore)
{
    score = newScore;
}

void Candidate::addSkill(const string& skill)
{
    skills.insert(skill);
}

void Candidate::display() const
{
    cout << "Candidate ID: " << candidateID << "\n";
    cout << "Name: " << name << "\n";
    cout << "Experience: " << experienceYears << " years\n";
    cout << "Skills: ";

    for (set<string>::const_iterator it = skills.begin(); it != skills.end(); ++it) {
        if (it != skills.begin()) {
            cout << ", ";
        }
        cout << *it;
    }

    cout << "\nScore: " << score << "\n";
}

void Candidate::saveToCSV(ostream& output) const
{
    output << candidateID << "," << name << "," << experienceYears;

    for (set<string>::const_iterator it = skills.begin(); it != skills.end(); ++it) {
        output << "," << *it;
    }

    output << "\n";
}

ITCandidate::ITCandidate(const string& id,
                             const string& candidateName,
                             int experience)
    : Candidate(id, candidateName, experience)
{
}

double ITCandidate::calculateScore(const JobDescription& job)
{
    double scoreFromSkills = 0.0;
    double missingPenalty = 0.0;
    set<string>::const_iterator it;

    for (it = skills.begin(); it != skills.end(); ++it) {
        if (job.hasSkill(*it)) {
            scoreFromSkills += job.getSkillWeight(*it);
        }
    }

    set<string> missing;
    job.getMissingSkills(skills, missing);
    for (it = missing.begin(); it != missing.end(); ++it) {
        missingPenalty += static_cast<double>(job.getSkillWeight(*it));
    }

    double experienceBonus = static_cast<double>(experienceYears) * 2.0;
    double result = scoreFromSkills + experienceBonus - missingPenalty * 0.5;

    score = result;
    return score;
}

NonITCandidate::NonITCandidate(const string& id,
                                   const string& candidateName,
                                   int experience)
    : Candidate(id, candidateName, experience)
{
}

double NonITCandidate::calculateScore(const JobDescription& job)
{
    double scoreFromSkills = 0.0;
    double missingPenalty = 0.0;
    set<string>::const_iterator it;

    for (it = skills.begin(); it != skills.end(); ++it) {
        if (job.hasSkill(*it)) {
            scoreFromSkills += job.getSkillWeight(*it) * 0.9;
        }
    }

    set<string> missing;
    job.getMissingSkills(skills, missing);
    for (it = missing.begin(); it != missing.end(); ++it) {
        missingPenalty += static_cast<double>(job.getSkillWeight(*it));
    }

    double experienceBonus = static_cast<double>(experienceYears) * 1.5;
    double result = scoreFromSkills + experienceBonus - missingPenalty * 0.5;

    score = result;
    return score;
}

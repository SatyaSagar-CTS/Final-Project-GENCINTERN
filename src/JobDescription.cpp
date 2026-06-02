#include "../include/JobDescription.h"

using namespace std;
using namespace TalentForge;

JobDescription::JobDescription()
{
    jobID = "";
    title = "";
}

JobDescription::JobDescription(const string& id, const string& jobTitle)
{
    jobID = id;
    title = jobTitle;
}

const string& JobDescription::getID() const
{
    return jobID;
}

const string& JobDescription::getTitle() const
{
    return title;
}

void JobDescription::setID(const string& id)
{
    jobID = id;
}

void JobDescription::setTitle(const string& jobTitle)
{
    title = jobTitle;
}

void JobDescription::addSkill(const string& skill, int weight)
{
    requiredSkills[skill] = weight;
}

bool JobDescription::hasSkill(const string& skill) const
{
    return requiredSkills.find(skill) != requiredSkills.end();
}

int JobDescription::getSkillWeight(const string& skill) const
{
    map<string, int>::const_iterator it = requiredSkills.find(skill);
    if (it != requiredSkills.end()) {
        return it->second;
    }
    return 0;
}

int JobDescription::getTotalWeight() const
{
    int total = 0;
    for (map<string, int>::const_iterator it = requiredSkills.begin();
         it != requiredSkills.end(); ++it) {
        total += it->second;
    }
    return total;
}

const map<string, int>& JobDescription::getRequiredSkills() const
{
    return requiredSkills;
}

void JobDescription::getMissingSkills(const set<string>& candidateSkills,
                                      set<string>& missingSkills) const
{
    missingSkills.clear();
    for (map<string, int>::const_iterator it = requiredSkills.begin();
         it != requiredSkills.end(); ++it) {
        if (candidateSkills.find(it->first) == candidateSkills.end()) {
            missingSkills.insert(it->first);
        }
    }
}

void JobDescription::saveToCSV(ostream& output) const
{
    output << jobID << "," << title;
    for (map<string, int>::const_iterator it = requiredSkills.begin();
         it != requiredSkills.end(); ++it) {
        output << "," << it->first << ":" << it->second;
    }
    output << "\n";
}

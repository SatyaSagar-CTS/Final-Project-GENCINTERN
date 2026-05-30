#ifndef JOBDESCRIPTION_H
#define JOBDESCRIPTION_H

#include <string>
#include <map>
#include <set>
#include <iostream>
using namespace std;

namespace TalentForge {

class JobDescription
{
private:
    string jobID;
    string title;
    map<string, int> requiredSkills;

public:
    JobDescription();
    JobDescription(const string& id, const string& jobTitle);

    const string& getID() const;
    const string& getTitle() const;

    void setID(const string& id);
    void setTitle(const string& jobTitle);

    void addSkill(const string& skill, int weight);
    bool hasSkill(const string& skill) const;
    int getSkillWeight(const string& skill) const;
    int getTotalWeight() const;

    void getMissingSkills(const set<string>& candidateSkills,
                          set<string>& missingSkills) const;

    void saveToCSV(ostream& output) const;
};

} // namespace TalentForge

#endif // JOBDESCRIPTION_H
#ifndef MATCHER_H
#define MATCHER_H

#include <vector>
#include "Candidate.h"
#include "JobDescription.h"
using namespace std;

namespace TalentForge {

struct MatchResult
{
    const Candidate* candidate;
    double score;
};

class Matcher
{
public:
    static MatchResult match(Candidate& candidate, const JobDescription& job);
    static vector<MatchResult> rankCandidates(const vector<Candidate*>& candidates,
                                             const JobDescription& job);
};

} // namespace TalentForge

#endif // MATCHER_H

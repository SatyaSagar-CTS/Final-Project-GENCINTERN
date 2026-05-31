#include "../include/Matcher.h"
#include <algorithm>

using namespace std;
using namespace TalentForge;

struct MatchResultComparator
{
    bool operator()(const MatchResult& a, const MatchResult& b) const
    {
        return a.score > b.score;
    }
};

MatchResult Matcher::match(Candidate& candidate, const JobDescription& job)
{
    MatchResult result;
    result.candidate = &candidate;
    result.score = candidate.calculateScore(job);
    return result;
}

vector<MatchResult> Matcher::rankCandidates(const vector<Candidate*>& candidates,
                                            const JobDescription& job)
{
    vector<MatchResult> results;
    results.reserve(candidates.size());

    for (vector<Candidate*>::const_iterator it = candidates.begin();
         it != candidates.end(); ++it) {
        if (*it != 0) {
            results.push_back(match(**it, job));
        }
    }

    sort(results.begin(), results.end(), MatchResultComparator());

    return results;
}

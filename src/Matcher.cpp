#include "../include/Matcher.h"
#include <algorithm>

using namespace std;
using namespace TalentForge;

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
        if (*it != nullptr) {
            results.push_back(match(**it, job));
        }
    }

    sort(results.begin(), results.end(),
         [](const MatchResult& a, const MatchResult& b) {
             return a.score > b.score;
         });

    return results;
}

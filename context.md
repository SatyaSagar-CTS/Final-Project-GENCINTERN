# Project Context Summary

## Project
- TalentForge ATS screening platform
- Workspace root: `/workspaces/Final-Project-GENCINTERN`

## Key files
- `include/Candidate.h`
- `include/JobDescription.h`
- `src/Candidate.cpp`
- `src/JobDescription.cpp`
- `src/main.cpp`

## Current style decisions
- Header files currently include `using namespace std;`.
- Implementation files also use `using namespace std;` for convenience.
- The preferred reference style in this project is now `const string&` and `const set<string>&`.

## Recent changes
- Updated header and source files to use `const T&` style consistently.
- Clarified that `std::` vs `using namespace std;` in headers affects all including translation units.
- Confirmed that `.h` and `.hpp` are naming conventions only, with no compiler-level difference.

## Notes for future sessions
- If you want the AI to keep project history, reopen this file or paste relevant sections when starting a new chat.
- A written summary can help preserve project decisions when conversation context is lost.

## AI internal context
- Maintain this file as the canonical project summary for the workspace.
- Store project-relevant decisions, style choices, and recent edits here for future reference.
- Use this file to recover important context when the conversation history becomes too long.

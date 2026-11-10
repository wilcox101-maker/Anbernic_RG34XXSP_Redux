## Summary
<!-- What changed and why. Link related notes or docs. -->

## Type of change
- [ ] Documentation
- [ ] Frontend (F0–F8)
- [ ] Kernel / Phase 1 or Phase 2
- [ ] Tooling
- [ ] Notes / status
- [ ] Other

## Checklist
- [ ] Work is on a `grok/` or `claude/` branch (not pushed straight to main)
- [ ] If a note event occurred, the note and `notes/STATUS.md` are updated in this PR
- [ ] `notes/ACTIVITY.md` line was appended on main (or will be)
- [ ] No Anbernic binaries, ROMs, or BIOS files are included
- [ ] No automatic partition formatting
- [ ] Nothing writes to the Roms partition (`/mnt/mmc`)
- [ ] `python tools/check_rules.py` passes locally
- [ ] Host tools (if any) are Python 3 stdlib only and run on Windows
- [ ] Unverified hardware claims are explicitly marked

## Evidence (required for review)
- Head commit:
- `git diff --stat` (paste):
- Build + test output (paste, or "CI green" with the run link):
- For each review item: addressed in `<file>` (`<commit>`)

## Test plan
- [ ] <!-- How to verify. Note anything not yet tested on real hardware. -->

## Reviewers
- [ ] Claude (if frontend or shared docs)
- [ ] Dale (phase-gate or sign-off required)

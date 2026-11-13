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
- [ ] Work is on a `claude/` branch (or `grok/` when Grok is active) (not pushed straight to main)
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

## Not verified on device
<!-- One bullet per thing Dale must check on the RG34XXSP. Merging copies these into the
     "Device test queue" issue automatically. Write "- none" for docs/CI-only PRs. -->
- 

## Test plan
- [ ] <!-- How to verify. Note anything not yet tested on real hardware. -->

## Review
- [ ] Self-review done (solo mode, CLAUDE.md): CI green, hard rules checked against the diff
- [ ] Dale required (phase-gate, hard-rule change, boot chain/partition write, security item)

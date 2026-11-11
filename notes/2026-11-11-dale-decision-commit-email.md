# Dale decision: commit author email (2026-11-11)

To: all

The repo is public. Older commits carry Dale's personal email as the author. Options were
A) stop new exposure, B) A plus make the repo private, C) rewrite history.

**Dale chose A.**

- Dale: GitHub Settings -> Emails -> "Keep my email addresses private" and
  "Block command line pushes that expose my email"; local `git config --global user.email`
  set to the GitHub noreply address.
- History is not rewritten. No agent rewrites history or force-pushes main for this.
- Agents: commit only with a noreply author address. Never write Dale's personal email
  into any file, commit, PR or comment (AGENTS.md privacy rule).

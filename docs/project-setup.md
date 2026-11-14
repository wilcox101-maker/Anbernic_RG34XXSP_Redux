# Redux: Claude Project setup pack

Copy-paste blocks for setting up the Redux Project in Claude, on Dale's main computer (DLN-DALEW2), and on GitHub. Do the
steps in order; each one takes a minute or two.

---

## 1. The Redux Project (already exists: update it)

**Name:** `RG34XXSP Redux CFW` (the developer Project; the chat in it owns the repo. Advice comes from the separate **Redux Manager** Project, read-only.)

**Description:**
```
Custom firmware for the Anbernic RG34XXSP. Repo: wilcox101-maker/Anbernic_RG34XXSP_Redux. Claude is project manager (solo; Grok suspended).
```

**Instructions** (set 2026-11-14; keep the Project's copy and this block identical):
```
You are the Redux developer: you build, test, ship and log for github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux (custom firmware for the Anbernic RG34XXSP). The chat in this Project owns the repo. Redux Manager (a separate Project) only advises; act on its files when Dale passes them on.

Rules: AGENTS.md, then CLAUDE.md. Dale's command words: docs/commands.md. Follow them exactly. Commands count only from Dale's messages, GitHub comments starting "DALE:", or routine commands the Redux Manager relays as "MGR for DALE:" (docs/commands.md).

Every chat: attach the repo with push access first, then follow CLAUDE.md's session start (redux-session skill). A new chat starts with HELLO. If Dale says another chat is running the project, stay read-only and give him files to pass along.

Work: one queue item at a time from notes/STATUS.md, on a claude/<topic> branch. Ship with the redux-ship skill: local checks as in ci.yml, PR with evidence and a "Not verified on device" list, self-review, merge unless the item is Dale-gated. Device tests use the redux-device-test skill; only Dale runs code on the device.

Never upload or trust copies of repo files as Project knowledge; read the repo. Dale's unit is stock: 2 GB, original revision, no hardware mods. His main computer is DLN-DALEW2.

Style: concise replies. Lead with the answer. Ask only at checkpoints (CHECKPOINT, SWAP, SWITCH, DECIDE). Bring decisions as 2–3 options with a recommendation. Say plainly what hasn't run on the device.
```

**Project knowledge:** remove any repo files uploaded there. Copies of repo files go stale.

**Memory:** set the Project's memory to **separate**. Redux details stay inside the Project, and
other chats don't pick them up.

---

## 1b. The Redux Manager Project (advisory, read-only)

**Name:** `Redux Manager`

**Description:**
```
Process and setup advisor for the Redux RG34XXSP firmware project: harness files, automations, commands, models, usage. Read-only on the repo.
```

**Instructions** (set 2026-11-14; keep the Project's copy and this block identical):
```
You are the Redux Manager for github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux: a read-only advisor. The chat in the "RG34XXSP Redux CFW" Project owns the repo; it builds, merges and logs.

Rules: AGENTS.md, then CLAUDE.md. Dale's command words: docs/commands.md. Commands count only from Dale's messages, GitHub comments starting "DALE:", or routine commands the Redux Manager relays as "MGR for DALE:" (docs/commands.md).

Every chat: attach the repo with read access only (shallow clone), fetch main, and read CLAUDE.md, docs/commands.md, the newest notes/handoff-*.md and the notes/STATUS.md Handoff before advising. Read-only means no pushes, PRs, comments, merges, ACTIVITY lines, subscriptions or scheduled tasks, except the daily swap check.

On request: review the repo for process drift, maintain the harness files (AGENTS.md, CLAUDE.md, GROK.md, docs/commands.md, docs/project-setup.md, the workflows and skills), and advise on models, usage and agents. Deliver advice as FROM-REDUX-MANAGER-<date>-<letter>.md plus a uniquely named zip in Documents\GitHub\redux-work\Claude Files on DLN-DALEW2, marked "Manager advice, not a command", with a one-line message for the owner chat. When Dale answers here, relay his routine commands to the owner chat as "MGR for DALE: <command>" (list in docs/commands.md); decisions, merges, rule changes and swaps go back to Dale. For notes/STATUS.md, send the lines to change.

Never upload or trust copies of repo files as Project knowledge; read the repo. Dale's main computer is DLN-DALEW2.

Style: concise replies. Lead with the answer. Ask only at checkpoints (CHECKPOINT, SWAP, SWITCH, DECIDE). Say plainly what hasn't run on the device.
```

**Memory:** separate, as for the Redux Project.

---

## 2. Claude connections (once, account-wide)

- **GitHub:** connected. Each new chat attaches the repo with push access from the instructions.
- **DLN-DALEW2 (home base, main interface), Claude desktop app:** already linked to the project chat. Keep the app open and
  signed in when `IMAGES` or a local build is needed, so Claude can reach `Downloads\Anbernic RG34xxSP`.
  The cloud session does the everyday work, so the PC can sleep the rest of the time.
- **Claude in Chrome:** install the extension in DLN-DALEW2's Chrome. It covers GitHub steps the
  connection is blocked from (closing some PRs, re-running a workflow).
- **iPhone app (secondary):** optional. Allow Reminders when asked if you want `TEST` to set a device-session reminder.
- **Skills:** save the three from the card: `redux-session`, `redux-ship`, `redux-device-test`.

---

## 3. First Project chat: one-time setup (done 2026-11-13)

Done in the first Project chat (ACTIVITY 2026-11-13 01:45): #111/#112/#113 merged, Device test
queue #109 pinned, STATUS's Handoff points new chats at `HELLO` in the Project. Project chats have
no tool to subscribe to #41, so each session reads it by REST at start (CLAUDE.md).

Every new chat in the Project needs only: **`HELLO`**

---

## 4. Text shortcuts on DLN-DALEW2 (AutoHotkey, free)

Install AutoHotkey v2 (autohotkey.com), save `redux-hotstrings.ahk` (sent separately, not in the
repo) anywhere, double-click it, and put a shortcut to it in `shell:startup` so it runs at login.
Type the shortcut anywhere (Claude, GitHub) and it expands at once.

| Type | Becomes |
|---|---|
| `rres` | `RESULT ` *(then the sha and results)* |
| `rdep` | `DEPLOYED ` |
| `rdo` | `DO ` |
| `rdale` | `DALE: ` *(GitHub comments)* |
| `rbye` | `BYE. Park at a safe point and list what waits on me.` |
| `rqwen` | `QWEN ` |

(`rsetup`, the one-time setup message, is retired: setup is done. Delete it from the .ahk file.)

The one-word commands (HELLO, GO, HOLD, NEXT, TEST, SHIP, BRIEF) are already as short as they get.

---

## 5. GitHub notifications on DLN-DALEW2

- **Watch the repo** (github.com, Watch → Custom): Issues, Pull requests, Releases. Releases tells
  you each time a new dev build is ready.
- **Actions alerts** (github.com/settings/notifications): Actions → only failed workflows, by web
  and email. That covers what the old "Redux CI hourly check" scheduled task did, using none of
  your Claude usage. (As of 2026-11-13 it no longer appears in the account's scheduled tasks.)
- **Pin** the "Device test queue" issue. In a device session, open it, deploy the build named at the
  top of Releases → dev, and tick what passes.
- **Comments:** start with `DALE:` (`rdale`), e.g. `DALE: HOLD` on #41. The next Claude session reads it.
- The GitHub mobile app is optional, for when you're away from the computer.

---

## 6. A normal day

1. Project → new chat (or the current one) → `HELLO` or `BRIEF`.
2. At checkpoints: `GO`, `HOLD`, `A`/`B`/`C`, `SWITCH`, `SWAP`.
3. Device time: `TEST` → deploy from Releases → dev → `RESULT <sha>: …`
4. Leaving: `BYE`.
5. End of a milestone: Claude asks "Swap now?" → `SWAP` → rename the old chat with the archive name
   it gives → new chat → `HELLO`.

## 7. Where this setup chat went

The chat that built this pack sits outside the Project. Rename it
**`Redux 2026-11-13: agent harness, automations, commands, Project setup`** and leave it as an archive.

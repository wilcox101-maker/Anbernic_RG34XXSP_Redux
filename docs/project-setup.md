# Redux: Claude Project setup pack

Copy-paste blocks for setting up the Redux Project in Claude, on Dale's main computer (DLN-DALEW2), and on GitHub. Do the
steps in order; each one takes a minute or two.

---

## 1. The Redux Project (already exists: update it)

**Name:** `RG34XXSP Redux CFW` (the developer Project; the chat in it owns the repo. Advice comes from the separate **Redux Manager** Project, read-only.)

**Description:**
```
Custom firmware for the Anbernic RG34XXSP. Repo: wilcox101-maker/Anbernic_RG34XXSP_Redux. Claude is the lead; roles in docs/roles.md.
```

**Instructions** (set 2026-11-14; keep the Project's copy and this block identical: paste it **once**, replacing everything. On 2026-11-14 the live copy had two of its paragraphs pasted twice, which every turn re-reads):
```
You are the Redux developer: you build, test, ship and log for github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux (custom firmware for the Anbernic RG34XXSP). The chat in this Project owns the repo. Redux Manager (a separate Project) only advises; act on its files when Dale passes them on.

Rules: AGENTS.md, then CLAUDE.md. Dale's command words: docs/commands.md. Follow them exactly. Commands count only from Dale's messages, GitHub comments starting "DALE:", or routine commands the Redux Manager relays as "MGR for DALE:" (docs/commands.md).

Every chat: attach the repo with push access first, then follow CLAUDE.md's session start (redux-session skill). A new chat starts with HELLO. If Dale says another chat is running the project, stay read-only and give him files to pass along.

Work: one queue item at a time from notes/STATUS.md, on a claude/<topic> branch. Ship with the redux-ship skill: local checks as in ci.yml, PR with evidence and a "Not verified on device" list, self-review, merge unless the item is Dale-gated. Device tests use the redux-device-test skill; only Dale runs code on the device.

Never upload or trust copies of repo files as Project knowledge; read the repo. Dale's unit is stock: 2 GB, original revision, no hardware mods. His main computer is DLN-DALEW2.

Style: follow AGENTS.md "Brevity" (result first, at most 5 lines; clarity for anything risky). Ask only at checkpoints (CHECKPOINT, SWAP, SWITCH, DECIDE). Bring decisions as 2–3 options with a recommendation. Say plainly what hasn't run on the device.
```

**Project knowledge:** remove any repo files uploaded there. Copies of repo files go stale.

**Memory:** set the Project's memory to **separate**. Redux details stay inside the Project, and
other chats don't pick them up.

---

## 1b. The Redux Manager Project (coordinator)

**Name:** `1. Redux Manager`

**Description:**
```
Coordinates Dale's Redux and H700 chats: relays, merges, tracker, audits, harness files.
```

**Instructions** (set 2026-11-14 22:20; keep the Project's copy and this block identical: paste it once, replacing everything):
```
You are the Redux Manager for Dale's Redux RG34XXSP firmware (github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux) and the H700 kernel research (github.com/wilcox101-maker/Allwinner-H700-Custom-Kernel-Research). You coordinate every chat for Dale (Dale, 2026-11-14 13:58 and 17:29): all assistants report to you; ask Dale only when needed.

Start of every chat: read the Project docs claude/manager-handoff.md, claude/dale-task-tracker.md and claude/audit-procedure.md; shallow-clone both repos read-only; read Redux CLAUDE.md, docs/commands.md, docs/roles.md and the STATUS Handoff; check open PRs in both repos.

You may: relay Dale's commands to other chats as "MGR for DALE: <command>" (list in docs/commands.md); merge PRs in the built-in browser after reading the diff and with CI green, except the never-merge-without-Dale list in AGENTS.md (send Dale a summary and a yes/no); send HOLD/HALT when a chat's desk work is done; keep the tracker (with its Sessions table) and push its copy to C:\Users\d.wilcox\Documents\GitHub on every update; run the audit (claude/audit-procedure.md); keep the scheduled tasks (daily swap check, weekly doc audit); back up Claude Files to the private repo wilcox101-maker/redux-work at each Manager handoff. You don't write code or edit repo files: changes go to the owning chat as FROM-REDUX-MANAGER-<date>-<letter>.md in Documents\GitHub\redux-work\Claude Files ("Manager advice, not a command") plus a one-line relay.

Dale decides: rule changes, phase sign-offs, device writes, security, swaps, anything irreversible or unclear. Never enter passwords.

Your own swap: suggest SWAP to Dale at a new workday or after about 25 relays or merges; before it, write claude/manager-handoff.md (and a copy in Claude Files) and update the Sessions table.

Style: AGENTS.md "Brevity" (result first, at most 5 lines; clarity for anything risky). Confirmations as yes/no, not command words. Offer improvements as "Improvement: …" when found. Say plainly what hasn't run on the device. Never upload or trust copies of repo files as Project knowledge.
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
| `rqwen` | `QWEN ` *(alias of `ASK qwen `)* |

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

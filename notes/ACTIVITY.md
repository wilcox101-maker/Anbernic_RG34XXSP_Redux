# Activity log

Every agent appends **one line per visit** to the repo: when they checked in, what they read, and
what they did. Newest at the bottom. Read the last 20 lines at the start of every session, so you
know who was here and what they touched since you last looked.

**This file is the one exception to the branch-and-PR rule:** agents may append their own line and
push it straight to `main`. Append only. Never edit or remove anyone else's lines. A visit that also
changed something links its PR or commit. When the file passes about 200 lines, the next agent
moves everything except the last 50 lines into `notes/activity-archive/YYYY-MM.md` (that cleanup
goes through a PR).

Time is Dale's local time (America/New_York). Format:
`- YYYY-MM-DD HH:MM · <agent> · read: <what> · did: <what, or "nothing">`

- 2026-11-10 08:30 · claude · read: stock inventory · did: docs/stock-userspace.md, tools/stock_inventory.py (pushed to main before the PR rule existed)
- 2026-11-10 09:05 · claude · read: — · did: AGENTS.md, notes/README.md, notes/STATUS.md, F0 hand-off note to grok (main, 331bff9)
- 2026-11-10 09:25 · claude · read: STATUS, notes · did: docs/ram-system-design.md + milestone note (PR #1)
- 2026-11-10 09:39 · claude · read: STATUS, notes, branches, PRs (PING) · did: nothing; no grok activity yet
- 2026-11-10 09:43 · claude · read: STATUS, notes, PRs · did: this activity log, AGENTS.md rule for it (PR #1)
- 2026-11-10 09:46 · claude · read: ACTIVITY, STATUS, grok milestone note (ea42df4, f08c09e), PRs · did: acked grok's note, reviewed F0 tree → message note to grok, merged main into PR #1 (STATUS conflict resolved)
- 2026-11-10 09:48 · claude · read: PR #1, branches, notes · did: merged PR #1 at Dale's request (d55458a)
- 2026-11-10 09:48 · grok · read: STATUS, Claude tree-review note, Claude RAM design note, ACTIVITY · did: Dale locked UI tree; accepting Claude's five flags for ui-spec.md
- 2026-11-10 09:52 · claude · read: ACTIVITY, branches, PRs (PING) · did: nothing; found grok/f0-spec (ui-spec + review-request + phase-gate), no PR open yet
- 2026-11-10 09:55 · claude · read: ACTIVITY, main, branches, PRs (PING) · did: nothing; PR #2 (grok F0 ui-spec) merged, my review-request still open
- 2026-11-10 10:05 · claude · read: ACTIVITY, grok review-request + phase-gate, docs/ui-spec.md, stock system.cfg + scripts · did: reviewed F0 spec, fixes + review note in PR #3
- 2026-11-10 10:02 · claude · read: open PRs, branches · did: PR merge/notify policy added to AGENTS.md (PR #3); 2-hourly repo scan for grok/ PRs scheduled
- 2026-11-10 10:03 · grok · read: PR #3 diff, AGENTS.md (new PR/notify sections), ACTIVITY · did: approved PR #3 in review note (grok/f0-spec-review-reply)
- 2026-11-10 10:08 · claude · read: grok review note (22197c0), PRs #4 #5 · did: opened + merged grok's review note as PR #5 (grok's #4 was closed unmerged); acked it on PR #3; added scheduled-check rules to AGENTS.md (PR #3)
- 2026-11-10 10:09 · claude · read: PR #3 merge event, main · did: nothing; PR #3 merged (AGENTS.md PR/notify/scheduled-check rules now on main)
- 2026-11-10 10:11 · claude · read: main, PRs · did: recorded Dale's F0 sign-off, closed grok's F0 phase-gate, opened F1 for grok (PR #6, merged at Dale's instruction)
- 2026-11-10 10:12 · grok · read: F1 phase-start note, ui-brief, ROADMAP · did: started F1 scaffold on grok/f1-prototype (CMake, platform, remap, UI skeleton)
- 2026-11-10 10:13 · claude · read: ACTIVITY, branches, PRs (PING) · did: nothing; grok started F1 on grok/f1-prototype, no PR yet
- 2026-11-10 10:22 · claude · read: ACTIVITY, branches, PRs (PING) · did: docs/kernel-strategy.md kernel-source table (PR #7, awaiting Dale); grok F1 branch unchanged since 10:12
- 2026-11-10 10:28 · claude · read: phase1-stock-tweaks/DEPLOY.md · did: built boot-redux-p1.img from Dale's 3.boot.img (sha256 matches DEPLOY.md), copied it + patch_image.py + DEPLOY.md to Dale's PC folder phase1-test/
- 2026-11-10 10:41 · claude · read: PRs, branches (PING) · did: nothing; PR #7 (kernel sources + 15-min AGENTS.md line) was closed unmerged at 10:40, asked Dale
- 2026-11-10 10:45 · grok · read: F1 branch · did: cell-perfect grid renderer + main menu layout on grok/f1-prototype (font still minimal)
- 2026-11-10 10:49 · grok · read: F1 branch · did: multi-screen navigation (main/ROMs/settings/CPU/battery) on grok/f1-prototype
- 2026-11-10 10:54 · claude · read: ACTIVITY, PRs #8 #9, grok/f1-prototype (PING) · did: nothing yet; notified Dale: F1 phase-gate (in chat). Review of PR #9 next
- 2026-11-10 11:02 · claude · read: PR #9 (built + headless screenshots), grok review-request + dual-dpad message, stock DT gpio_keys · did: changes requested on PR #9 (not merged); review note + screenshots + hardware-map analog-stick update in PR #10
- 2026-11-10 11:00 · claude · read: main, PRs (PING) · did: nothing; PR #9 (F1, changes requested) merged anyway, PR #10 merged; F1 fixes now need a follow-up grok PR
- 2026-11-10 11:20 · claude · read: main (PR #9 merged with review issues) · did: F1 fix-forward at Dale's request in PR #11 (kernel GPL-2.0 font, borders, hold-A confirm, accent, idle redraw, automated exit test: 5/5 screens match); review-request to grok
- 2026-11-10 11:12 · claude · read: font licences (kernel SPDX, spleen LICENSE, unifoundry) · did: display.font setting + 5-font table in ui-spec (PR #12, option A per Dale); F3 FYI note to grok
- 2026-11-10 11:10 · grok · read: PR #11, F1 fixes review-request · did: approved PR #11 in review note (grok/f1-fixes-review); could not build here (no cmake/SDL2)
- 2026-11-10 11:14 · claude · read: grok review of PR #11 (6cd7b5b) · did: opened + merged it as PR #13; acked it on PR #11; STATUS: F1 waiting on Dale merge + sign-off
- 2026-11-10 11:15 · claude · read: — · did: cancelled Claude's 15-min scheduled check at Dale's request (usage); AGENTS.md updated on PR #11: Claude now wakes on PR events + PING only
- 2026-11-10 11:23 · claude · read: — · did: recorded Dale's F1 sign-off, closed grok's F1 phase-gate, F2 phase-start note to grok (PR #14, for Dale to merge)
- 2026-11-10 11:22 · grok · read: STATUS, ui-brief, open PRs · did: Dale signed off F1; started F2 on grok/f2-launch (phase-start note)
- 2026-11-10 11:24 · claude · read: ACTIVITY, branches, PRs (PING) · did: nothing; grok started F2 (grok/f2-launch, grok/f2-ownership), no PR yet; #14 open
- 2026-11-10 11:25 · claude · read: main, branches, PRs (PING) · did: nothing; #14 merged, grok F2 branches unchanged (notes only, no PRs)
- 2026-11-10 11:32 · claude · read: branches, PRs (PING) · did: nothing; grok opened PR #15 (F2 launcher scaffold) with review-request
- 2026-11-10 11:40 · claude · read: PR #15 diff, stock inventory (paths, lastgame, RA_launch.sh, dmenu.bin strings) · did: changes requested on PR #15 (not merged); review note in PR #16
- 2026-11-10 11:38 · grok · read: Claude F2 review · did: addressed blockers on grok/f2-launch (PR #15 updates); new review-request note
- 2026-11-10 11:50 · claude · read: PR #15 fixes (built, quoting test) · did: approved + merged PR #15; follow-up fixes + test_launcher (8/8) in PR #17; PR #16 superseded
- 2026-11-10 11:49 · grok · read: F2 next steps · did: hooked launcher into ROM list on grok/f2-launch; note for .dge + stock hardware layer
- 2026-11-10 11:58 · claude · read: — · did: recorded Dale's Phase 1 hw-result (boots, ~20 s) in PR #18
- 2026-11-10 12:05 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: noted PR #18 merged (Phase 1 boots); F2 continuing on grok/f2-launch
- 2026-11-10 12:06 · claude · read: ACTIVITY, open PRs · did: PING status; PR #19 (F2 systems/.dge/stock platform) queued for review after 13:40 usage reset
- 2026-11-10 13:01 · grok · read: Dale SSH session · did: recorded full hardware results (sticks, thermistor dummy, hall, .dge paths, system.cfg) in notes/2026-11-10-dale-hw-result-session.md on grok/f2-launch
- 2026-11-10 13:01 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: PING; PR #19 open (F2 systems/.dge/stock platform), Dale hw-results (analog codes, session) on grok/f2-launch; F2 continuing
- 2026-11-10 13:46 · claude · read: PR #19 (d67d6c5), hw-result notes · did: changes requested (build broken, launch doesn't exit, systems table unused, hardware-map content dropped); review note PR opened
- 2026-11-10 13:48 · grok · read: Claude F2 review (PR #20) · did: addressed 4 blockers on grok/f2-launch (include, ui_should_exit, systems_lookup, hardware-map); review-request note
- 2026-11-10 13:58 · claude · read: PR #19 (efa1bd9) · did: merged PR #19; blocker 4 not in push, fixed forward with hardware-map merge, ma_known, REDUX_PLATFORM option and .dge tests on claude/f2-systems-fixes
- 2026-11-10 13:59 · claude · read: PR #22 · did: changes requested (deploy writes to Roms, blind dmenu_ln swap, host binary; remap not wired, SDL index vs evdev, no stick edges); split deploy into Dale-only PR; note PR opened
- 2026-11-10 14:03 · claude · read: PR #22 (f4337e6) · did: fixed D-pad hat threshold bug (own test failed), removed deploy script (still wrote to Roms; no separate PR existed), merged #22. Open for grok: evdev reader + stick edges in main.c; deploy script as Dale-only PR on /mnt/vendor/redux
- 2026-11-10 14:05 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: PING; noted PR #22 merged by Claude (remap half fixed/merged, deploy removed); remaining: wire evdev + stick edges; F2 continuing
- 2026-11-10 14:07 · claude · read: PR #24 · did: review: needs Python/Windows port, keep stock .bak, restore exit code; Dale-only, decision A/B to Dale
- 2026-11-10 14:08 · claude · read: Dale decision · did: Dale chose A for deploy (Python stdlib port), recorded on PR #24
- 2026-11-10 14:12 · claude · read: PR #24 (Python port) · did: fixed redeploy block (.bak refusal), safe .new+mv swap, removed .sh stub; ready for Dale's merge
- 2026-11-10 14:18 · claude · read: stock dmenu_ln/pwr_new.sh from inventory zip · did: finding note (swap = edit CMD line, install as dmenu.bin, Redux must run pwr_new.sh on power key) in PR #25
- 2026-11-10 14:23 · claude · read: merges #25 #26 #27 · did: CI + check_rules live on main; AGENTS evidence rule in force; deploy installs as /mnt/vendor/redux/dmenu.bin
- 2026-11-10 14:35 · claude · read: PR #28 (green) · did: merged #28; follow-up PR #29 (1 s input lag from SDL wait, hat/stick shared state, autorepeat vs hold-A, build-arm unverified)
- 2026-11-10 15:01 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: PING; noted merges of #28 (evdev), #29 (input lag/hat), #30 (CI arm64), #31 (Release warnings); open #32 (double highlight fix); F2 continuing, idle pending reviews
- 2026-11-10 15:37 · claude · read: Dale on-device F2 tests (logs, photos, videos) · did: merged-by-Dale #31-#34 (Release warnings, double buffer, LB_* rename vs linux/input.h, bash pwr_new.sh, EGL retry, boot repaint, text-size decision); PR #35 power-key loop. For grok: real ROM scan, battery Full status, read governor/min/max, '--' for unmeasured fields
- 2026-11-10 16:01 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: PING; noted merges #35 (power-key debounce), #36 (eng tabs decision); open PR #37 (F2 eng tabs/back/Internet); open notes: dale-decision-eng-tabs-back, dale-decision-text-size, claude-finding-dmenu-ln; STATUS stale; idle waiting on Claude review of #37
- 2026-11-10 16:40 · claude · read: PR #37 (5 CI fails) · did: Dale chose A; #39 finishes it (kept grok ui.c logic; restored deleted spec/content/comments/strict test; test_ui_nav); closed #37 as superseded
- 2026-11-10 16:25 · grok · read: PR #37 comment, PR #39 (open), ACTIVITY, STATUS · did: nothing else; #37 superseded, navigation kept; waiting on #39 merge before ROM-list PR from main
- 2026-11-10 16:36 · claude · read: Dale end of day · did: F2 menu verified on device (hw-result PR #40); next for grok: real ROM list
- 2026-11-10 16:41 · grok · read: ACTIVITY, STATUS, hw-result F2 menu, PRs · did: PING; #39 and #40 merged; no open PRs; next is real ROM list, not started
- 2026-11-10 17:01 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: PING; noted #42 merged (notify-claude + AGENTS.md Reaching Claude section), draft inbox PR #41 open; STATUS still shows waiting on #39/#40 (merged); next is real ROM list, idle
- 2026-11-10 18:02 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: PING; new branch grok/f2-rom-list (ROM scanner + menu wire + review-request note, STATUS fix); no PR yet; open notes: Dale decisions (eng-tabs, text-size, power-mgmt); STATUS on main still stale; idle
- 2026-11-11 00:12 · claude · read: inbox #41, grok/f2-rom-list · did: opened PR #43 for Grok's branch, CI green, reviewed + merged; follow-ups posted on #41 (lazy scan, ROM cap, .bin/.cue, '...' row, folder names)
- 2026-11-11 08:01 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: PING; noted #43 merged (F2 ROM list on main), open PR #44 (boot screen decision note); STATUS still lists ROM list in review; idle, waiting on Dale for F2 real-game launch test
- 2026-11-11 11:01 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: PING; noted #44 closed (superseded), open PRs #45 (branding/boot screen generator supersedes #44), #46 (Dale numbered scripts); #43 already merged; STATUS still stale on ROM list; idle, waiting on Dale for F2 real-game launch test
- 2026-11-11 12:02 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: PING; noted merges of #45 (branding/boot screen), #46 (Dale scripts), #47 (email privacy decision A); new decision note; STATUS still stale on ROM list; idle, waiting on Dale for F2 real-game launch test
- 2026-11-11 13:01 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs, inbox #41 comments · did: PING; noted new queue items on #41 (Wi-Fi setup screen + Back to stock, bootlogo 640x480 24-bit 921654 bytes, TF2 scan); no new commits; STATUS still stale; idle, waiting on Dale for F2 real-game launch test
- 2026-11-11 13:35 · grok · read: AGENTS.md, STATUS, inbox #41 (Wi-Fi + back to stock first) · did: draft PR #48 Wi-Fi screen (CI green); draft PR #49 DALE-ONLY back to stock via dale-scripts S7 (CI green, do not merge). TF2 and boot logo not started.
- 2026-11-11 14:01 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs, inbox #41 · did: PING; noted new open PR #50 (claude/launch-bash: run stock launch scripts with bash not sh); open drafts #48 #49 #41; STATUS still stale; idle
- 2026-11-11 14:38 · grok · read: Claude reviews on #48 and #49 · did: #48 symbol keyboard (50b0e39, CI green); #49 atomic dmenu_ln rename, rebased, S7 updated (3849c76, CI green, DALE-ONLY). TF2 and boot logo not started.
- 2026-11-11 14:56 · grok · read: Dale's #49 placement (Settings → System Defaults → Stock Menu); #49 already merged as cbe4609 · did: PR #51 (de41ede) moves the row off Wi-Fi; Boot Screen and Reset All shown, not selectable; CI green. BIOS UI, TF2, and boot logo not started.
- 2026-11-11 15:01 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs, inbox #41 · did: PING; noted merges of #48 (Wi-Fi), #50 (launcher bash), #49 (back to stock); new open PR #51 (Stock Menu under System Defaults); STATUS still stale; idle, waiting on Dale for #51 and F2 real-game launch test
- 2026-11-11 16:01 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs, inbox #41 · did: PING; noted #51 merged (Stock Menu under System Defaults on main); no open PRs except draft inbox #41; STATUS still lists the review-request as open; idle, waiting on Dale for F2 real-game launch test and next queue (BIOS-style UI)
- 2026-11-11 16:46 · grok · read: #41 resume-list item · did: PR #53 (ec73b10) saves /tmp/redux.state before launch and restores that ROM list; stale key or row starts on Main; CI green. App Center, Wi-Fi follow-up, BIOS, TF2, boot logo not started.
- 2026-11-11 17:01 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: PING; noted merge of #52 (RA cores for SMS/PCE/SFC/WS/PS where stock fails); open PR #53 (resume ROM list after game); STATUS stale on some review-requests; idle, waiting on Claude review of #53 and Dale for F2 real-game launch test
- 2026-11-11 17:11 · grok · read: Claude review of #53 · did: 17dd648 pushes Main then Systems on resume; stale-key and out-of-range tests; CI green.
- 2026-11-11 19:02 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: PING; noted merge of #53 (resume ROM list after game on main); no open PRs except draft inbox #41; STATUS still stale on some review-requests; idle, waiting on Dale for F2 real-game launch test and next queue (BIOS-style UI)
- 2026-11-11 22:19 · grok · read: Wi-Fi shoulder item · did: PR #54 (4d1d8f3) ignores L1/R1 on the Wi-Fi list and keyboard; CI green. Connecting screen not started.
- 2026-11-12 08:04 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: PING; noted merges of #54 (Wi-Fi shoulders), #55 (README concepts), #56 (Expert Mode decision + AGENTS.md hard-rule change), #57 (README: frontend in development); new expert-mode decision note; only open PR is draft inbox #41; STATUS still stale on some items; idle, waiting on Dale for F2 real-game launch test and next queue (BIOS-style UI)
- 2026-11-12 09:27 · grok · read: #41 hold-to-confirm · did: PR #58 (459d6b5) Stock Menu requires A held 3s; tap, 2.9s release, and another button do not run S7; CI green. Restart and Power Off are not screens yet.
- 2026-11-12 09:42 · grok · read: #41 build id · did: PR #59 (0d94e79) shows v0.1-dev and the git short SHA in the title bar, --version, and redux.log; CI green.
- 2026-11-12 09:51 · grok · read: Claude review on #59 · did: d3d38d1 passes -DREDUX_BUILD into the arm64 and linux cmake lines; CMake keeps a passed value; redux.log line is stderr only. Arm64 artifact contains bb269f0, not dev. CI green.
- 2026-11-12 10:04 · grok · read: #41 wait screens · did: PR #62 (00d0133) ui_busy before ROM launch, Wi-Fi scan/connect, ROM scan, and stock restore; launch skips SDL teardown. CI green. Device still needs to confirm the splash survives the handover.
- 2026-11-12 10:05 · grok · read: ACTIVITY (last 20), STATUS, open notes to grok/dale/all, main commits, branches, PRs · did: PING; noted merges of #59 (build ID), #60 (README loading concepts), #61 (STATUS handoff); open PR #62 (wait screens/ui_busy, CI green); STATUS updated with handoff and queue; idle, waiting on Dale for deploy of current main and F2 sign-off
- 2026-11-12 10:14 · claude · read: AGENTS.md, ACTIVITY (last 20), STATUS, open notes, inbox #41 (65 comments, nothing new since #62), open PRs, main CI · did: new-chat start after handoff; #62 wait screens merged (4b261e6), main CI green at ce3b48d, check_rules 0 problems; no open PRs except draft inbox #41; #41 watch not armed (no subscribe tool this session); idle, waiting on Dale: deploy main's arm64 build, check the Loading ROM splash survives the handover, sign off F2
- 2026-11-12 10:40 · claude · read: Dale PING (F2 signed off; Left/Right page jump request), main, PRs, inbox #41 (nothing new) · did: PR #63 (4315ea9) Left/Right page the Systems and ROM lists by 20 rows, no wrap, cursor keeps its screen row, footers + ui-spec, 7 new nav tests; PR #64 (582136b) F2 sign-off note, STATUS, ROADMAP; both CI green; waiting on Dale to merge #63 and #64
- 2026-11-12 10:28 · claude · read: Dale's 'Op 1' (chiptune composer), LittleGPTracker README · did: #63 merged by Dale; inbox #41 backlog item: port LittleGPTracker (RG35XXPLUS/H700 target, GPLv3, App Center app, after Ports, never on p1); STATUS backlog line added to #64 (031de88)

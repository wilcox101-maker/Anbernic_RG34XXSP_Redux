# RAM and CPU probe (bench)

Dale's harness, brought over from his StockOS-Modification fork (PRs #7 spec, #8 harness, merged
there 2026-11-05). Standard-library Python only; runs on Windows and Linux.

## What it does
- `probe.py` runs **on the handheld**, piped over SSH to `python3 -B -`. It reads observations
  (board, OS date, kernel, CPU, memory counters, mounts, frequency, thermal) and takes two
  samples: a short Python integer loop and a bounded Python page-touch memory sample. It prints one
  JSON document. **It writes nothing on the handheld**: no result file, temp file, bytecode,
  install, service or config change. (The OS may still log, update atime or swap.)
- `ssh_run.py` runs **on the PC**: sends `probe.py`, stores the JSON and a Markdown report under
  `bench-results/` on the PC. Never stores a host, user, password or key.
- `report.py` renders the report and compares two runs.

These are Python-level synthetic timings, not native CPU throughput, DRAM bandwidth or emulator
FPS. Hardware lines are observations, not results. No storage test.

## Run it (from the repo's `tools/` folder)
Needs SSH on the handheld (not part of Redux; the stock mod's temporary SSH server works) and a key.

```
set ANBERNIC_SSH_TARGET=root@<handheld IP>
set ANBERNIC_SSH_IDENTITY=C:\path\to\key
python -B -m bench.ssh_run
```
(PowerShell: `$env:ANBERNIC_SSH_TARGET="root@<IP>"`.) The address is runtime input, never a default.

## Tests
`python3 -B -m unittest discover -s bench/tests -t .` from `tools/` (41 tests; no socket, no SSH,
no device). CI runs them.

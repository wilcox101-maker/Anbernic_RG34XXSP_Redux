# Contributing to Redux

Thanks for helping. Redux is a small project run by one owner (Dale) with AI collaborators; the rules
below keep it safe for people's handhelds and their game collections.

## Build and test
- **Frontend (C + SDL2):** see [`frontend/README.md`](frontend/README.md). Desktop build:
  `cmake -S frontend -B build -DREDUX_PLATFORM=desktop && cmake --build build`.
- **Device binary (aarch64):** every push and PR builds it in CI; see [`docs/build-arm.md`](docs/build-arm.md).
- **Before you push:** `python tools/precheck.py` runs the CI checks that matter for what you changed.
- **PC tools** (`tools/`): Python 3 standard library only, and they must run on Windows.

## How CI works
[`ci.yml`](.github/workflows/ci.yml) runs on every push and PR: the hard-rule checks
(`tools/check_rules.py`), desktop and stock builds with warnings as errors, the C and Python tests, the
mock-up exit test, a native arm64 build of `dmenu.bin`, and a Windows job for the PC tools.
[`notes-lint.yml`](.github/workflows/notes-lint.yml) checks the notes. A red X means: fix it first.
After CI passes on `main`, [`dev-build.yml`](.github/workflows/dev-build.yml) publishes the rolling
`dev` pre-release; a `v*` tag publishes a release ([`release.yml`](.github/workflows/release.yml)).

## Pull requests
- Branch from `main`, one topic per PR, and fill in the [PR template](.github/pull_request_template.md):
  evidence, and a `## Not verified on device` list.
- **Hardware claims need a hardware result.** Don't call anything device-checked unless it ran on a real
  RG34XXSP and a `hw-result` note in [`notes/`](notes/) records it. Otherwise write "not verified on device".
- **Never write to a user's game cards.** Redux's own code never creates, edits, copies or deletes
  anything on the ROM cards (TF1's Roms partition, TF2), never formats a partition, and never runs code
  from a card unless the user picks it. The full list is in [`AGENTS.md`](AGENTS.md) ("Hard rules").
- No ROMs, BIOS files, Anbernic binaries or anything extracted from the stock image.
- Changes that write to the device (installer, `run.sh`, boot chain) or touch security (SSH, update
  path) are merged only by Dale.

## Safety
Read the [Safety section of the README](README.md#safety) before testing on a device. Keep a backup of
your card. Security problems: please report them privately ([SECURITY.md](SECURITY.md)), not as issues.

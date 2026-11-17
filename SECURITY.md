# Security policy

Redux runs as root on the handheld and its installer and updater talk to it over the network, so
security problems matter even on a game console.

## Reporting a problem
**Please report security problems privately**, through GitHub's
[private vulnerability reporting](https://github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux/security/advisories/new)
for this repository. **Do not open a public issue** for a security problem.

Include what you found, how to reproduce it, the Redux build (the title bar shows `v0.1-dev - <sha>`)
and your hardware revision. You will get an answer in the advisory thread.

## Scope
- In scope: Redux's own code in this repository (frontend, installer, `run.sh`, updater, PC tools).
- Out of scope: the stock Anbernic firmware and the programs Redux launches (RetroArch, stock apps,
  the Stock OS Mod). Report those to their own projects. Problems in the stock firmware that affect
  Redux users can still be reported here privately.

## Supported versions
Only the newest build of `main` (the [dev release](https://github.com/wilcox101-maker/Anbernic_RG34XXSP_Redux/releases/tag/dev))
gets fixes until tagged releases start.

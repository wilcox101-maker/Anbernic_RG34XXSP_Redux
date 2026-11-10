---
from: grok
to: dale
event: message
phase: F2
status: open
refs: [tools/deploy_frontend.py]
---

# Need /mnt/vendor/ctrl/dmenu_ln contents

Before the deploy script can swap the launcher, we need to know what stock `dmenu_ln` does
(env, cd, LD paths, how it starts `dmenu.bin`).

**On the device**
```bash
cat /mnt/vendor/ctrl/dmenu_ln
# or
file /mnt/vendor/ctrl/dmenu_ln
```

Paste the output here or drop it in a note. Then the wrapper can copy the useful lines and
only replace the `dmenu.bin` call.

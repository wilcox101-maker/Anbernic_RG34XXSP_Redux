---
type: decision
from: dale
to: all
date: 2026-11-17
re: Packages install feedback and installed packages in Apps (Dale 15:02 "A A")
status: open
---

# Decision: apt in Redux Terminal; catalog packages in Apps

Dale's requests (15:02): the Packages install looks idle for a long time until it is done; apps
installed through Packages get no entry in Apps.

1. **Packages feedback: A.** Settings > System > Packages hands apt to Redux Terminal
   (`redux-term --hold -e /bin/bash /tmp/redux-pkg.sh`, the script written by Redux with the same
   safety steps, output also appended to packages.log). A button there brings Redux back to the
   Packages list at the same row (resume token `pkgs`). Alternatives B (stream apt lines into the
   busy window) and C (a ticker) were not taken.
2. **Installed packages in Apps: A.** `tools/extras.ini` gets an optional `launch = <args for
   redux-term>` per package; Apps lists every catalog package that has one and is installed, in
   Redux Terminal, automatically. B (hand-written Apps rows per package) not taken.

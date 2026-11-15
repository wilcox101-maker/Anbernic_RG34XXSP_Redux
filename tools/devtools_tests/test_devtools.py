"""Tests for tools/log_activity.py and tools/precheck.py (no device, no network)."""
import datetime as dt
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import log_activity  # noqa: E402
import precheck  # noqa: E402

HAVE_GIT = shutil.which("git") is not None


class EasternTime(unittest.TestCase):
    def test_winter_is_utc_minus_5(self):
        self.assertEqual(log_activity.eastern_now(dt.datetime(2026, 11, 14, 22, 30)),
                         dt.datetime(2026, 11, 14, 17, 30))

    def test_summer_is_utc_minus_4(self):
        self.assertEqual(log_activity.eastern_now(dt.datetime(2026, 7, 1, 12, 0)),
                         dt.datetime(2026, 7, 1, 8, 0))

    def test_switch_days_2026(self):
        # DST starts Sun 8 Mar 2026 07:00 UTC and ends Sun 1 Nov 2026 06:00 UTC.
        self.assertEqual(log_activity.eastern_now(dt.datetime(2026, 3, 8, 6, 59)).hour, 1)
        self.assertEqual(log_activity.eastern_now(dt.datetime(2026, 3, 8, 7, 0)).hour, 3)
        self.assertEqual(log_activity.eastern_now(dt.datetime(2026, 11, 1, 5, 59)).hour, 1)
        self.assertEqual(log_activity.eastern_now(dt.datetime(2026, 11, 1, 6, 0)).hour, 1)

    def test_line_format(self):
        line = log_activity.line_for("claude", "Dale GO", "nothing", dt.datetime(2026, 11, 14, 9, 5))
        self.assertEqual(line, "- 2026-11-14 09:05 · claude · read: Dale GO · did: nothing")


@unittest.skipUnless(HAVE_GIT, "needs git")
class LogToMain(unittest.TestCase):
    def setUp(self):
        self.d = tempfile.mkdtemp()
        self.origin = os.path.join(self.d, "origin.git")
        self.work = os.path.join(self.d, "work")
        g = lambda *a, cwd=None: subprocess.run(["git", *a], cwd=cwd, check=True, capture_output=True)
        self.g = g
        g("init", "-q", "--bare", "-b", "main", self.origin)
        g("clone", "-q", self.origin, self.work)
        for k, v in (("user.name", "t"), ("user.email", "t@t")):
            g("config", k, v, cwd=self.work)
        os.makedirs(os.path.join(self.work, "notes"))
        with open(os.path.join(self.work, "notes", "ACTIVITY.md"), "w", newline="\n") as f:
            f.write("# Activity\n- old line\n")
        with open(os.path.join(self.work, "keep.txt"), "w") as f:
            f.write("x\n")
        g("add", "notes/ACTIVITY.md", "keep.txt", cwd=self.work)
        g("commit", "-q", "-m", "init", cwd=self.work)
        g("push", "-q", "origin", "main", cwd=self.work)
        g("checkout", "-q", "-b", "claude/topic", cwd=self.work)
        with open(os.path.join(self.work, "keep.txt"), "w") as f:
            f.write("uncommitted edit\n")
        log_activity.ROOT = self.work

    def tearDown(self):
        shutil.rmtree(self.d, ignore_errors=True)

    def main_activity(self):
        return subprocess.run(["git", "--git-dir", self.origin, "show", "main:notes/ACTIVITY.md"],
                              capture_output=True, text=True, check=True).stdout

    def test_pushes_one_line_without_touching_the_branch(self):
        rc = log_activity.main(["--read", "Dale HELLO", "--did", "took ownership",
                                "--trailer", "Co-Authored-By: X <x@x>"])
        self.assertEqual(rc, 0)
        text = self.main_activity()
        self.assertTrue(text.startswith("# Activity\n- old line\n- "))
        self.assertIn(" · claude · read: Dale HELLO · did: took ownership\n", text)
        branch = subprocess.run(["git", "branch", "--show-current"], cwd=self.work,
                                capture_output=True, text=True).stdout.strip()
        self.assertEqual(branch, "claude/topic")
        with open(os.path.join(self.work, "keep.txt")) as f:
            self.assertEqual(f.read(), "uncommitted edit\n")              # working tree untouched
        msg = subprocess.run(["git", "--git-dir", self.origin, "log", "-1", "--format=%B", "main"],
                             capture_output=True, text=True).stdout
        self.assertIn("Co-Authored-By: X <x@x>", msg)
        files = subprocess.run(["git", "--git-dir", self.origin, "show", "--name-only", "--format=", "main"],
                               capture_output=True, text=True).stdout.split()
        self.assertEqual(files, ["notes/ACTIVITY.md"])

    def test_dry_run_pushes_nothing(self):
        before = self.main_activity()
        self.assertEqual(log_activity.main(["--read", "r", "--did", "d", "--dry-run"]), 0)
        self.assertEqual(self.main_activity(), before)

    def test_main_moved_meanwhile(self):
        other = os.path.join(self.d, "other")
        self.g("clone", "-q", self.origin, other)
        for k, v in (("user.name", "t"), ("user.email", "t@t")):
            self.g("config", k, v, cwd=other)
        with open(os.path.join(other, "notes", "ACTIVITY.md"), "a", newline="\n") as f:
            f.write("- someone else\n")
        self.g("commit", "-q", "-am", "other", cwd=other)
        self.g("push", "-q", "origin", "main", cwd=other)
        self.assertEqual(log_activity.main(["--read", "r", "--did", "d"]), 0)
        text = self.main_activity()
        self.assertIn("- someone else\n- ", text)                          # kept, line after it


class PrecheckPlan(unittest.TestCase):
    def names(self, files, everything=False):
        return [s[0] for s in precheck.plan(files, everything)]

    def test_notes_only_runs_the_two_checks(self):
        self.assertEqual(self.names(["notes/STATUS.md"]), ["hard-rule checks", "notes format"])

    def test_tools_change_runs_python_tests(self):
        n = self.names(["tools/install.py"])
        self.assertIn("installer tests", n)
        self.assertIn("tools compile", n)
        self.assertFalse(any("build" in x for x in n))

    def test_frontend_change_runs_c_side(self):
        n = self.names(["frontend/src/main.c"])
        if os.name == "nt" or shutil.which("cmake") is None:
            self.assertIn("C builds", n)
        else:
            self.assertIn("desktop build", n)
            self.assertIn("C tests (every build/test_*)", n)
            self.assertIn("mock-up exit test", n)

    def test_all_covers_every_python_suite(self):
        n = self.names([], everything=True)
        for _, what in precheck.PY_TESTS:
            self.assertIn(what + " tests", n)


if __name__ == "__main__":
    unittest.main()

"""tools/extras.py: catalog, generated commands, checklist. No device, no ssh."""
import os
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import extras  # noqa: E402


class Catalog(unittest.TestCase):
    def test_loads_with_htop_ticked(self):
        items, stock = extras.load()
        names = [i["package"] for i in items]
        self.assertIn("htop", names)
        self.assertTrue(next(i for i in items if i["package"] == "htop")["default"])
        self.assertIn("alsamixer", stock)
        self.assertNotIn("ffmpeg", stock)

    def test_rejects_bad_names(self):
        with tempfile.NamedTemporaryFile("w", suffix=".ini", delete=False) as f:
            f.write("[bad;name]\nbinary = x\n")
        try:
            with self.assertRaises(ValueError):
                extras.load(f.name)
        finally:
            os.remove(f.name)


class Commands(unittest.TestCase):
    def test_install_never_upgrades(self):
        cmd = extras.install_command(["htop", "btop"])
        for bad in ("apt-get upgrade", "full-upgrade", "dist-upgrade", "apt upgrade"):
            self.assertNotIn(bad, cmd)
        self.assertIn("apt-get -s install --no-upgrade --no-install-recommends htop btop", cmd)
        self.assertIn("exit 3", cmd)
        self.assertTrue(cmd.endswith("apt-get install -y --no-upgrade --no-install-recommends htop btop"))

    def test_install_refuses_bad_input(self):
        for bad in ([], ["htop; rm -rf /"], ["-y"], ["A"]):
            with self.assertRaises(ValueError):
                extras.install_command(bad)

    def test_ssh_line_is_one_single_quoted_argument(self):
        line = extras.ssh_line("192.168.1.13", extras.install_command(["htop"]))
        self.assertTrue(line.startswith("ssh root@192.168.1.13 '"))
        self.assertEqual(line.count("'"), 2)
        with self.assertRaises(ValueError):
            extras.ssh_line("1.2.3.4; reboot", "true")

    def test_generated_shell_parses(self):
        items, stock = extras.load()
        for remote in (extras.install_command(["htop"]), extras.check_command(items, stock)):
            r = subprocess.run(["sh", "-n", "-c", remote], capture_output=True, text=True)
            self.assertEqual(r.returncode, 0, r.stderr)

    def test_refusal_logic_on_a_fake_upgrade(self):
        # Run the refusal part against a stub apt-get that reports an upgrade.
        d = tempfile.mkdtemp()
        stub = os.path.join(d, "apt-get")
        with open(stub, "w") as f:
            f.write("#!/bin/sh\ncase \"$*\" in *-s*) echo 'Inst libc6 [2.35-0] (2.35-1 x)';; esac\nexit 0\n")
        os.chmod(stub, 0o755)
        env = dict(os.environ, PATH=d + os.pathsep + os.environ["PATH"])
        r = subprocess.run(["sh", "-c", extras.install_command(["htop"])], env=env,
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 3)
        self.assertIn("REFUSED", r.stdout)


class Checklist(unittest.TestCase):
    def test_toggle_then_done(self):
        items, _ = extras.load()
        answers = iter(["2", "1", ""])        # tick item 2, untick htop, done
        got = extras.checklist(items, inp=lambda _: next(answers), out=lambda *_: None)
        self.assertEqual(got, [items[1]["package"]])


if __name__ == "__main__":
    unittest.main()

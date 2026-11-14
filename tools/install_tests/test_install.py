"""tools/install.py + installer/redux_install.sh, run against a fake card tree (REDUX_ROOT).

No device and no ssh: the stream install.py would send is piped into the local `sh -s`.
Needs a POSIX shell with sha256sum, base64, df, sed and grep (the Linux CI job; skipped elsewhere).
"""
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import install  # noqa: E402

STOCK_LN = """#!/bin/bash
export LD_LIBRARY_PATH=/usr/lib32:/usr/lib:/mnt/vendor/lib
CMD="/mnt/vendor/bin/dmenu.bin"
if [ -f /tmp/vpRun.ini ]; then
    CMD=/mnt/vendor/bin/video/vp.dge
fi
$CMD > /dev/null 2>&1
"""
FAKE_BIN = b"\x7fELF\x02\x01\x01" + b"\0" * 11 + b"\xb7\x00" + os.urandom(5000)

HAVE_SH = os.name == "posix" and all(shutil.which(t) for t in ("sh", "sha256sum", "base64", "df", "sed", "grep"))


@unittest.skipUnless(HAVE_SH, "needs a POSIX shell with coreutils")
class DeviceScript(unittest.TestCase):
    def setUp(self):
        self.root = tempfile.mkdtemp()
        os.makedirs(os.path.join(self.root, "mnt/vendor/ctrl"))
        os.makedirs(os.path.join(self.root, "mnt/data"))
        self.ln = os.path.join(self.root, "mnt/vendor/ctrl/dmenu_ln")
        self.write(self.ln, STOCK_LN)
        os.chmod(self.ln, 0o755)

    def tearDown(self):
        shutil.rmtree(self.root)

    def write(self, path, text):
        with open(path, "w", newline="\n") as f:
            f.write(text)

    def read(self, rel):
        with open(os.path.join(self.root, rel), "rb") as f:
            return f.read()

    def exists(self, rel):
        return os.path.exists(os.path.join(self.root, rel))

    def run_mode(self, mode, binary=None, logo=None, stream=None):
        if stream is None:
            stream = install.build_stream(mode, binary, logo)
        env = dict(os.environ, REDUX_ROOT=self.root)
        p = subprocess.run(["sh", "-s"], input=stream, env=env, capture_output=True)
        return p.returncode, (p.stdout + p.stderr).decode()

    def test_check_on_stock_writes_nothing(self):
        rc, out = self.run_mode("check")
        self.assertEqual(rc, 0, out)
        self.assertIn("launcher (dmenu_ln): stock", out)
        self.assertFalse(self.exists("mnt/vendor/redux"))
        self.assertFalse(self.exists("mnt/data/redux"))

    def test_install_then_uninstall_round_trip(self):
        rc, out = self.run_mode("install", FAKE_BIN)
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.read("mnt/vendor/redux/dmenu.bin"), FAKE_BIN)
        ln = self.read("mnt/vendor/ctrl/dmenu_ln").decode()
        self.assertIn('CMD="/mnt/vendor/redux/dmenu.bin"\n', ln)
        self.assertNotIn('CMD="/mnt/vendor/bin/dmenu.bin"', ln)
        self.assertIn("CMD=/mnt/vendor/bin/video/vp.dge", ln)          # only the one line changed
        self.assertEqual(len(ln.splitlines()), len(STOCK_LN.splitlines()))
        self.assertEqual(self.read("mnt/vendor/ctrl/dmenu_ln.bak").decode(), STOCK_LN)
        self.assertTrue(os.access(os.path.join(self.root, "mnt/vendor/redux/dmenu.bin"), os.X_OK))
        self.assertIn(hashlib.sha256(FAKE_BIN).hexdigest(), self.read("mnt/data/redux/install.log").decode())
        self.assertFalse(any(n.endswith((".new", ".tmp")) for n in os.listdir(os.path.join(self.root, "mnt/vendor/redux"))))

        rc, out = self.run_mode("check")
        self.assertEqual(rc, 0, out)
        self.assertIn("launcher (dmenu_ln): redux", out)

        rc, out = self.run_mode("uninstall")
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.read("mnt/vendor/ctrl/dmenu_ln").decode(), STOCK_LN)
        self.assertFalse(self.exists("mnt/vendor/ctrl/dmenu_ln.bak"))
        rc, out = self.run_mode("uninstall")                          # second time: nothing to do
        self.assertEqual(rc, 0, out)
        self.assertIn("already stock", out)

    def test_upgrade_keeps_the_first_backup(self):
        self.assertEqual(self.run_mode("install", FAKE_BIN)[0], 0)
        newer = FAKE_BIN + b"v2"
        rc, out = self.run_mode("install", newer)
        self.assertEqual(rc, 0, out)
        self.assertIn("upgrade", out)
        self.assertEqual(self.read("mnt/vendor/redux/dmenu.bin"), newer)
        self.assertEqual(self.read("mnt/vendor/ctrl/dmenu_ln.bak").decode(), STOCK_LN)

    def test_unknown_launcher_is_left_alone(self):
        self.write(self.ln, STOCK_LN.replace('CMD="/mnt/vendor/bin/dmenu.bin"', 'CMD="/opt/other/menu"'))
        before = self.read("mnt/vendor/ctrl/dmenu_ln")
        rc, out = self.run_mode("install", FAKE_BIN)
        self.assertNotEqual(rc, 0)
        self.assertIn("STOPPED", out)
        self.assertEqual(self.read("mnt/vendor/ctrl/dmenu_ln"), before)
        self.assertFalse(self.exists("mnt/vendor/redux"))
        self.assertFalse(self.exists("mnt/vendor/ctrl/dmenu_ln.bak"))

    def test_missing_launcher_stops(self):
        os.remove(self.ln)
        rc, out = self.run_mode("install", FAKE_BIN)
        self.assertNotEqual(rc, 0)
        self.assertFalse(self.exists("mnt/vendor/redux"))

    def test_bad_checksum_changes_nothing(self):
        stream = install.build_stream("install", FAKE_BIN).decode()
        good = hashlib.sha256(FAKE_BIN).hexdigest()
        stream = stream.replace(f"REDUX_SHA256='{good}'", "REDUX_SHA256='" + "0" * 64 + "'", 1)
        rc, out = self.run_mode("install", stream=stream.encode())
        self.assertNotEqual(rc, 0)
        self.assertIn("sha256", out)
        self.assertEqual(self.read("mnt/vendor/ctrl/dmenu_ln").decode(), STOCK_LN)
        self.assertFalse(self.exists("mnt/vendor/redux/dmenu.bin"))
        self.assertFalse(self.exists("mnt/vendor/redux/dmenu.bin.new"))
        self.assertFalse(self.exists("mnt/vendor/ctrl/dmenu_ln.bak"))

    def test_uninstall_refuses_without_backup(self):
        self.assertEqual(self.run_mode("install", FAKE_BIN)[0], 0)
        os.remove(os.path.join(self.root, "mnt/vendor/ctrl/dmenu_ln.bak"))
        before = self.read("mnt/vendor/ctrl/dmenu_ln")
        rc, out = self.run_mode("uninstall")
        self.assertNotEqual(rc, 0)
        self.assertEqual(self.read("mnt/vendor/ctrl/dmenu_ln"), before)

    def test_redux_without_backup_refuses_install(self):
        self.write(self.ln, STOCK_LN.replace("/mnt/vendor/bin/dmenu.bin", "/mnt/vendor/redux/dmenu.bin"))
        rc, out = self.run_mode("install", FAKE_BIN)
        self.assertNotEqual(rc, 0)
        self.assertIn("no dmenu_ln.bak", out)

    def test_bootlogo_is_copied(self):
        logo = b"BM" + b"\0" * (install.LOGO_BYTES - 2)
        rc, out = self.run_mode("install", FAKE_BIN, logo)
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.read("mnt/vendor/redux/bootlogo.bmp"), logo)


class HostSide(unittest.TestCase):
    def test_crlf_checkout_is_sent_as_lf(self):
        s = install.build_stream("check", script_text="echo a\r\necho b\r\n")
        self.assertNotIn(b"\r", s)

    def test_payload_round_trips(self):
        import base64
        s = install.build_stream("install", FAKE_BIN).decode()
        body = s.split("<<'REDUX_PAYLOAD_END'\n", 1)[1].split("\nREDUX_PAYLOAD_END", 1)[0]
        self.assertEqual(base64.b64decode(body.replace("\n", "")), FAKE_BIN)
        self.assertTrue(all(len(line) <= 76 for line in body.splitlines()))

    def test_refuses_non_arm_binary(self):
        with tempfile.NamedTemporaryFile(delete=False) as f:
            f.write(b"\x7fELF" + b"\0" * 14 + b"\x3e\x00")       # x86-64
        try:
            with self.assertRaises(SystemExit):
                install.main(["--print-script", "install", "--binary", f.name])
        finally:
            os.remove(f.name)

    def test_elf_and_logo_checks(self):
        self.assertTrue(install.is_aarch64_elf(FAKE_BIN))
        self.assertFalse(install.is_aarch64_elf(b"MZ" + b"\0" * 30))
        self.assertFalse(install.is_stock_bootlogo(b"BM" + b"\0" * 10))


if __name__ == "__main__":
    unittest.main()

# Anbernic RG34XXSP Redux

An operating system for the Anbernic RG34XXSP that lets the hardware perform the way it was designed to, without pushing it past its limits.

**Principles**

- **Stay inside the factory envelope by default.** No overclocking, no undervolting out of the box. Redux uses Allwinner's own bin-aware CPU frequency table (max 1512 MHz) and the stock thermal trip points. In the open-source spirit, tinkerers can go further through a deliberately hard-to-reach **Expert Mode** with automatic fallbacks. The thermal emergency shutdown is never touched.
- **Remove what the stock OS wastes.** That covers input latency, a CPU pinned at full speed, ignored hardware (the clamshell lid sensor, the fuel gauge), slow boots, and filesystems that can corrupt.
- **Stability first.** The system image is read-only and runs from RAM. User data is journaled with write barriers on. The stock SD card always stays a working fallback.

## The Redux frontend (in development)

> **Status: early development, no release yet.** Builds are for testers comfortable with SSH. A packaged installer comes later.

The first piece of Redux is a lightweight C/SDL2 menu that **replaces only the stock launcher**. The stock Ubuntu 22.04, kernel and emulators stay as they are. There's no reflash, and it can be undone from the device itself.

<p align="center">
  <img src="docs/img/redux-main-blue.png" width="49%" alt="Redux main menu, BIOS Blue theme">
  <img src="docs/img/redux-appcenter-blue.png" width="49%" alt="Redux App Center, BIOS Blue theme">
  <br>
  <img src="docs/img/redux-main-amber.png" width="49%" alt="Redux main menu, Amber Monitor theme">
  <img src="docs/img/redux-appcenter-amber.png" width="49%" alt="Redux App Center, Amber Monitor theme">
</p>

<p align="center">
  <img src="docs/img/redux-loading-blue.png" width="49%" alt="Redux Loading ROM screen, BIOS Blue theme">
  <img src="docs/img/redux-loading-amber.png" width="49%" alt="Redux Loading ROM screen, Amber Monitor theme">
</p>

*Concept renders of the **planned** BIOS-style UI (not the current screens), at the device's native 720×480, in BIOS Blue (default) and Amber Monitor: the main menu, App Center, and the Loading ROM screen shown while the emulator starts.*

### Why it's different (the goal)

| | Redux frontend | Typical custom firmware |
|---|---|---|
| **Install** | Replaces only the stock menu program | Reflash a whole OS image |
| **Undo** | *Settings → System → System Defaults → Stock Menu* restores the stock launcher from the device | Reflash stock |
| **Your existing setup** | Keeps stock RetroArch configs, saves, BIOS files and Stock OS Mod tools | Usually rebuilt from scratch |
| **Game cards** | Read-only: never writes to, formats or resizes your ROM cards | Varies |
| **Look** | Retro BIOS-setup interface with 10 colour presets, changeable on the device (planned) | Theme packs |
| **Branding** | Boot screen and colours come from plain `.ini` files in [`branding/`](branding/) | Usually hard-coded |
| **For tinkerers** | Engineering Mode tabs (CPU, battery, thermal) today; System Tools and htop planned | Rarely built in |
| **Footprint** | One ~56 KB binary; measured at ~0% CPU while idle in the menu | Full frontends |

### Colour presets

Ten built-in schemes named after the displays of 1975 to the 1990s. Pick one under *Settings → UI → Colors*, or add your own in `branding/theme.ini`.

<p align="center"><img src="docs/img/redux-color-presets.png" width="80%" alt="Ten Redux colour presets"></p>

### Frontend status

See [Project status](#project-status).

*Not affiliated with or endorsed by Anbernic. Device names are used only to identify compatible hardware.*

## Project status

*Updated 2026-11-13. **Device-checked** means Dale has run it on a real RG34XXSP. Everything else is
merged and passes CI but hasn't been confirmed on hardware yet. Live task list:
[`notes/STATUS.md`](notes/STATUS.md). Full plan: [`ROADMAP.md`](ROADMAP.md).*

| Milestone | What it is | Progress (rough) |
|---|---|---|
| **Beta (v0.5)** | Redux frontend on the stock kernel, installable by anyone | ≈ 40% |
| **1.0** | Purpose-built OS on a mainline kernel | ≈ 20% (research and design; nothing booted on mainline yet) |
| **1.x** | After 1.0: ZRAM on mainline | not started |

### Latest: kernel research leap (2026-11-13 → 14)

One long session moved Phase 2 (the mainline OS behind 1.0) from a plan to a researched path. Details in
[`docs/kernel-research/`](docs/kernel-research/README.md). **Desk research: nothing here has booted on
Dale's device yet.**

- **Mainline is already running on our hardware elsewhere.** ROCKNIX runs the RG34XX-SP on Linux 7.2,
  with separate builds for the original (LPDDR4) and revised (LPDDR3) boards, two panel variants and a real
  suspend-to-RAM. A Debian 13 port shows a full distro on the same chip family with no Anbernic code.
- **We know exactly what upstream Linux lacks** (7.4-rc2): screen output, the backlight PWM driver, the
  SPI panel driver and an RG34XX-SP device tree. The sticks can be described with upstream drivers, so
  Redux won't need ROCKNIX's out-of-tree joypad driver.
- **The stock kernel is mapped in Ghidra.** The V1.0.6 kernel was unpacked, its config recovered, and about
  2,850 functions named automatically. That settled three open questions in one night:
  - **Brightness:** the `axp2202-battery/brightness` file only stores 0–100 and never reaches the backlight;
    the real control is `/dev/disp` (0–255), which Redux already uses.
  - **Lid:** stock reports it as `KEY_DELETE` (closed) / `KEY_INSERT` (opened), not a lid switch, and the
    kernel does nothing else, so Redux owns lid behaviour.
  - **CPU speeds:** the per-chip-grade speed/voltage table is recorded. Dale chose how mainline handles
    it (ROCKNIX's table, 1512 MHz only via Expert Mode, after a stress test); the decision note is waiting
    for his merge.
- **A Wi-Fi crash to fix before mainline ships:** a known RTL8821CS receive bug (our chip) that neither
  upstream nor ROCKNIX patches yet.
- **Ubuntu 24.04 track planned** ([plan](docs/kernel-research/ubuntu-track.md)): a newer userland in parallel
  with Beta, on a bench card only. Waiting on Dale's first decision.

### Completed
- **Phase 0, dissect stock:** partitions, boot chain, hardware map, stock userspace inventory.
- **Phase 1, stock-kernel baseline:** patched `boot.img` (5 ms input polling, `schedutil` governor,
  write barriers). Boots to the menu in about 20 s on the device (2026-11-10).
- **Frontend F0–F2 signed off:** UI spec, desktop prototype, launch parity with the stock menu.
- **Device-checked features:**
  - Game launching through stock RetroArch (PS1, Neo Geo, CPS, Saturn and others), PSP, and
    **MAME**; back to the same game list after quitting. Another 27 system folders are mapped;
    not every one has been run on the device
  - Wi-Fi setup with a full on-screen keyboard
  - BIOS-style UI with 10 colour presets (Amber checked; it survives a reboot)
  - **Boot screen modification:** the Redux boot logo, rendered in the stock format (640×480 BMP),
    installed to the boot-resource partition by the Boot Screen manager, which keeps a backup
  - System Tools (read-only), Engineering Mode tabs, Restart/Power Off with a 3 s hold,
    persistent volume, restore the stock menu from Settings
  - Apps list: Terminal, Task Monitor (htop), Music, and five Tools apps (Audio Mixer, Network
    Setup, Bluetooth, Input Test, Speaker Test). The stock File Manager opens from Main (it tears
    while loading)
  - Ports: known-ports table (PortMaster Wolfenstein tested)
  - `tools/extras.py`: optional Ubuntu extras (htop) from a checklist, one SSH line. It refuses
    any upgrade that could break RetroArch (the refusal is tested on the PC only)

### In progress (merged, waiting for a device check or a fix)
- **Video Player** and **OpenBOR**: both crash back to the menu; logging added to find the cause
- **Brightness** (Settings > Display): reworked after the first device test, re-check due
- Screen Timeout (Off, 1–30 min), TF2 ROM scan, Ports Card Scripts switch (default Off),
  returning to the Apps list after an app, File Manager on Main only, Ebook Reader quit button,
  in-game buttons in Ports (Y, X, Start, Select)
- Boot Screen **restore** (install is checked; restore not yet)
- Open device checks are listed in [`notes/STATUS.md`](notes/STATUS.md)

### Next (to reach Beta)
- F3 settings parity: UI font picker, Reset All
- F4 power and lid: sleep and wake on the lid, no lost saves over 50 lid cycles mid-game
- F5 Engineering Mode, inside the stock CPU frequency table
- Phase 1 measurements, stock vs. Redux: boot time, idle battery drain, temperature, button latency
- One opt-in installer for a stock card, with a documented way back to stock
- User docs; Beta exit test: one week of daily use with no lost save

### Future
- **1.0, mainline:** mainline kernel and u-boot, a read-only system that runs from RAM, lid wake,
  then the frontend checklists passed again on the new OS, and an install/update path
- **1.x:** ZRAM (compressed RAM swap) on mainline
- **Backlog:** Expert Mode (above-factory settings behind an unlock), PC link (browser file
  manager), chiptune composer, DOS Mode menu style, vsync rebuilds of stock apps that tear,
  hardware video decode, in-game HUD

## Hardware at a glance

| | |
|---|---|
| SoC | Allwinner H700 (H616 family, `sun50iw9`), 4× Cortex-A53 @ up to 1.512 GHz, Mali-G31 |
| RAM | 2 GB (the device tree's 512 MB memory node is a template that u-boot overwrites) |
| PMIC | X-Powers AXP2202 (battery fuel gauge, USB-C, power key) |
| Display | 720×480 parallel RGB, about 59.16 Hz (exactly 3× GBA, 3× GB vertical) |
| Lid | Hall sensor on **PE7** |
| Wi-Fi/BT | Realtek RTL8821CS (SDIO) |
| Stock kernel | Linux 4.9.170 (Allwinner BSP), built 2025-04-12 |

The details are in [`docs/hardware-map.md`](docs/hardware-map.md).

## Repo layout

```
AGENTS.md             rules for AI collaborators; they talk through notes/
notes/                dated hand-off notes between AIs + STATUS.md board
docs/                 hardware map, stock boot chain and userspace, kernel strategy, frontend brief
docs/kernel-research/  kernel, bootloader and Ubuntu research (desk only)
reference/stock-202505/  device tree source, kernel .config, u-boot env and
                      initramfs init script, extracted from Anbernic firmware
                      rg34xxsp-fw-sd_202505 (sources only, no binaries)
tools/                image tools (pure Python 3, no dependencies)
phase1-stock-tweaks/  baseline patch set on the stock kernel
phase2-mainline/      mainline kernel + rootfs build (planned)
```

## Tools

All of them are standard-library Python 3 and run on Windows, WSL or Linux.

| Tool | What it does |
|---|---|
| `tools/bootimg.py` | Unpack and repack Android boot images (v0–v2). Verified to rebuild the stock `boot.img` **byte-for-byte**. |
| `tools/initramfs.py` | Unpack and repack the gzip'd cpio initramfs, keeping ownership, modes and symlinks in a manifest so a Windows checkout can't lose them. |
| `tools/dt_keys.py` | Print the button → GPIO pin → Linux event map from a device tree. |
| `tools/fdt_patch.py` | Read or overwrite a u32 property in a compiled `.dtb` in place (no `dtc` needed). |
| `tools/stock_inventory.py` | Read-only inventory of `RG34XXSP.img` with no mounting: cuts the 36 MiB bootloader region, lists rootfs/appfs/UDISK through a built-in ext4 reader, and flags files that touch the CPU governor, kernel modules or power-key/lid handling. Never reads the Roms partition. |

```bash
python3 tools/bootimg.py unpack 3.boot.img work/boot
python3 tools/initramfs.py unpack work/boot/ramdisk work/initramfs
# ...edit...
python3 tools/initramfs.py pack work/initramfs work/boot/ramdisk
python3 tools/bootimg.py pack work/boot new-boot.img --pad-to 67108864
```

## Safety

- Only flash a **spare** SD card. Keep the stock card untouched.
- Nothing here writes to the device's bootloader region (boot0/u-boot in the first 36 MiB of the card).
- **Never commit ROMs, BIOS files, or the stock image itself.** `.gitignore` blocks the common cases.

## License

GPL-2.0. The reference device tree and kernel config come from GPL-2.0 kernel sources.

#ifndef REDUX_BRIGHTNESS_H
#define REDUX_BRIGHTNESS_H

/* Persistent display brightness.
   The backlight is set with the sunxi display driver: ioctl(/dev/disp, DISP_LCD_SET_BRIGHTNESS
   0x102, {screen 0, value 0-255}). Device-checked by Dale 2026-11-13: 40 dims, 200 brightens.
   The AXP2202 attribute /sys/class/power_supply/axp2202-battery/brightness only stores a number
   (writing it never changed the screen, Dale 2026-11-13); Redux still writes the level there too,
   in case a stock script reads it. Levels 1-20, never 0.
   Min, max and default live here so a later builder can widen the range once
   the real upper limit is known. Do not probe above BRIGHTNESS_MAX.
   Saved to /mnt/data/redux/brightness on the device, /tmp/redux.brightness on desktop.
   Desktop builds do not write sysfs unless a test hook is set. */

#define BRIGHTNESS_MIN     1
#define BRIGHTNESS_MAX     20
#define BRIGHTNESS_DEFAULT 5

typedef int (*brightness_write_fn)(int level);

/* Tests replace the sysfs write. NULL restores the real path (stock builds only). */
void brightness_set_write(brightness_write_fn fn);

/* Tests point at a temp file. NULL restores the build default. */
void brightness_set_path(const char *path);

/* Current level, BRIGHTNESS_MIN..BRIGHTNESS_MAX. Default BRIGHTNESS_DEFAULT until loaded. */
int brightness_get(void);

/* Step by dir (+1 or -1), clamp, apply, save. Returns the new level. */
int brightness_step(int dir);

/* Load the saved level if the file exists, then apply it. */
void brightness_load(void);

/* Re-apply the current level (startup). */
int brightness_apply(void);

/* Backlight value 0-255 for a level: 16 at level 1 (never dark), 255 at level 20. */
int brightness_disp_value(int level);

/* The level the last apply asked to write, or -1. For tests. */
int brightness_last_written(void);

#endif

/* SPDX-License-Identifier: GPL-2.0-only */
/* Host test double for osk_sys_t: records ioctls and writes, serves reads, never touches a real device. */
#ifndef FAKE_SYS_H
#define FAKE_SYS_H

#include <stdint.h>
#include <linux/input.h>
#include "osk_sys.h"
#include "sunxi_disp2_abi.h"

#define FAKE_MAX 256

typedef struct {
	unsigned long req;
	int           fd;
	long          int_arg;                  /* for EVIOCGRAB, UI_SET_*BIT */
	struct disp_layer_config2 cfg;          /* copy for DISP_LAYER_SET_CONFIG2 */
	unsigned long disp_arg[4];
} fake_ioctl_t;

typedef struct {
	/* behaviour */
	int  fail_open_path_ion;                /* /dev/ion open fails */
	int  legacy_alloc_fails;                /* ION legacy alloc -> ENOTTY, new ABI succeeds */
	int  grab_busy;                         /* EVIOCGRAB(1) -> EBUSY */
	int  scn_w, scn_h;
	const char *input_name;                 /* name returned for /dev/input/event3 */
	/* records */
	fake_ioctl_t ioctls[FAKE_MAX];
	int          n_ioctls;
	struct input_event writes[FAKE_MAX];
	int          n_writes;
	int          uinput_created, uinput_destroyed, sleeps;
	/* input queue for read() */
	struct input_event reads[FAKE_MAX];
	int          n_reads, read_pos;
	int          open_fds;
} fake_t;

extern fake_t fake;
const osk_sys_t *fake_sys(void);
void fake_reset(void);
int  fake_count(unsigned long req);
const fake_ioctl_t *fake_last(unsigned long req);

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #c); return 1; } } while (0)

#endif

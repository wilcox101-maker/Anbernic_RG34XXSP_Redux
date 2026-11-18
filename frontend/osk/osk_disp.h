/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * osk_disp (KR-2): the OSK's own display layer on the stock RG34XXSP kernel (sunxi disp2, /dev/disp).
 * One ARGB8888 layer on disp0 channel 3 (or 2), layer 0, zorder 30, above whatever app is on fb0 (channel 1).
 * Buffers come from ION (system heap, as the stock launcher does) and are drawn by the caller through osk_buf_t.px.
 * Evidence: H700 docs/r26-global-osk-stock-disp2.md sections 1 and 4. Nothing here has run on the device yet.
 *
 * Rules kept here (R26 section 4):
 *  - channels 0 (video) and 1 (fb0, launcher) are refused;
 *  - osk_disp_show() commits only when the rectangle or buffer changed, so a hidden or idle OSK never commits;
 *  - osk_disp_close() hides the layer if shown; /dev/disp keeps a layer after the process dies, so the owner also
 *    runs osk_disp_off_once() at start and after a crash (Redux plan D1).
 * Return values: 0 or a positive count on success, -errno on failure.
 */
#ifndef OSK_DISP_H
#define OSK_DISP_H

#include <stddef.h>
#include <stdint.h>
#include "osk_sys.h"

typedef struct osk_disp osk_disp_t;

typedef struct {
	int32_t  x, y;          /* top-left on the screen, pixels */
	uint32_t w, h;          /* size on the screen; may differ from the buffer (the UI channel scales) */
} osk_rect_t;

typedef struct {
	int       fd;           /* dma-buf fd (ION) */
	uint32_t *px;           /* ARGB8888, w * h words, row stride = w; not premultiplied */
	uint32_t  w, h;
	size_t    len;          /* mapped bytes (page-rounded) */
} osk_buf_t;

typedef struct {
	int            channel; /* 2 or 3; default 3 */
	int            zorder;  /* default 30 (must be above the launcher's 20) */
	const osk_sys_t *sys;   /* NULL = real system calls */
} osk_disp_opts_t;

#define OSK_DISP_DEFAULT_CHANNEL 3
#define OSK_DISP_DEFAULT_ZORDER  30

int  osk_disp_open(osk_disp_t **out, const osk_disp_opts_t *opts);
/* Screen 0 size from DISP_GET_SCN_WIDTH/HEIGHT; call again after an HDMI change. */
int  osk_disp_screen_size(osk_disp_t *d, uint32_t *w, uint32_t *h);
/* Added to the Owner's list: buffers need an owner. Allocates w x h ARGB8888 from ION and maps it. */
int  osk_disp_buf_new(osk_disp_t *d, uint32_t w, uint32_t h, osk_buf_t *out);
void osk_disp_buf_free(osk_disp_t *d, osk_buf_t *b);
/* Show buf at rect (commits only on change). For double buffering, draw into the other buffer and show it. */
int  osk_disp_show(osk_disp_t *d, osk_rect_t rect, const osk_buf_t *buf);
int  osk_disp_hide(osk_disp_t *d);
int  osk_disp_is_shown(const osk_disp_t *d);
void osk_disp_close(osk_disp_t *d);
/* One-shot "layer off" for `redux-osk --off` (no state, always commits). */
int  osk_disp_off_once(const osk_disp_opts_t *opts);
/* Bottom band helper: full screen width (or want_w if smaller), want_h high, centred, at the bottom. */
osk_rect_t osk_disp_rect_bottom(uint32_t screen_w, uint32_t screen_h, uint32_t want_w, uint32_t want_h);

#endif

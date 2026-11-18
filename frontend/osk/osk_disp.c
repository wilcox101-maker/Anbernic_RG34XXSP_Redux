// SPDX-License-Identifier: GPL-2.0-only
#define _POSIX_C_SOURCE 200809L
#include "osk_disp.h"
#include "sunxi_disp2_abi.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#define ION_IOC_ALLOC_LEGACY _IOWR('I', 0, struct ion_allocation_data_legacy)
#define ION_IOC_FREE_LEGACY  _IOWR('I', 1, struct ion_handle_data_legacy)
#define ION_IOC_SHARE_LEGACY _IOWR('I', 4, struct ion_fd_data_legacy)
#define ION_IOC_ALLOC_NEW    _IOWR('I', 0, struct ion_allocation_data_new)

struct osk_disp {
	const osk_sys_t *sys;
	int              disp;
	int              channel, zorder;
	int              shown;
	osk_rect_t       last_rect;
	int              last_fd;
};

static int neg_errno(void) { return errno ? -errno : -EIO; }

static int resolve_opts(const osk_disp_opts_t *o, const osk_sys_t **sys, int *ch, int *z)
{
	*sys = (o && o->sys) ? o->sys : osk_sys_default();
	*ch = (o && o->channel) ? o->channel : OSK_DISP_DEFAULT_CHANNEL;
	*z = (o && o->zorder) ? o->zorder : OSK_DISP_DEFAULT_ZORDER;
	if (*ch != 2 && *ch != 3)
		return -EINVAL;                 /* 0 = video, 1 = fb0 and the launcher */
	if (*z < 21 || *z > 255)
		return -EINVAL;                 /* must sit above the launcher's zorder 20 */
	return 0;
}

static int commit(const osk_sys_t *sys, int disp, struct disp_layer_config2 *cfg)
{
	unsigned long arg[4];
	arg[0] = 0;                             /* screen 0 */
	arg[1] = (unsigned long)cfg;
	arg[2] = 1;                             /* one layer */
	arg[3] = 0;
	errno = 0;
	if (sys->ioctl(disp, DISP_LAYER_SET_CONFIG2, arg) < 0)
		return neg_errno();
	return 0;
}

static int commit_off(const osk_sys_t *sys, int disp, int channel)
{
	struct disp_layer_config2 cfg;
	memset(&cfg, 0, sizeof cfg);
	cfg.enable = false;
	cfg.channel = (uint32_t)channel;
	cfg.layer_id = 0;
	return commit(sys, disp, &cfg);
}

int osk_disp_open(osk_disp_t **out, const osk_disp_opts_t *opts)
{
	const osk_sys_t *sys;
	int ch, z, r;
	osk_disp_t *d;

	if (!out)
		return -EINVAL;
	*out = NULL;
	r = resolve_opts(opts, &sys, &ch, &z);
	if (r < 0)
		return r;
	d = calloc(1, sizeof *d);
	if (!d)
		return -ENOMEM;
	d->sys = sys;
	d->channel = ch;
	d->zorder = z;
	d->last_fd = -1;
	errno = 0;
	d->disp = sys->open("/dev/disp", O_RDWR);
	if (d->disp < 0) {
		r = neg_errno();
		free(d);
		return r;
	}
	*out = d;
	return 0;
}

int osk_disp_screen_size(osk_disp_t *d, uint32_t *w, uint32_t *h)
{
	unsigned long arg[4] = { 0, 0, 0, 0 };
	int rw, rh;

	if (!d || !w || !h)
		return -EINVAL;
	rw = d->sys->ioctl(d->disp, DISP_GET_SCN_WIDTH, arg);
	rh = d->sys->ioctl(d->disp, DISP_GET_SCN_HEIGHT, arg);
	if (rw <= 0 || rh <= 0)
		return -EIO;
	*w = (uint32_t)rw;
	*h = (uint32_t)rh;
	return 0;
}

int osk_disp_buf_new(osk_disp_t *d, uint32_t w, uint32_t h, osk_buf_t *out)
{
	const osk_sys_t *sys;
	struct ion_allocation_data_legacy a;
	struct ion_allocation_data_new n;
	size_t len;
	int ion, fd = -1, r;

	if (!d || !out || w == 0 || h == 0 || w > 4096 || h > 4096)
		return -EINVAL;
	sys = d->sys;
	memset(out, 0, sizeof *out);
	out->fd = -1;
	len = ((size_t)w * h * 4u + 4095u) & ~(size_t)4095u;
	errno = 0;
	ion = sys->open("/dev/ion", O_RDONLY);
	if (ion < 0)
		return neg_errno();

	memset(&a, 0, sizeof a);
	a.len = len;
	a.align = 4096;
	a.heap_id_mask = 1;                     /* heap id 0: sys_user (ion_system), as the stock launcher */
	a.flags = 0;                            /* uncached: no cache maintenance needed before scan-out */
	if (sys->ioctl(ion, ION_IOC_ALLOC_LEGACY, &a) == 0) {
		struct ion_fd_data_legacy s;
		struct ion_handle_data_legacy f;
		s.handle = a.handle;
		s.fd = -1;
		r = sys->ioctl(ion, ION_IOC_SHARE_LEGACY, &s);
		f.handle = a.handle;
		sys->ioctl(ion, ION_IOC_FREE_LEGACY, &f); /* the dma-buf fd keeps the buffer alive */
		if (r < 0) {
			r = neg_errno();
			sys->close(ion);
			return r;
		}
		fd = s.fd;
	} else {
		memset(&n, 0, sizeof n);
		n.len = len;
		n.heap_id_mask = 1;
		n.flags = 0;
		if (sys->ioctl(ion, ION_IOC_ALLOC_NEW, &n) < 0) {
			r = neg_errno();
			sys->close(ion);
			return r;
		}
		fd = (int)n.fd;
	}
	sys->close(ion);

	out->px = sys->mmap_rw(len, fd);
	if (!out->px) {
		r = neg_errno();
		sys->close(fd);
		out->fd = -1;
		return r;
	}
	out->fd = fd;
	out->w = w;
	out->h = h;
	out->len = len;
	memset(out->px, 0, len);                /* fully transparent */
	return 0;
}

void osk_disp_buf_free(osk_disp_t *d, osk_buf_t *b)
{
	if (!d || !b || b->fd < 0)
		return;
	if (d->shown && d->last_fd == b->fd)
		osk_disp_hide(d);               /* never free the buffer on screen */
	if (b->px)
		d->sys->munmap(b->px, b->len);
	d->sys->close(b->fd);
	memset(b, 0, sizeof *b);
	b->fd = -1;
}

static int rect_eq(osk_rect_t a, osk_rect_t b)
{
	return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

int osk_disp_show(osk_disp_t *d, osk_rect_t rect, const osk_buf_t *buf)
{
	struct disp_layer_config2 cfg;
	int i, r;

	if (!d || !buf || buf->fd < 0 || rect.w == 0 || rect.h == 0)
		return -EINVAL;
	if (d->shown && d->last_fd == buf->fd && rect_eq(d->last_rect, rect))
		return 0;                       /* nothing changed: no commit */
	memset(&cfg, 0, sizeof cfg);
	cfg.info.mode = LAYER_MODE_BUFFER;
	cfg.info.zorder = (uint8_t)d->zorder;
	cfg.info.alpha_mode = 0;                /* per-pixel alpha */
	cfg.info.alpha_value = 0xff;
	cfg.info.screen_win.x = rect.x;
	cfg.info.screen_win.y = rect.y;
	cfg.info.screen_win.width = rect.w;
	cfg.info.screen_win.height = rect.h;
	cfg.info.u.fb.fd = buf->fd;
	for (i = 0; i < 3; i++) {
		cfg.info.u.fb.size[i].width = buf->w;
		cfg.info.u.fb.size[i].height = buf->h;
	}
	cfg.info.u.fb.format = DISP_FORMAT_ARGB_8888;
	cfg.info.u.fb.color_space = DISP_BT601;
	cfg.info.u.fb.pre_multiply = false;
	cfg.info.u.fb.crop.width = (int64_t)buf->w << 32;
	cfg.info.u.fb.crop.height = (int64_t)buf->h << 32;
	cfg.info.u.fb.flags = DISP_BF_NORMAL;
	cfg.info.u.fb.scan = DISP_SCAN_PROGRESSIVE;
	cfg.enable = true;
	cfg.channel = (uint32_t)d->channel;
	cfg.layer_id = 0;
	r = commit(d->sys, d->disp, &cfg);
	if (r < 0)
		return r;
	d->shown = 1;
	d->last_fd = buf->fd;
	d->last_rect = rect;
	return 1;                               /* committed */
}

int osk_disp_hide(osk_disp_t *d)
{
	int r;

	if (!d)
		return -EINVAL;
	if (!d->shown)
		return 0;                       /* already hidden: no commit */
	r = commit_off(d->sys, d->disp, d->channel);
	if (r < 0)
		return r;
	d->shown = 0;
	d->last_fd = -1;
	return 1;
}

int osk_disp_is_shown(const osk_disp_t *d) { return d ? d->shown : 0; }

void osk_disp_close(osk_disp_t *d)
{
	if (!d)
		return;
	osk_disp_hide(d);
	d->sys->close(d->disp);
	free(d);
}

int osk_disp_off_once(const osk_disp_opts_t *opts)
{
	const osk_sys_t *sys;
	int ch, z, disp, r;

	r = resolve_opts(opts, &sys, &ch, &z);
	if (r < 0)
		return r;
	errno = 0;
	disp = sys->open("/dev/disp", O_RDWR);
	if (disp < 0)
		return neg_errno();
	r = commit_off(sys, disp, ch);
	sys->close(disp);
	return r;
}

osk_rect_t osk_disp_rect_bottom(uint32_t screen_w, uint32_t screen_h, uint32_t want_w, uint32_t want_h)
{
	osk_rect_t r;
	uint32_t w = (want_w == 0 || want_w > screen_w) ? screen_w : want_w;
	uint32_t h = want_h > screen_h ? screen_h : want_h;
	r.w = w;
	r.h = h;
	r.x = (int32_t)((screen_w - w) / 2u);
	r.y = (int32_t)(screen_h - h);
	return r;
}

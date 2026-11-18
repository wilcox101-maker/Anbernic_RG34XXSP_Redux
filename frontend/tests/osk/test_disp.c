// SPDX-License-Identifier: GPL-2.0-only
/* Host tests for osk_disp (KR-2). No device access: every call goes to fake_sys. */
#include <stdio.h>
#include <string.h>
#include "fake_sys.h"
#include "osk_disp.h"

static osk_disp_opts_t opts(int ch, int z)
{
	osk_disp_opts_t o;
	o.channel = ch;
	o.zorder = z;
	o.sys = fake_sys();
	return o;
}

static int t_refuses_bad_channels(void)
{
	osk_disp_t *d = NULL;
	osk_disp_opts_t o;
	fake_reset();
	o = opts(1, 0);
	CHECK(osk_disp_open(&d, &o) < 0 && d == NULL);
	o = opts(0, 30);                        /* 0 means default (3), allowed */
	CHECK(osk_disp_open(&d, &o) == 0 && d != NULL);
	osk_disp_close(d);
	o = opts(4, 0);
	CHECK(osk_disp_open(&d, &o) < 0);
	o = opts(3, 20);                        /* must be above the launcher's 20 */
	CHECK(osk_disp_open(&d, &o) < 0);
	o = opts(1, 30);
	CHECK(osk_disp_off_once(&o) < 0);
	CHECK(fake_count(DISP_LAYER_SET_CONFIG2) == 0);
	return 0;
}

static int t_show_fields_and_dedupe(void)
{
	osk_disp_t *d;
	osk_buf_t b;
	osk_rect_t r;
	const fake_ioctl_t *c;
	uint32_t w, h;
	osk_disp_opts_t o = opts(0, 0);
	fake_reset();
	CHECK(osk_disp_open(&d, &o) == 0);
	CHECK(osk_disp_screen_size(d, &w, &h) == 0 && w == 720 && h == 480);
	CHECK(osk_disp_buf_new(d, 720, 200, &b) == 0);
	CHECK(b.fd == 20 && b.px != NULL && b.len == 577536u);   /* 720 x 200 x 4 rounded up to 4 KiB */
	r = osk_disp_rect_bottom(w, h, 0, 200);
	CHECK(r.x == 0 && r.y == 280 && r.w == 720 && r.h == 200);
	CHECK(osk_disp_show(d, r, &b) == 1);
	c = fake_last(DISP_LAYER_SET_CONFIG2);
	CHECK(c && c->disp_arg[0] == 0 && c->disp_arg[2] == 1 && c->disp_arg[3] == 0);
	CHECK(c->cfg.enable && c->cfg.channel == 3 && c->cfg.layer_id == 0);
	CHECK(c->cfg.info.mode == LAYER_MODE_BUFFER && c->cfg.info.zorder == 30);
	CHECK(c->cfg.info.alpha_mode == 0 && c->cfg.info.alpha_value == 0xff);
	CHECK(c->cfg.info.screen_win.y == 280 && c->cfg.info.screen_win.width == 720);
	CHECK(c->cfg.info.u.fb.fd == 20 && c->cfg.info.u.fb.format == DISP_FORMAT_ARGB_8888);
	CHECK(c->cfg.info.u.fb.size[0].width == 720 && c->cfg.info.u.fb.size[0].height == 200);
	CHECK(c->cfg.info.u.fb.crop.width == ((int64_t)720 << 32) && c->cfg.info.u.fb.crop.height == ((int64_t)200 << 32));
	/* same rect + buffer: no second commit */
	CHECK(osk_disp_show(d, r, &b) == 0);
	CHECK(fake_count(DISP_LAYER_SET_CONFIG2) == 1);
	/* moved (HDMI resize): commits */
	r = osk_disp_rect_bottom(1280, 720, 720, 200);
	CHECK(r.x == 280 && r.y == 520);
	CHECK(osk_disp_show(d, r, &b) == 1);
	CHECK(fake_count(DISP_LAYER_SET_CONFIG2) == 2);
	/* hide commits once, then not again */
	CHECK(osk_disp_hide(d) == 1);
	c = fake_last(DISP_LAYER_SET_CONFIG2);
	CHECK(!c->cfg.enable && c->cfg.channel == 3 && c->cfg.info.u.fb.fd == 0);
	CHECK(osk_disp_hide(d) == 0);
	CHECK(fake_count(DISP_LAYER_SET_CONFIG2) == 3);
	osk_disp_buf_free(d, &b);
	osk_disp_close(d);
	CHECK(fake_count(DISP_LAYER_SET_CONFIG2) == 3);  /* close while hidden: no commit */
	CHECK(fake.open_fds == 0);
	return 0;
}

static int t_close_hides_and_free_visible(void)
{
	osk_disp_t *d;
	osk_buf_t b;
	osk_disp_opts_t o = opts(2, 40);
	fake_reset();
	CHECK(osk_disp_open(&d, &o) == 0);
	CHECK(osk_disp_buf_new(d, 100, 10, &b) == 0);
	CHECK(b.len == 4096);
	CHECK(osk_disp_show(d, osk_disp_rect_bottom(720, 480, 100, 10), &b) == 1);
	CHECK(fake_last(DISP_LAYER_SET_CONFIG2)->cfg.channel == 2);
	CHECK(fake_last(DISP_LAYER_SET_CONFIG2)->cfg.info.zorder == 40);
	osk_disp_buf_free(d, &b);              /* the buffer on screen: hides first */
	CHECK(!osk_disp_is_shown(d));
	CHECK(!fake_last(DISP_LAYER_SET_CONFIG2)->cfg.enable);
	osk_disp_close(d);
	CHECK(fake.open_fds == 0);
	return 0;
}

static int t_ion_new_abi_fallback_and_off_once(void)
{
	osk_disp_t *d;
	osk_buf_t b;
	osk_disp_opts_t o = opts(0, 0);
	fake_reset();
	fake.legacy_alloc_fails = 1;
	CHECK(osk_disp_open(&d, &o) == 0);
	CHECK(osk_disp_buf_new(d, 64, 64, &b) == 0 && b.fd == 21);
	osk_disp_buf_free(d, &b);
	osk_disp_close(d);
	fake_reset();
	fake.fail_open_path_ion = 1;
	CHECK(osk_disp_open(&d, &o) == 0);
	CHECK(osk_disp_buf_new(d, 64, 64, &b) < 0 && b.fd == -1);
	osk_disp_close(d);
	fake_reset();
	CHECK(osk_disp_off_once(&o) == 0);
	CHECK(fake_count(DISP_LAYER_SET_CONFIG2) == 1);
	CHECK(!fake_last(DISP_LAYER_SET_CONFIG2)->cfg.enable && fake_last(DISP_LAYER_SET_CONFIG2)->cfg.channel == 3);
	CHECK(fake.open_fds == 0);
	return 0;
}

int main(void)
{
	int fails = 0;
	fails += t_refuses_bad_channels();
	fails += t_show_fields_and_dedupe();
	fails += t_close_hides_and_free_visible();
	fails += t_ion_new_abi_fallback_and_off_once();
	printf("test_disp: %s\n", fails ? "FAIL" : "ok");
	return fails != 0;
}

/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Minimal user-space view of the Allwinner sunxi disp2 layer ABI (DISP_LAYER_SET_CONFIG2) as used by the stock
 * RG34XXSP kernel 4.9.170 (V1.0.6).
 *
 * Origin: the layout follows Allwinner's GPL-2.0 BSP header include/video/sunxi_display2.h
 * (Copyright (c) Allwinnertech Co., Ltd.), as published in orangepi-xunlong/linux-orangepi,
 * branch orange-pi-4.9-sun50iw9, commit 0cd0547ea405. Only the types needed for one RGB buffer layer are kept;
 * field names follow that header. The offsets below were computed from the full BSP header and match the stock
 * launcher's 32-bit use (H700 repo docs/r26-global-osk-stock-disp2.md section 1.3).
 */
#ifndef OSK_SUNXI_DISP2_ABI_H
#define OSK_SUNXI_DISP2_ABI_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "osk_sys.h"

/* /dev/disp ioctl numbers (plain numbers, not _IOC encoded); argument: unsigned long arg[4] */
#define DISP_GET_SCN_WIDTH       0x07u
#define DISP_GET_SCN_HEIGHT      0x08u
#define DISP_LAYER_SET_CONFIG2   0x49u
#define DISP_LAYER_GET_CONFIG2   0x4Au

#define DISP_FORMAT_ARGB_8888    0x00u
#define LAYER_MODE_BUFFER        0u
#define DISP_BT601               0x104u   /* value the stock launcher uses below 720 px wide */
#define DISP_BF_NORMAL           0u
#define DISP_SCAN_PROGRESSIVE    0u

struct disp_rect   { int32_t x, y; uint32_t width, height; };
struct disp_rectsz { uint32_t width, height; };
struct disp_rect64 { int64_t x, y, width, height; };

struct disp_fb_info2 {
	int32_t            fd;
	struct disp_rectsz size[3];
	uint32_t           align[3];
	uint32_t           format;          /* enum disp_pixel_format */
	uint32_t           color_space;     /* enum disp_color_space */
	int32_t            trd_right_fd;
	bool               pre_multiply;
	struct disp_rect64 crop;            /* 32.32 fixed point */
	uint32_t           flags;           /* enum disp_buffer_flags */
	uint32_t           scan;            /* enum disp_scan_flags */
	uint32_t           eotf;            /* enum disp_eotf */
	int32_t            depth;
	uint32_t           fbd_en;
	int32_t            metadata_fd;
	uint32_t           metadata_size;
	uint32_t           metadata_flag;
};

struct disp_atw_info {
	bool     used;
	uint32_t mode;                      /* enum disp_atw_mode */
	uint32_t b_row;
	uint32_t b_col;
	int32_t  cof_fd;
};

struct disp_snr_info {
	uint8_t          en;
	uint8_t          demo_en;
	struct disp_rect demo_win;
	uint8_t          y_strength, u_strength, v_strength;
	uint8_t          th_ver_line, th_hor_line;
};

struct disp_layer_info2 {
	uint32_t         mode;              /* enum disp_layer_mode */
	uint8_t          zorder;
	uint8_t          alpha_mode;        /* 0 pixel, 1 global, 2 pixel x global */
	uint8_t          alpha_value;
	struct disp_rect screen_win;
	bool             b_trd_out;
	uint32_t         out_trd_mode;      /* enum disp_3d_out_mode */
	union {
		uint32_t             color;
		struct disp_fb_info2 fb;
	} u;                                /* anonymous in the BSP header; named for C99 */
	uint32_t             id;
	struct disp_atw_info atw;
	uint32_t             transform;     /* enum disp_transform */
	struct disp_snr_info snr;
};

struct disp_layer_config2 {
	struct disp_layer_info2 info;
	bool     enable;
	uint32_t channel;
	uint32_t layer_id;
};

/* Layout checks (same values on aarch64 and armhf; R26 section 1.3). C99 copy of tools/oskprobe/sunxi_disp2_abi.h. */
OSK_ASSERT(fb_size, sizeof(struct disp_fb_info2) == 120);
OSK_ASSERT(info_size, sizeof(struct disp_layer_info2) == 208);
OSK_ASSERT(cfg_size, sizeof(struct disp_layer_config2) == 224);
OSK_ASSERT(zorder, offsetof(struct disp_layer_info2, zorder) == 4);
OSK_ASSERT(screen_win, offsetof(struct disp_layer_info2, screen_win) == 8);
OSK_ASSERT(fb, offsetof(struct disp_layer_info2, u.fb) == 32);
OSK_ASSERT(id, offsetof(struct disp_layer_info2, id) == 152);
OSK_ASSERT(snr, offsetof(struct disp_layer_info2, snr) == 180);
OSK_ASSERT(format, offsetof(struct disp_fb_info2, format) == 40);
OSK_ASSERT(pre_multiply, offsetof(struct disp_fb_info2, pre_multiply) == 52);
OSK_ASSERT(crop, offsetof(struct disp_fb_info2, crop) == 56);
OSK_ASSERT(metadata_flag, offsetof(struct disp_fb_info2, metadata_flag) == 116);
OSK_ASSERT(enable, offsetof(struct disp_layer_config2, enable) == 208);
OSK_ASSERT(channel, offsetof(struct disp_layer_config2, channel) == 212);
OSK_ASSERT(layer_id, offsetof(struct disp_layer_config2, layer_id) == 216);

/*
 * Stock ION (legacy, handle based, linux 4.9 staging ABI; stock kernel has ion_handle_create, R26 section 1.2).
 * Stock launcher: heap mask 1 (heap id 0 = sys_user, ion_system), 4 KiB alignment.
 */
struct ion_allocation_data_legacy {
	size_t   len;
	size_t   align;
	uint32_t heap_id_mask;
	uint32_t flags;
	int32_t  handle;
};
struct ion_fd_data_legacy     { int32_t handle; int32_t fd; };
struct ion_handle_data_legacy { int32_t handle; };
/* Newer (4.12+) ABI, which the stock launcher tries second */
struct ion_allocation_data_new {
	uint64_t len;
	uint32_t heap_id_mask;
	uint32_t flags;
	uint32_t fd;
	uint32_t unused;
};

#endif /* OSK_SUNXI_DISP2_ABI_H */

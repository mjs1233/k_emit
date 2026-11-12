//
// Created by takakura on 11/7/26.
//

#ifndef KURUMI_EMIT_DRM_ABI_H
#define KURUMI_EMIT_DRM_ABI_H
#include <linux/types.h>
struct drm_get_cap {
    __u64 capability;
    __u64 value;
};

struct drm_mode_card_res {
    __u64 fb_id_ptr;
    __u64 crtc_id_ptr;
    __u64 connector_id_ptr;
    __u64 encoder_id_ptr;
    __u32 count_fbs;
    __u32 count_crtcs;
    __u32 count_connectors;
    __u32 count_encoders;
    __u32 min_width, max_width;
    __u32 min_height, max_height;
};

struct drm_mode_modeinfo {
    __u32 clock;
    __u16 hdisplay, hsync_start, hsync_end, htotal, hskew;
    __u16 vdisplay, vsync_start, vsync_end, vtotal, vscan;
    __u32 vrefresh;
    __u32 flags;
    __u32 type;
    char name[32];
};

struct drm_mode_get_connector {
    __u64 encoders_ptr;
    __u64 modes_ptr;
    __u64 props_ptr;
    __u64 prop_values_ptr;

    __u32 count_modes;
    __u32 count_props;
    __u32 count_encoders;

    __u32 encoder_id;
    __u32 connector_id;
    __u32 connector_type;
    __u32 connector_type_id;

    __u32 connection;
    __u32 mm_width;
    __u32 mm_height;
    __u32 subpixel;
    __u32 pad;
};

struct drm_mode_get_encoder {
    __u32 encoder_id;
    __u32 encoder_type;
    __u32 crtc_id;
    __u32 possible_crtcs;
    __u32 possible_clones;
};

struct drm_mode_create_dumb {
    __u32 height;
    __u32 width;
    __u32 bpp;
    __u32 flags;
    __u32 handle;
    __u32 pitch;
    __u32 size;
};
struct drm_mode_destroy_dumb {
    __u32 handle;
};

struct drm_mode_map_dumb {
    __u32 handle;
    __u32 pad;
    __u32 offset;
};
#endif //KURUMI_EMIT_DRM_ABI_H

//
// Created by takakura on 11/7/26.
//

#ifndef KURUMI_EMIT_DRI_CARD_H
#define KURUMI_EMIT_DRI_CARD_H
#include <stdint.h>
#include "drm_abi.h"
typedef struct {
    char path[128];
    uint32_t connector_id;
    uint32_t encoder_id;
    uint32_t crtc_id;
    int fd;
    struct drm_mode_modeinfo mode;
} dri_card;

int open_dri_card(dri_card* card);
void close_dri_card(dri_card* card);

#endif //KURUMI_EMIT_DRI_CARD_H

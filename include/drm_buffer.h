//
// Created by takakura on 11/10/26.
//

#ifndef KURUMI_EMIT_DRM_BUFFER_H
#define KURUMI_EMIT_DRM_BUFFER_H
#include "drm_abi.h"
#include "dri_card.h"

typedef struct {
    struct drm_mode_create_dumb create;
    void* data;
} drm_buffer;

int create_drm_buffer(drm_buffer *buffer,dri_card *card);
int destroy_drm_buffer(drm_buffer *buffer,dri_card *card);
#endif //KURUMI_EMIT_DRM_BUFFER_H

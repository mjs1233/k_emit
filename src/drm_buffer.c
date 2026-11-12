//
// Created by takakura on 11/10/26.
//
#include "drm_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "drm_io.h"
#define DRM_OPEN_DUMB_BUFFER 0xb2
#define DRM_DESTROY_DUMB_BUFFER 0xb4

int create_drm_buffer(drm_buffer *buffer,dri_card *card) {

    struct drm_mode_create_dumb buf = {
        .width = card->mode.hdisplay,
        .height = card->mode.vdisplay,
        .bpp = 32
    };

    if (ioctl(card->fd, drm_iowr(DRM_OPEN_DUMB_BUFFER, sizeof(buf)), &buf) < 0) {
        perror("CREATE_DUMB");
        return -1;
    }

    printf("handle: %u\n", buf.handle);
    printf("pitch:  %u\n", buf.pitch);
    printf("size:   %lu\n", (unsigned long)buf.size);
    buffer->create = buf;

    struct drm_mode_map_dumb map = {
        .handle = buf.handle
    };

    if (ioctl(card->fd, drm_iowr(0xB3, sizeof(map)), &map) < 0)
        return -1;

    buffer->data = mmap(
        NULL,
        (size_t)buf.size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        card->fd,
        (off_t)map.offset
    );

}

int destroy_drm_buffer(drm_buffer *buffer,dri_card *card) {

    munmap(buffer->data, (size_t)buffer->create.size);

    struct drm_mode_destroy_dumb arg = {
        .handle = buffer->create.handle
    };

    return ioctl(card->fd, drm_iowr(DRM_DESTROY_DUMB_BUFFER, sizeof(arg)), &arg);
}
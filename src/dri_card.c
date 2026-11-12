#include "dri_card.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/types.h>

#include "drm_io.h"
#include "drm_abi.h"

#define DRM_CAP_DUMB_BUFFER     0x1
#define DRM_GET_CAP             0x0c
#define DRM_GET_RES             0xa0
#define DRM_GETCONNECTOR        0xa7
#define DRM_GETENCODER          0xa6

#define CARD_DIR "/dev/dri"

static int fill_card_properties(const char* device_path,dri_card* card);
static bool check_drm_cap(int fd, const char* device_path);
static bool check_drm_res(int fd, const char* device_path, uint32_t* connector_id);
static bool select_drm_encoder(int fd, dri_card* card);

int open_dri_card(dri_card* card) {

    DIR *dir = opendir(CARD_DIR);
    if (dir == NULL) {
        printf("fail to open %s\n", CARD_DIR);
        return -1;
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        if (strncmp(entry->d_name, "card", 4) != 0)
            continue;

        char device_path[128] = {0,};
        strcpy(device_path, CARD_DIR);
        strcat(device_path, "/");
        strcat(device_path, entry->d_name);

        if (fill_card_properties(device_path,card) < 0)
            continue;

        strncpy(card->path, device_path,128);
    }

    closedir(dir);
    return 0;

}

int fill_card_properties(const char* device_path,dri_card* card) {

    int fd = open(device_path, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        return -1;
    }

    if (!check_drm_cap(fd, device_path)) {
        close(fd);
        return -2;
    }

    if (!check_drm_res(fd, device_path, &card->connector_id)) {
        close(fd);
        return -3;
    }

    if (!select_drm_encoder(fd,card)) {
        close(fd);
        return -4;
    }

    card->fd = fd;

    return 0;
}


bool check_drm_cap(int fd, const char* device_path) {

    struct drm_get_cap cap = {
        .capability = DRM_CAP_DUMB_BUFFER
    };

    if (ioctl(fd, drm_iowr(DRM_GET_CAP,sizeof(cap)), &cap) < 0) {
        close(fd);
        return false;
    }
    return true;
}


bool check_drm_res(int fd, const char* device_path, uint32_t* connector_id) {

    struct drm_mode_card_res res = {0,};

    if (ioctl(fd, drm_iowr(DRM_GET_RES, sizeof(res)), &res) < 0) {
        return false;
    }

    if (res.count_connectors == 0 || res.count_crtcs == 0) {
        return false;
    }

    uint32_t capacity = res.count_connectors;

    uint32_t *ids = calloc(capacity, sizeof(*ids));
    if (!ids) {
        return false;
    }

    res.connector_id_ptr = (uintptr_t)ids;
    res.count_connectors = capacity;

    res.count_fbs = 0;
    res.count_crtcs = 0;
    res.count_encoders = 0;

    if (ioctl(fd, drm_iowr(DRM_GET_RES, sizeof(res)), &res) < 0) {
        free(ids);
        return false;
    }

    //device config change ..?
    if (res.count_connectors > capacity) {
        free(ids);
        return false;
    }


    for (uint32_t i = 0; i < res.count_connectors; i++) {
        struct drm_mode_modeinfo temporary_mode = {0};

        struct drm_mode_get_connector conn = {
            .connector_id = ids[i],
            .count_modes = 1,
            .modes_ptr = (uintptr_t)&temporary_mode
        };

        if (ioctl(fd, drm_iowr(DRM_GETCONNECTOR, sizeof(conn)), &conn) < 0) {
            continue;
        }

        if (conn.connection == 1) {
            *connector_id = conn.connector_id;
        }
    }

    free(ids);

    return true;
}

#define DRM_ENCODER_CLEAN_UP \
free(crtcs);    \
free(encoders); \
free(modes);    \

#define DRM_MODE_TYPE_PREFERRED 0x8

bool select_drm_encoder(int fd, dri_card* card) {

    int result = -1;
    uint32_t* crtcs = NULL;
    uint32_t* encoders = NULL;
    struct drm_mode_modeinfo* modes = NULL;

    //get id list
    struct drm_mode_card_res res = {0};

    if (ioctl(fd, drm_iowr(DRM_GET_RES, sizeof(res)), &res) < 0) {
        DRM_ENCODER_CLEAN_UP
        return false;
    }

    uint32_t crtc_cap = res.count_crtcs;
    if (!crtc_cap) {
        errno = ENODEV;
        DRM_ENCODER_CLEAN_UP
        return false;
    }

    crtcs = calloc(crtc_cap, sizeof(*crtcs));
    if (!crtcs) {
        DRM_ENCODER_CLEAN_UP
        return false;
    }

    res = (struct drm_mode_card_res){
        .crtc_id_ptr = (uint64_t)(uintptr_t)crtcs,
        .count_crtcs = crtc_cap
    };

    if (ioctl(fd, drm_iowr(DRM_GET_RES, sizeof(res)), &res) < 0) {
        DRM_ENCODER_CLEAN_UP
        return false;
    }

    if (res.count_crtcs > crtc_cap) {
        errno = EAGAIN;
        DRM_ENCODER_CLEAN_UP
        return false;
    }

    uint32_t crtc_count = res.count_crtcs;

    // get encoder, mode count
    struct drm_mode_modeinfo temp_mode = {0};
    struct drm_mode_get_connector conn = {
        .connector_id =  card->connector_id,
        .modes_ptr = (uintptr_t)&temp_mode,
        .count_modes = 1
    };

    if (ioctl(fd, drm_iowr(DRM_GETCONNECTOR, sizeof(conn)), &conn) < 0){
        DRM_ENCODER_CLEAN_UP
        return false;
    }

    if (conn.connection != 1 || !conn.count_modes || !conn.count_encoders) {
        errno = ENODEV;
        DRM_ENCODER_CLEAN_UP
        return false;
    }

    uint32_t mode_cap = conn.count_modes;
    uint32_t enc_cap = conn.count_encoders;

    modes = calloc(mode_cap, sizeof(*modes));
    encoders = calloc(enc_cap, sizeof(*encoders));

    if (!modes || !encoders){
        DRM_ENCODER_CLEAN_UP
        return false;
    }

    // get encoder ID, mode list
    conn = (struct drm_mode_get_connector){
        .connector_id =  card->connector_id,
        .modes_ptr = (uint64_t)(uintptr_t)modes,
        .count_modes = mode_cap,
        .encoders_ptr = (uint64_t)(uintptr_t)encoders,
        .count_encoders = enc_cap
    };

    if (ioctl(fd, drm_iowr(DRM_GETCONNECTOR, sizeof(conn)), &conn) < 0){
        DRM_ENCODER_CLEAN_UP
        return false;
    }

    if (conn.connection != 1 || conn.count_modes > mode_cap || conn.count_encoders > enc_cap) {
        errno = EAGAIN;
        DRM_ENCODER_CLEAN_UP
        return false;
    }

    //check 2 times for using enabled crtcs first and trying possible one.
    for (int pass = 0; pass < 2; ++pass) {
        for (uint32_t e = 0; e < conn.count_encoders; ++e) {
            struct drm_mode_get_encoder enc = {
                .encoder_id = encoders[e]
            };

            if (ioctl(fd, drm_iowr(DRM_GETENCODER, sizeof(enc)), &enc) < 0)
                continue;

            //check bit index.
            //
            for (uint32_t i = 0; i < crtc_count && i < 32; ++i) {
                if (!(enc.possible_crtcs & (1u << i)))
                    continue;

                if (pass == 0 && enc.crtc_id != crtcs[i])
                    continue;

                card->encoder_id = enc.encoder_id;
                card->crtc_id = crtcs[i];

                //select preferred mode
                uint32_t chosen = 0;
                for (uint32_t mode_idx = 0; mode_idx < conn.count_modes; ++mode_idx) {
                    if (modes[mode_idx].type & DRM_MODE_TYPE_PREFERRED) {
                        chosen = mode_idx;
                        break;
                    }
                }

                card->mode = modes[chosen];
                result = 0;
                DRM_ENCODER_CLEAN_UP
                return true;
            }
        }
    }

    errno = ENODEV;
    DRM_ENCODER_CLEAN_UP
    return false;
}


void close_dri_card(dri_card* card) {
    close(card->fd);
}
#include <stdio.h>
#include <stdlib.h>
#include "dri_card.h"
#include "drm_buffer.h"


int main(int argc, char *argv[]) {
    dri_card card;
    printf("%d\n", open_dri_card(&card));
    printf("dri_card: %s\n", card.path);
    printf("conn: %d\n", card.connector_id);
    printf("crtc: %d\n", card.crtc_id);
    printf("encoder: %d\n", card.encoder_id);
    printf("clk: %d\n", card.mode.clock);
    printf("mode: %s\n", card.mode.name);
    printf("resolution: %u x %u\n",
           card.mode.hdisplay,
           card.mode.vdisplay);

    printf("refresh: %u Hz\n",
           card.mode.vrefresh);

    drm_buffer buffer = {};

    create_drm_buffer(&buffer,&card);
    destroy_drm_buffer(&buffer,&card);
    close_dri_card(&card);

}
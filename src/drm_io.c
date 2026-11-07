#include "drm_io.h"
#include <sys/ioctl.h>

unsigned long drm_iowr(unsigned int nr, size_t size) {
        return _IOC(_IOC_READ | _IOC_WRITE, 'd', nr, size);
}

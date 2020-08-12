#ifndef _ME_CAMERA_LOOP_H_
#define _ME_CAMERA_LOOP_H_

#include "vidi.h"

void camera_loop(size_t r, size_t c, const char* cam_path, void (*process)(size_t r, size_t c, vidi_rgb_t frame[r][c]));


#endif
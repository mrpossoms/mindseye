#include "camera_loop.h"
#include "common.h"

void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{
	vidi_rgb_t o[r][c];
	memcpy(o, frame, sizeof(vidi_rgb_t) * r * c);

	point_t frame_dim = {r, c};

	me_dc_dx(frame_dim, o, frame);
	me_dc_dy(frame_dim, o, frame);

	me_bias(frame_dim, frame, frame, (vidi_rgb_t){ 0, 32, 0}, (win_t){ 25, 25, 25, 25}); 
}


int main (int argc, const char* argv[])
{
	camera_loop(480, 640, argv[1], process);

	return 0;
}
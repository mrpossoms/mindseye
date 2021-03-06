#include "camera_loop.h"
#include "common.h"

void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{
	short o[r][c][3];
	short f[r][c][3];
	short v[r][c][3];

	dim_t frame_dim = {r, c, 3};

	me_rgb_to_MTYPE(frame_dim, frame, o);
	memset(f, 0, (sizeof(short) * 3) * r * c);

	me_dc_dx(frame_dim, o, f);
	me_dc_dy(frame_dim, o, f);

	me_kernel_variance(frame_dim, f, v, (dim_t){3, 3});
	me_scl_f(frame_dim, v, v, (float[3]){ 1/25.f, 1/25.f, 1/25.f});

	// me_bias(frame_dim, f, f, (short[3]){ 128, 128, 128}, (win_t){ 0, 0, c, r}); 

	me_MTYPE_to_rgb(frame_dim, v, frame);
}


int main (int argc, const char* argv[])
{
	camera_loop(480, 640, argv[1], process);

	return 0;
}
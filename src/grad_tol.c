#include "camera_loop.h"
#include "common.h"

void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{
	short full[r][c][3];

	dim_t frame_dim = {r, c, 3};
	dim_t ds_dim = {r>>0, c>>0, 3};

	short o[ds_dim.r][ds_dim.c][ds_dim.d];
	short f[ds_dim.r][ds_dim.c][ds_dim.d];

	me_rgb_to_short(frame_dim, frame, full);
	me_downsample(frame_dim, full, ds_dim, o);
	memset(f, 0, (sizeof(short) * ds_dim.d) * ds_dim.r * ds_dim.c);

	me_dc_dx(frame_dim, o, f);
	me_dc_dy(frame_dim, o, f);

	// square the gradients
	// me_mul(ds_dim, f, f, f);

	me_abs(ds_dim, f, f);
	for (int r = frame_dim.r; r--;)
	for (int c = frame_dim.c; c--;)
	{
		int sum = 0;

		for (int d = frame_dim.d; d--;)
		sum += f[r][c][d];

		if (sum < 100) { memset(f[r][c], 0, sizeof(short) * 3); }				
		else { for (int d = frame_dim.d; d--;) f[r][c][d] = 255; }
	}


	me_bias(ds_dim, f, f, (short[3]){ 32, 32, 32}, (win_t){ 0, 0, c, r}); 
	
	// short full_mask[r][c][3];
	// me_upsample(ds_dim, f, frame_dim, full_mask);
	// me_mask(frame_dim, full, full, full_mask, 0);	

	me_upsample(ds_dim, f, frame_dim, full);
	me_short_to_rgb(frame_dim, full, frame);
}


int main (int argc, const char* argv[])
{
	camera_loop(480, 640, argv[1], process);

	return 0;
}
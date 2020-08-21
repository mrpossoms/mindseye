#include "camera_loop.h"
#include "common.h"

// short LAST[640>>1][480>>1][3];

void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{
	
	dim_t f_dim = {r, c, 3};
	dim_t hf_dim = {r>>1, c>>1, 3};

	short full[r][c][3];
	short ds[hf_dim.r][hf_dim.c][hf_dim.d];
	me_rgb_to_short(f_dim, frame, full);
	me_downsample(f_dim, full, hf_dim, ds);	

	short dc_dx[hf_dim.r][hf_dim.c][hf_dim.d];
	short dc_dy[hf_dim.r][hf_dim.c][hf_dim.d];
	memset(dc_dx, 0, (sizeof(short) * 3) * hf_dim.r * hf_dim.c);
	memset(dc_dy, 0, (sizeof(short) * 3) * hf_dim.r * hf_dim.c);


	dim_t k_s = {3, 3, 1};

	float k_dx[3][3][1] = {
		{ -1,  0,  1 },
		{ -1,  0,  1 },
		{ -1,  0,  1 },
	};
	me_convolve(hf_dim, ds, dc_dx, k_s, k_dx);

	float k_dy[3][3][1] = {
		{ -1, -1, -1 },
		{  0,  0,  0 },
		{  1,  1,  1 },
	};
	me_convolve(hf_dim, ds, dc_dy, k_s, k_dy);


	// me_dc_dx(hf_dim, ds, dc_dx);
	// me_dc_dy(hf_dim, ds, dc_dy);

	// me_sub(hf_dim, LAST, g, o);
	
	// me_abs(hf_dim, o, m);
	// me_mask(hf_dim, o, f, m, 32);

	// me_bias(hf_dim, f, f, (short[3]){ 128, 128, 128}, (win_t){ 0, 0, c, r}); 

	short f[hf_dim.r][hf_dim.c][hf_dim.d];
	me_mul(hf_dim, dc_dx, dc_dy, f);

	me_upsample(hf_dim, dc_dx, f_dim, full);
	me_short_to_rgb(f_dim, full, frame);
}


int main (int argc, const char* argv[])
{
	camera_loop(480, 640, argv[1], process);

	return 0;
}
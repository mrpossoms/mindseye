#include "camera_loop.h"
#include "common.h"

MTYPE LAST[640>>1][480>>1][3];

void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{
	dim_t hf_dim = {r>>1, c>>1, 3};
	MTYPE o[hf_dim.r][hf_dim.c][3];
	MTYPE f[hf_dim.r][hf_dim.c][3];
	MTYPE m[hf_dim.r][hf_dim.c][3];

	dim_t f_dim = {r, c, 3};


	MTYPE full[r][c][3];
	me_rgb_to_MTYPE(f_dim, frame, full);
	me_downsample(f_dim, full, hf_dim, f);	


	dim_t k_s = {3, 3, 1};
	MTYPE k_gauss[3][3][1] = {
		{  1,  2,  1 },
		{  2,  4,  2 },
		{  1,  2,  1 },
	};
	me_convolve(hf_dim, f, o, k_s, k_gauss);
	float gauss_norm[3] = { 1/16.f, 1/16.f, 1/16.f, };
	me_scl_f(hf_dim, o, o, gauss_norm);


	MTYPE g[hf_dim.r][hf_dim.c][3];
	memset(g, 0, (sizeof(MTYPE) * 3) * hf_dim.r * hf_dim.c);
	me_dc_dx(hf_dim, o, g);
	me_dc_dy(hf_dim, o, g);

	me_sub(hf_dim, LAST, o, m);
	me_mul(hf_dim, g, m, g);
	me_abs(hf_dim, g, g);
	// me_mask(hf_dim, o, g, m, 32);

	// me_bias(hf_dim, f, f, (MTYPE[3]){ 128, 128, 128}, (win_t){ 0, 0, c, r}); 

	me_add(hf_dim, g, m, m);

	me_upsample(hf_dim, m, f_dim, full);
	me_MTYPE_to_rgb(f_dim, full, frame);

	memcpy(LAST, o, sizeof(LAST));
}


int main (int argc, const char* argv[])
{
	camera_loop(480, 640, argv[1], process);

	return 0;
}
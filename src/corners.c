#include "camera_loop.h"

#define MTYPE float

#include "common.h"

// MTYPE LAST[640>>1][480>>1][3];

void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{
	
	dim_t f_dim = {r, c, 3};
	dim_t hf_dim = {r>>1, c>>1, 3};

	MTYPE full[r][c][3];
	MTYPE ds[hf_dim.r][hf_dim.c][hf_dim.d];
	MTYPE ds_t[hf_dim.r][hf_dim.c][hf_dim.d];
	me_rgb_to_MTYPE(f_dim, frame, full);
	me_downsample(f_dim, full, hf_dim, ds_t);	

	MTYPE dc_dx[hf_dim.r][hf_dim.c][hf_dim.d];
	MTYPE dc_dy[hf_dim.r][hf_dim.c][hf_dim.d];
	memset(dc_dx, 0, (sizeof(MTYPE) * 3) * hf_dim.r * hf_dim.c);
	memset(dc_dy, 0, (sizeof(MTYPE) * 3) * hf_dim.r * hf_dim.c);


	dim_t k_s = {3, 3, 1};

	MTYPE k_gauss[3][3][1] = {
		{  1,  2,  1 },
		{  2,  4,  2 },
		{  1,  2,  1 },
	};
	me_convolve(hf_dim, ds_t, ds, k_s, k_gauss);
	float gauss_norm[3] = { 1/16.f, 1/16.f, 1/16.f, };
	me_scl_f(hf_dim, ds, ds, gauss_norm);


	MTYPE k_dx[3][3][1] = {
		{ -1,  0,  1 },
		{ -2,  0,  2 },
		{ -1,  0,  1 },
	};
	me_dc_dx(hf_dim, ds, dc_dx);
	// me_convolve(hf_dim, ds, dc_dx, k_s, k_dx);
	me_abs(hf_dim, dc_dx, dc_dx);

	MTYPE k_dy[3][3][1] = {
		{ -1, -2, -1 },
		{  0,  0,  0 },
		{  1,  2,  1 },
	};
	me_dc_dy(hf_dim, ds, dc_dy);
	// me_convolve(hf_dim, ds, dc_dy, k_s, k_dy);
	me_abs(hf_dim, dc_dy, dc_dy);

	// me_bias(hf_dim, dc_dx, dc_dx, (MTYPE[3]){ -100, -100, -100}, (win_t){ 0, 0, hf_dim.c, hf_dim.r});
	// me_bias(hf_dim, dc_dy, dc_dy, (MTYPE[3]){ -100, -100, -100}, (win_t){ 0, 0, hf_dim.c, hf_dim.r});

	// me_clamp(hf_dim, dc_dx, dc_dx, (MTYPE[3]){0, 0, 0}, (MTYPE[3]){1000, 1000, 1000});
	// me_clamp(hf_dim, dc_dy, dc_dy, (MTYPE[3]){0, 0, 0}, (MTYPE[3]){1000, 1000, 1000});



	// me_sub(hf_dim, LAST, g, o);
	
	// me_abs(hf_dim, o, m);
	// me_mask(hf_dim, o, f, m, 32);

	// me_bias(hf_dim, f, f, (MTYPE[3]){ 128, 128, 128}, (win_t){ 0, 0, c, r}); 

	// MTYPE f[hf_dim.r][hf_dim.c][hf_dim.d];
	me_mul(hf_dim, dc_dx, dc_dy, ds);


	// me_bias(hf_dim, ds, ds, (MTYPE[3]){ -1256, -1256, -1256}, (win_t){ 0, 0, hf_dim.c, hf_dim.r}); 
	// me_clamp(hf_dim, ds, ds, (MTYPE[3]){0, 0, 0}, (MTYPE[3]){1000, 1000, 1000});


	MTYPE max[3] = { 1000, 1000, 1000};
	// me_max(hf_dim, ds, max);
	printf("max {%f, %f, %f}\n", max[0], max[1], max[2]);

	float max_f[3] = { 255.f / (float)max[0], 255.f / (float)max[1], 255.f / (float)max[2] };
	me_scl_f(hf_dim, ds, ds, max_f);

	// me_bias(hf_dim, f, f, (MTYPE[3]){ -100, -100, -100}, (win_t){ 0, 0, hf_dim.c, hf_dim.r});
	// me_clamp(hf_dim, f, f, (MTYPE[3]){0, 0, 0}, (MTYPE[3]){1000, 1000, 1000});

	me_upsample(hf_dim, ds, f_dim, full);
	me_MTYPE_to_rgb(f_dim, full, frame);
}


int main (int argc, const char* argv[])
{
	camera_loop(480, 640, argv[1], process);

	return 0;
}
#include "camera_loop.h"
#include "common.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define F_SIZE 7

match_t last_match;
MTYPE feature[3][F_SIZE][F_SIZE][3];

MTYPE LAST[640 >> 1][480 >> 1][3];

void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{
	static int frames;

	const dim_t frame_dim = {r, c, 3};
	const dim_t ds_dim = {r >> 1, c >> 1, 3};
	const dim_t feat_dim = {F_SIZE, F_SIZE, 3};

	MTYPE full[r][c][3];
	MTYPE ds[ds_dim.r][ds_dim.c][3];
	MTYPE diff[ds_dim.r][ds_dim.c][3];
	MTYPE grad[ds_dim.r][ds_dim.c][3];

	me_rgb_to_MTYPE(frame_dim, frame, full);
	me_downsample(frame_dim, full, ds_dim, ds);

	{ // compute first derivatives
		memset(grad, 0, sizeof(MTYPE) * ds_dim.r * ds_dim.c * ds_dim.d);
		me_dc_dx(ds_dim, ds, grad);
		me_dc_dy(ds_dim, ds, grad);		
	}

	// compute frame difference
	me_sub(ds_dim, ds, LAST, diff);
	me_abs(ds_dim, diff, diff);


	// find the average center of motion
	short tol[3] = {64, 64, 64};
	point_t com = me_center_of_mass(ds_dim, diff, tol);
	point_t min = com, max = com;

	for (int r = 0; r < ds_dim.r; ++r)
	{
		for (int c = 0; c < ds_dim.c; ++c)
		{
			if (diff[r][c][1] > tol[1])
			{
				min.r = ME_MIN(min.r, r);
				max.r = ME_MAX(max.r, r);
				min.c = ME_MIN(min.c, c);
				max.c = ME_MAX(max.c, c);		
			}
		}
	}

	win_t movement = {min.r, min.c, max.c - min.c, max.r - min.r};

	// find the strongest feature in that window
	int hf = (F_SIZE - 1) >> 1;
	for (int r = min.r; r < max.r; ++r)
	for (int c = min.c; c < max.c; ++c)
	{
		for (int kr = -hf; kr <= hr; kr++)
		for (int kc = -hf; kc <= hc; kr++)
		{
			int ri = r + kr, ci = c + kc;
			if (ri < 0 || ri >= max.r) { continue; }
			if (ci < 0 || ci >= max.c) { continue; }

			// TODO compute score here
		}
	}

	// me_bias(ds_dim, ds, ds, (MTYPE[3]){ 128, 128, 128}, (win_t){0, 0, ds_dim.c, ds_dim.r}); 
	me_bias(ds_dim, diff, diff, (MTYPE[3]){ 0, 64, 0}, movement); 

	me_upsample(ds_dim, diff, frame_dim, full);
	me_MTYPE_to_rgb(frame_dim, full, frame);

	frames++;
	memcpy(LAST, ds, sizeof(LAST));
}


int main (int argc, const char* argv[])
{
	camera_loop(480, 640, argv[1], process);

	return 0;
}
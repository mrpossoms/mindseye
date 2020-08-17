#include "camera_loop.h"
#include "common.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define F_SIZE 7

match_t last_match;
vidi_rgb_t feature[F_SIZE][F_SIZE];

void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{
	static int frames;

	const point_t frame_dim = {r, c};
	const point_t ds_dim = {r >> 2, c >> 2};
	const point_t feat_dim = {F_SIZE, F_SIZE};

	vidi_rgb_t ds[ds_dim.r][ds_dim.c];

	me_downsample(frame_dim, frame, ds_dim, ds);

	{ // compute first derivatives
		vidi_rgb_t o[r][c];
		memcpy(o, ds, sizeof(vidi_rgb_t) * ds_dim.r * ds_dim.c);
		me_dc_dx(ds_dim, o, ds);
		me_dc_dy(ds_dim, o, ds);		
	}

	// { // compute second derivaties
	// 	vidi_rgb_t o[r][c];
	// 	memcpy(o, ds, sizeof(vidi_rgb_t) * ds_dim.r * ds_dim.c);
	// 	me_dc_dx(ds_dim, o, ds);
	// 	me_dc_dy(ds_dim, o, ds);		
	// }

	me_variance(ds_dim, (vidi_rgb_t){128, 128, 128}, ds, ds);


	if (frames < 10 || last_match.score > 1000)
	{
		me_patch(
			ds_dim, ds, 
			(win_t){ (ds_dim.r >> 1) - F_SIZE/2, (ds_dim.c >> 1) - F_SIZE/2, F_SIZE, F_SIZE },
			feature
			);
	}
	else if (last_match.score > 50)
	{
		me_patch(
			ds_dim, ds, 
			last_match.win,
			feature
			);	
	}

	match_t match = me_match_feature(ds_dim, ds, feat_dim, feature, (win_t){0, 0, ds_dim.r, ds_dim.c});
	match.score += pow(match.win.r - last_match.win.r, 2) + pow(match.win.c - last_match.win.c, 2);

	printf("score: %d (%d, %d)\n", match.score, match.win.r, match.win.c);
	last_match = match;


	me_bias(ds_dim, ds, ds, (vidi_rgb_t){ 0, 64, 0}, match.win); 

	me_upsample(ds_dim, ds, frame_dim, frame);

	frames++;
}


int main (int argc, const char* argv[])
{
	camera_loop(480, 640, argv[1], process);

	return 0;
}
#include "camera_loop.h"
#include "common.h"
#include <stdbool.h>
#include <string.h>

#define F_SIZE 9

match_t last_match;
vidi_rgb_t feature[F_SIZE][F_SIZE];

void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{
	static bool not_first;

	point_t frame_dim = {r, c};
	point_t ds_dim = {r >> 2, c >> 2};
	point_t feat_dim = {F_SIZE, F_SIZE};

	vidi_rgb_t ds[ds_dim.r][ds_dim.c];

	me_downsample(frame_dim, frame, ds_dim, ds);

	vidi_rgb_t o[r][c];
	memcpy(o, ds, sizeof(vidi_rgb_t) * ds_dim.r * ds_dim.c);
	me_dc_dx(ds_dim, o, ds);
	me_dc_dy(ds_dim, o, ds);

	if (!not_first)
	{
		me_patch(
			ds_dim, ds, 
			(win_t){ r - F_SIZE/2, c - F_SIZE/2, F_SIZE, F_SIZE },
			feature
			);
	}

	match_t match = me_match_feature(ds_dim, ds, feat_dim, feature, (win_t){0, 0, r, c});
	me_bias(ds_dim, ds, ds, (vidi_rgb_t){ 0, 32, 0}, match.win); 

	me_upsample(ds_dim, ds, frame_dim, frame);

	last_match = match;
	not_first = true;
}


int main (int argc, const char* argv[])
{
	camera_loop(480, 640, argv[1], process);

	return 0;
}
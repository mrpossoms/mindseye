#include "camera_loop.h"
#include "common.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define F_SIZE 7
#define F_COUNT 4

match_t last_match;
dim_t feat_dim = { F_SIZE, F_SIZE, 3 };
MTYPE feature [F_COUNT][F_SIZE][F_SIZE][3];
bool feature_valid = false;

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
	win_t feat_win = {};

	if (frames > 10)
	if (!feature_valid && movement.w > 0 && movement.h > 0)
	{
		// find the strongest feature in that window
		int hf = (F_SIZE - 1) >> 1;
		unsigned long best_score[F_COUNT] = {};
		point_t best_point[F_COUNT] = {};
		int best_idx = 0;

		int r_per_f = (max.r - min.r) / F_COUNT; 

		for (int fi = 0; fi < F_COUNT; fi++)
		for (int r = min.r + (fi * r_per_f); r < min.r + (fi + 1) * r_per_f; ++r)
		for (int c = min.c; c < max.c; ++c)
		{
			int kern_score = 0;
			for (int kr = -hf; kr <= hf; kr++)
			for (int kc = -hf; kc <= hf; kc++)
			{
				int ri = r + kr, ci = c + kc;
				if (ri < 0 || ri >= max.r) { continue; }
				if (ci < 0 || ci >= max.c) { continue; }

				for (int di = 0; di < ds_dim.d; di++)
				{
					kern_score += abs(grad[ri][ci][di]);
				}
			}

			if (kern_score > best_score[best_idx])
			{
				// int dist_sq = 0;
				// for (int fi = 0; fi < F_COUNT; fi++)
				// {
				// 	if (best_score[fi] <= 0) { continue; }

				// 	dist_sq += (pow((int)best_point[fi].r - r, 2) + pow((int)best_point[fi].c - c, 2));
				// }

				best_score[best_idx] = kern_score;
				best_point[best_idx] = (point_t){ r, c };
				best_idx = (best_idx + 1) % F_COUNT;
			}
		}

		printf("best score: ");
		for (int fi = 0; fi < F_COUNT; fi++)
		{
			feat_win = (win_t){best_point[fi].r-hf, best_point[fi].c-hf, F_SIZE, F_SIZE};
			printf("%ld ", best_score[fi]);
			me_patch(ds_dim, ds, feat_win, feature[fi]);
			feature_valid = true;
		}
		printf("\n");
	}
	else
	{
		for (int fi = 0; fi < F_COUNT; fi++)
		{
			match_t match = me_match_feature(ds_dim, ds, feat_dim, feature[fi], (win_t){});
			printf("score: %d\n", match.score);
			feat_win = match.win;

			if (match.score > 30) { feature_valid = false; }
			me_bias(ds_dim, diff, diff, (MTYPE[3]){ 128, 0, 0}, feat_win);
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
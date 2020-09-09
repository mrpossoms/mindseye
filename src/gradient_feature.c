#include "camera_loop.h"
#include "common.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

MTYPE LAST[640 >> 6][480 >> 6][3];

#define F_SIZE 7
#define F_COUNT ((640 >> 6) * (480 >> 6))
dim_t feat_dim = { F_SIZE, F_SIZE, 3 };
MTYPE feature [F_COUNT][F_SIZE][F_SIZE][3];
win_t feat_win [F_COUNT];
int feats_active;
match_t last_match;


win_t idx_to_win(dim_t full_dim, dim_t ds_dim, point_t idx)
{
	int full_r_per_ds_r = full_dim.r / ds_dim.r;
	int full_c_per_ds_c = full_dim.c / ds_dim.c;

	return (win_t) {
		idx.r * full_r_per_ds_r,
		idx.c * full_c_per_ds_c,
		full_c_per_ds_c,
		full_r_per_ds_r
	};
}

int extract_features(
	dim_t dd, MTYPE diff[dd.r][dd.c][dd.d],
	dim_t fd, MTYPE frame[fd.r][fd.c][fd.d],
	int min_feature_score
	)
{
	for (int diff_r = 0; diff_r < dd.r; diff_r++)
	for (int diff_c = 0; diff_c < dd.c; diff_c++)
	{
		bool skip = true;
		for (int d = 0; d < dd.d; d++)
		{
			if (diff[diff_r][diff_c][d] > 64) { skip = false; break; }
		}
		if (skip) { continue; }

		win_t patch_win = idx_to_win(fd, dd, (point_t){ diff_r, diff_c });
		point_t min = { patch_win.r, patch_win.c };
		point_t max = { patch_win.r + patch_win.h, patch_win.c + patch_win.w };
		// find the strongest feature in that window
		int hf = (F_SIZE - 1) >> 1;
		int best_score = -1;
		point_t best_point = {};

		for (int r = min.r; r < max.r; ++r)
		for (int c = min.c; c < max.c; ++c)
		{
			int kern_score = 0;
			for (int kr = -hf; kr <= hf; kr++)
			for (int kc = -hf; kc <= hf; kc++)
			{
				int ri = r + kr, ci = c + kc;
				if (ri < 0 || ri >= max.r) { continue; }
				if (ci < 0 || ci >= max.c) { continue; }

				for (int di = 0; di < fd.d; di++)
				{
					kern_score += abs(frame[ri][ci][di]);
				}
			}

			if (kern_score > min_feature_score && kern_score > best_score)
			{
				best_score = kern_score;
				best_point = (point_t){ r, c };
			}
		}

		printf("best score: ");
		if (best_score >= min_feature_score)
		{
			win_t win = (win_t){best_point.r-hf, best_point.c-hf, F_SIZE, F_SIZE};
			// printf("%ld ", best_score[fi]);
			me_patch(fd, frame, win, feature[feats_active]);
			feat_win[feats_active] = win;

			feats_active++;
		}

		printf("\n");
	}

	return feats_active;
}

void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{
	static int frames;

	const dim_t frame_dim = {r, c, 3};
	const dim_t ds_dim = {r >> 1, c >> 1, 3};
	const dim_t ds_diff_dim = {r >> 6, c >> 6, 3};
	const dim_t feat_dim = {F_SIZE, F_SIZE, 3};

	MTYPE full[r][c][3];
	MTYPE ds[ds_dim.r][ds_dim.c][3];
	MTYPE ds_red[ds_diff_dim.r][ds_diff_dim.c][3];
	MTYPE diff[ds_diff_dim.r][ds_diff_dim.c][3];
	MTYPE grad[ds_dim.r][ds_dim.c][3];

	me_rgb_to_MTYPE(frame_dim, frame, full);
	me_downsample(frame_dim, full, ds_dim, ds);
	me_downsample(frame_dim, full, ds_diff_dim, ds_red);


	{ // compute first derivatives
		memset(grad, 0, sizeof(MTYPE) * ds_dim.r * ds_dim.c * ds_dim.d);
		me_dc_dx(ds_dim, ds, grad);
		me_dc_dy(ds_dim, ds, grad);		
	}

	// compute frame difference
	me_sub(ds_diff_dim, ds_red, LAST, diff);
	me_abs(ds_diff_dim, diff, diff);


	// find the average center of motion
	short tol[3] = {64, 64, 64};
	point_t com = me_center_of_mass(ds_dim, diff, tol);
	point_t min = com, max = com;

	// for (int r = 0; r < ds_dim.r; ++r)
	// {
	// 	for (int c = 0; c < ds_dim.c; ++c)
	// 	{
	// 		if (diff[r][c][1] > tol[1])
	// 		{
	// 			min.r = ME_MIN(min.r, r);
	// 			max.r = ME_MAX(max.r, r);
	// 			min.c = ME_MIN(min.c, c);
	// 			max.c = ME_MAX(max.c, c);		
	// 		}
	// 	}
	// }

	// win_t movement = {min.r, min.c, max.c - min.c, max.r - min.r};
	// win_t feat_win = {};



	if (frames > 10)
	if (!feats_active)// && movement.w > 0 && movement.h > 0)
	{
		extract_features(ds_diff_dim, diff, ds_dim, grad, 30);
	}
	else
	{
		for (int fi = 0; fi < feats_active; fi++)
		{
			// match_t match = me_match_feature(ds_dim, ds, feat_dim, feature[fi], (win_t){});
			// printf("score: %d\n", match.score);
			// feat_win[] = match.win;

			// if (match.score > 30) { feature_valid = false; }
			me_bias(frame_dim, full, full, (MTYPE[3]){ 128, 0, 0}, feat_win[fi]);
		}
	}

	

	// me_bias(ds_dim, ds, ds, (MTYPE[3]){ 128, 128, 128}, (win_t){0, 0, ds_dim.c, ds_dim.r}); 
	// me_bias(ds_dim, diff, diff, (MTYPE[3]){ 0, 64, 0}, movement); 

	me_upsample(ds_dim, ds, frame_dim, full);
	me_MTYPE_to_rgb(frame_dim, full, frame);

	frames++;
	memcpy(LAST, ds_red, sizeof(LAST));
}


int main (int argc, const char* argv[])
{
	camera_loop(480, 640, argv[1], process);

	return 0;
}

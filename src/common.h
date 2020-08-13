#ifndef _ME_COMMON_H_
#define _ME_COMMON_H_

#include "vidi.h"
#include <math.h>
#include <stdlib.h>

typedef struct {
	size_t r, c;
} point_t;

typedef struct {
	int r, c, w, h;
} win_t;

typedef struct {
	win_t win;
	int score;
} match_t;

#define ME_MIN(x, a) ((x) > (a) ? (a) : (x))
#define ME_MAX(x, a) ((x) < (a) ? (a) : (x))

static void me_dc_dx(point_t size, vidi_rgb_t in[size.r][size.c], vidi_rgb_t out[size.r][size.c])
{
	for (int ri = 0; ri < size.r; ++ri)
	{
		int ri_n = ri + 1;
		if (ri_n > size.r) { ri_n = size.r - 2; }
		for (int ci = 0; ci < size.c; ++ci)
		{
			int ci_n = ci + 1;
			out[ri][ci].r = out[ri][ci].g = out[ri][ci].b = 128;

			if (ci_n > size.c) { ci_n = size.c - 2; }

			for (int i = 0; i < 3; i++)
			{ // dc / dx 
				int dc = in[ri][ci].v[i] - in[ri][ci_n].v[i];
				out[ri][ci].v[i] += dc / (ci_n - ci);
			}
		}
	}
}


static void me_dc_dy(point_t size, vidi_rgb_t in[size.r][size.c], vidi_rgb_t out[size.r][size.c])
{
	for (int ri = 0; ri < size.r; ++ri)
	{
		int ri_n = ri + 1;
		if (ri_n > size.r) { ri_n = size.r - 2; }
		for (int ci = 0; ci < size.c; ++ci)
		{
			int ci_n = ci + 1;
			out[ri][ci].r = out[ri][ci].g = out[ri][ci].b = 128;

			if (ci_n > size.c) { ci_n = size.c - 2; }

			for (int i = 0; i < 3; i++)
			{ // dc / dy 
				int dc = in[ri][ci].v[i] - in[ri_n][ci].v[i];
				out[ri][ci].v[i] += dc / (ri_n - ri);
			}
		}
	}
}


static void me_variance(
	point_t dims, 
	vidi_rgb_t mu,
	vidi_rgb_t in[dims.r][dims.c],
	vidi_rgb_t out[dims.r][dims.c])
{
	for (int r = 0; r < dims.r; r++)
	for (int c = 0; c < dims.c; c++)
	{
		int delta[3] = {};
		for (int i = 3; i--;)
		{
			delta[i] = in[r][c].v[i] - mu.v[i];
			out[r][c].v[i] = ME_MIN(sqrt(delta[i] * delta[i]), 255);
		}
	}
}


static void me_downsample(
	point_t sd, vidi_rgb_t src[sd.r][sd.c],
	point_t dd, vidi_rgb_t dst[dd.r][dd.c])
{
	int src_r_per_dst_r = sd.r / dd.r;
	int src_c_per_dst_c = sd.c / dd.c;

	for (int r = 0; r < dd.r; r++)
	for (int c = 0; c < dd.c; c++)
	{
		int avg[3] = {};
		size_t sr = r * src_r_per_dst_r;
		size_t sc = c * src_c_per_dst_c;

		for (int ri = sr; ri < sr + src_r_per_dst_r; ri++)
		for (int ci = sc; ci < sc + src_c_per_dst_c; ci++)
		{ // for each patch of the src frame
			for (int i = 3; i--;)
			avg[i] += src[ri][ci].v[i];
		}

		for (int i = 3; i--;)
		dst[r][c].v[i] = avg[i] / (src_r_per_dst_r * src_c_per_dst_c);
	}
}


static void me_upsample(
	point_t sd, vidi_rgb_t src[sd.r][sd.c],
	point_t dd, vidi_rgb_t dst[dd.r][dd.c])
{
	int dst_r_per_src_r = dd.r / sd.r;
	int dst_c_per_src_c = dd.c / sd.c;

	for (int r = 0; r < dd.r; r++)
	for (int c = 0; c < dd.c; c++)
	{
		dst[r][c] = src[r / dst_r_per_src_r][c / dst_c_per_src_c];
	}
}


static void me_patch(point_t size, vidi_rgb_t src[size.r][size.c], win_t out_win, vidi_rgb_t out[out_win.h][out_win.w])
{
	for (int ri = 0; ri < out_win.h; ++ri)
	for (int ci = 0; ci < out_win.w; ++ci)
	{
		out[ri][ci] = src[out_win.r+ri][out_win.c+ci];
	}
}


static void me_blit(win_t sw, vidi_rgb_t src[sw.h][sw.w], point_t dd, vidi_rgb_t dst[dd.r][dd.c])
{
	for (int r = 0; r < sw.h; r++)
	for (int c = 0; c < sw.w; c++)
	{
		dst[sw.r + r][sw.c + c] = src[r][c];
	}
}


static void me_bias(point_t dim, const vidi_rgb_t from[dim.r][dim.c], vidi_rgb_t to[dim.r][dim.c], vidi_rgb_t bias, win_t win)
{
	for(int ri = win.h; ri--;)
	for(int ci = win.w; ci--;)
	{
		to[win.r+ri][win.c+ci].r = from[win.r+ri][win.c+ci].r + bias.r;
		to[win.r+ri][win.c+ci].g = from[win.r+ri][win.c+ci].g + bias.g;
		to[win.r+ri][win.c+ci].b = from[win.r+ri][win.c+ci].b + bias.b;
	}
}


match_t me_match_feature(
	const point_t frame_size,
	vidi_rgb_t frame[frame_size.r][frame_size.c],
	const point_t feat_size,
	vidi_rgb_t feature[feat_size.r][feat_size.c],
	win_t search_win)
{
	match_t match = { .score = 256 * feat_size.r * feat_size.c };

	int feat_hr = (feat_size.r - 1) / 2; 
	int feat_hc = (feat_size.c - 1) / 2; 

	int start_r = feat_hr + search_win.r;
	int end_r = start_r + search_win.h - feat_hr;
	int start_c = feat_hc + search_win.c;
	int end_c = start_c + search_win.w - feat_hc;

	for (int r = start_r; r < end_r; r++)
	for (int c = start_c; c < end_c; c++)
	{
		int score = 0;
		for (int i = -feat_hr; i <= feat_hr; i++)
		for (int j = -feat_hc; j <= feat_hc; j++)
		{
			for (int ci = 3; ci--;)
			{
				score += abs(feature[i + feat_hr][j + feat_hc].v[ci] - frame[r + i][c + j].v[ci]);				
			}
		}
		score /= (feat_size.r * feat_size.c * 3);

		if (score < match.score)
		{
			match.win.r = r - feat_hr;
			match.win.c = c - feat_hc;
			match.win.w = feat_size.c;
			match.win.h = feat_size.r;
			match.score = score;
		}
	}

	return match;
}



#endif
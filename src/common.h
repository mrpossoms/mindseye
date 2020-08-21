#ifndef _ME_COMMON_H_
#define _ME_COMMON_H_

#include "vidi.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	size_t r, c, d;
} dim_t;

typedef struct {
	int r, c, w, h;
} win_t;

typedef struct {
	win_t win;
	int score;
} match_t;

#define ME_MIN(x, a) ((x) > (a) ? (a) : (x))
#define ME_MAX(x, a) ((x) < (a) ? (a) : (x))

static void me_rgb_to_short(dim_t size, vidi_rgb_t in[size.r][size.c], short out[size.r][size.c][3])
{
	for (int ri = 0; ri < size.r; ++ri)
	for (int ci = 0; ci < size.c; ++ci)
	{
		for (int di = 0; di < size.d; ++di)
		out[ri][ci][di] = in[ri][ci].v[di];
	}

}


static void me_short_to_rgb(dim_t size, short in[size.r][size.c][size.d], vidi_rgb_t out[size.r][size.c])
{
	for (int ri = 0; ri < size.r; ++ri)
	for (int ci = 0; ci < size.c; ++ci)
	{
		for (int di = 0; di < size.d; ++di)
		out[ri][ci].v[di] = ME_MIN(255, ME_MAX(0, in[ri][ci][di]));
	}
}


static void me_dc_dx(dim_t size, short in[size.r][size.c][size.d], short out[size.r][size.c][size.d])
{
	for (int ri = 0; ri < size.r; ++ri)
	{
		int ri_n = ri + 1;
		if (ri_n >= size.r) { ri_n = size.r - 2; }
		for (int ci = 0; ci < size.c; ++ci)
		{
			int ci_n = ci + 1;

			if (ci_n >= size.c) { ci_n = size.c - 2; }

			for (int i = 0; i < size.d; i++)
			{ // dc / dx 
				int dc = in[ri][ci][i] - in[ri][ci_n][i];
				out[ri][ci][i] += dc / (ci_n - ci);
			}
		}
	}
}


static void me_dc_dy(dim_t size, short in[size.r][size.c][size.d], short out[size.r][size.c][size.d])
{
	for (int ri = 0; ri < size.r; ++ri)
	{
		int ri_n = ri + 1;
		if (ri_n >= size.r) { ri_n = size.r - 2;break; }
		for (int ci = 0; ci < size.c; ++ci)
		{
			int ci_n = ci + 1;

			if (ci_n >= size.c) { ci_n = size.c - 2; }

			for (int i = 0; i < size.d; i++)
			{ // dc / dy 
				int dc = in[ri][ci][i] - in[ri_n][ci][i];
				out[ri][ci][i] += dc / (ri_n - ri);
			}
		}
	}
}


static void me_sub(dim_t size, short left[size.r][size.c][size.d], short right[size.r][size.c][size.d], short out[size.r][size.c][size.d])
{
	for (int r = 0; r < size.r; ++r)
	{
		for (int c = 0; c < size.c; ++c)
		{
			for (int d = 0; d < size.d; ++d)
			{ 
				out[r][c][d] = left[r][c][d] - right[r][c][d];
			}
		}
	}
}


static void me_add(dim_t size, short left[size.r][size.c][size.d], short right[size.r][size.c][size.d], short out[size.r][size.c][size.d])
{
	for (int r = 0; r < size.r; ++r)
	{
		for (int c = 0; c < size.c; ++c)
		{
			for (int d = 0; d < size.d; ++d)
			{ 
				out[r][c][d] = left[r][c][d] + right[r][c][d];
			}
		}
	}
}



static void me_mul(dim_t size, short left[size.r][size.c][size.d], short right[size.r][size.c][size.d], short out[size.r][size.c][size.d])
{
	for (int r = 0; r < size.r; ++r)
	{
		for (int c = 0; c < size.c; ++c)
		{
			for (int d = 0; d < size.d; ++d)
			{ 
				out[r][c][d] = left[r][c][d] * right[r][c][d];
			}
		}
	}
}


static void me_abs(dim_t size, short in[size.r][size.c][size.d], short out[size.r][size.c][size.d])
{
	for (int r = 0; r < size.r; ++r)
	{
		for (int c = 0; c < size.c; ++c)
		{
			for (int d = 0; d < size.d; ++d)
			{ 
				out[r][c][d] = abs(in[r][c][d]);
			}
		}
	}
}


static void me_mask(dim_t size, short in[size.r][size.c][size.d], short out[size.r][size.c][size.d], short mask[size.r][size.c][size.d], short tol)
{
	for (int r = 0; r < size.r; ++r)
	{
		for (int c = 0; c < size.c; ++c)
		{
			for (int d = 0; d < size.d; ++d)
			{ 
				out[r][c][d] = mask[r][c][d] > tol ? in[r][c][d] : 0;
			}
		}
	}
}


static void me_dc_dx_f(dim_t size, float in[size.r][size.c][size.d], float out[size.r][size.c][size.d])
{
	for (int ri = 0; ri < size.r; ++ri)
	{
		int ri_n = ri + 1;
		if (ri_n > size.r) { ri_n = size.r - 2; }
		for (int ci = 0; ci < size.c; ++ci)
		{
			int ci_n = ci + 1;

			if (ci_n > size.c) { ci_n = size.c - 2; }

			for (int i = 0; i < size.d; i++)
			{ // dc / dx 
				float dc = in[ri][ci][i] - in[ri][ci_n][i];
				out[ri][ci][i] += dc / (float)(ci_n - ci);
			}
		}
	}
}


static void me_dc_dy_f(dim_t size, float in[size.r][size.c][size.d], float out[size.r][size.c][size.d])
{
	for (int ri = 0; ri < size.r; ++ri)
	{
		int ri_n = ri + 1;
		if (ri_n > size.r) { ri_n = size.r - 2; }
		for (int ci = 0; ci < size.c; ++ci)
		{
			int ci_n = ci + 1;

			if (ci_n > size.c) { ci_n = size.c - 2; }

			for (int i = 0; i < size.d; i++)
			{ // dc / dy 
				float dc = in[ri][ci][i] - in[ri_n][ci][i];
				out[ri][ci][i] += dc / (float)(ri_n - ri);
			}
		}
	}
}


static void me_variance(
	dim_t dims, 
	short mu[dims.d],
	short in[dims.r][dims.c][dims.d],
	short out[dims.r][dims.c][dims.d])
{
	for (int r = 0; r < dims.r; r++)
	for (int c = 0; c < dims.c; c++)
	{
		int delta[dims.d];
		for (int i = dims.d; i--;)
		{
			delta[i] = in[r][c][i] - mu[i];
			out[r][c][i] = ME_MIN(sqrt(delta[i] * delta[i]), 255);
		}
	}
}


static void me_downsample(
	dim_t sd, short src[sd.r][sd.c][sd.d],
	dim_t dd, short dst[dd.r][dd.c][dd.d])
{
	int src_r_per_dst_r = sd.r / dd.r;
	int src_c_per_dst_c = sd.c / dd.c;

	for (int r = 0; r < dd.r; r++)
	for (int c = 0; c < dd.c; c++)
	{
		int avg[dd.d];
		size_t sr = r * src_r_per_dst_r;
		size_t sc = c * src_c_per_dst_c;

		// initalize 'avg'
		memset((void*)avg, 0, sizeof(avg));

		for (int ri = sr; ri < sr + src_r_per_dst_r; ri++)
		for (int ci = sc; ci < sc + src_c_per_dst_c; ci++)
		{ // for each patch of the src frame
			for (int i = dd.d; i--;)
			avg[i] += src[ri][ci][i];
		}

		for (int i = dd.d; i--;)
		dst[r][c][i] = avg[i] / (src_r_per_dst_r * src_c_per_dst_c);
	}
}


static void me_upsample(
	dim_t sd, short src[sd.r][sd.c][sd.d],
	dim_t dd, short dst[dd.r][dd.c][dd.d])
{
	int dst_r_per_src_r = dd.r / sd.r;
	int dst_c_per_src_c = dd.c / sd.c;

	for (int r = 0; r < dd.r; r++)
	for (int c = 0; c < dd.c; c++)
	{
		for (int d = 0; d < dd.d; d++)
		dst[r][c][d] = src[r / dst_r_per_src_r][c / dst_c_per_src_c][d];
	}
}


static void me_patch(dim_t size, short src[size.r][size.c][size.d], win_t out_win, short out[out_win.h][out_win.w][size.d])
{
	for (int ri = 0; ri < out_win.h; ++ri)
	for (int ci = 0; ci < out_win.w; ++ci)
	{
		for (int d = 0; d < size.d; d++)
		out[ri][ci][d] = src[out_win.r+ri][out_win.c+ci][d];
	}
}


static void me_blit(dim_t ds, short src[ds.r][ds.c][ds.d], dim_t dd, short dst[dd.r][dd.c][dd.d], win_t sw)
{
	for (int r = 0; r < sw.h; r++)
	for (int c = 0; c < sw.w; c++)
	for (int d = 0; d < ds.d; d++)
	{
		dst[sw.r + r][sw.c + c][d] = src[r][c][d];
	}
}


static void me_bias(dim_t dim, const short from[dim.r][dim.c][dim.d], short to[dim.r][dim.c][dim.d], short bias[dim.d], win_t win)
{
	for(int ri = win.h; ri--;)
	for(int ci = win.w; ci--;)
	{
		for (int di = 0; di < dim.d; di++)
		to[win.r+ri][win.c+ci][di] = from[win.r+ri][win.c+ci][di] + bias[di];
	}
}


static void me_convolve(
	dim_t sd,
	const short src[sd.r][sd.c][sd.d],
	short dst[sd.r][sd.c][sd.d],
	dim_t kd, const float kern[kd.r][kd.c][kd.d])
{
	int h_kr = (kd.r) >> 1;
	int h_kc = (kd.c) >> 1;

	for (int r = 0; r < sd.r; r++)
	for (int c = 0; c < sd.c; c++)
	{
		for (int kr = -h_kr; kr <= h_kr; kr++)
		for (int kc = -h_kc; kc <= h_kc; kc++)
		{
			int samples = 0;
			int ri = r + kr, ci = c + kc;

			if (ri < 0 || ri >= sd.r) { continue; }
			if (ci < 0 || ci >= sd.c) { continue; }

			for (int d = 0; d < sd.d; d++)
			dst[r][c][d] += src[ri][ci][d] * kern[kr + h_kr][kc + h_kc][d % kd.d]; 
		}
	}
}


match_t me_match_feature(
	const dim_t fd,
	short frame[fd.r][fd.c][fd.d],
	const dim_t feat_size,
	short feature[feat_size.r][feat_size.c][fd.d],
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
			for (int ci = fd.d; ci--;)
			{
				score += abs(feature[i + feat_hr][j + feat_hc][ci] - frame[r + i][c + j][ci]);				
			}
		}
		score /= (feat_size.r * feat_size.c * fd.d);

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
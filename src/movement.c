#include "camera_loop.h"
#include "common.h"

MTYPE LAST[640 >> 4][480 >> 4][3];
// MTYPE LAST_1[640 >> 1][480 >> 1][3];
// MTYPE LAST_2[640 >> 2][480 >> 2][3];
// MTYPE LAST_3[640 >> 3][480 >> 3][3];


void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{
	dim_t ds_dim = {r>>4, c>>4, 3};
	MTYPE o[ds_dim.r][ds_dim.c][3];
	MTYPE d[ds_dim.r][ds_dim.c][3];
	MTYPE m[ds_dim.r][ds_dim.c][3];

	dim_t f_dim = {r, c, 3};


	MTYPE full[r][c][3];
	me_rgb_to_MTYPE(f_dim, frame, full);
	me_downsample(f_dim, full, ds_dim, o);	

	me_sub(ds_dim, LAST, o, d);

	int tol = pow(5, 2);
	ME_EACH(ds_dim, r, c, d)
	{
		int mag = 0;
		for (int i = ds_dim.d; i--;)
		{
			mag += d[r][c][i] * d[r][c][i];
		}

		if (mag >= tol) { d[r][c][1] = 255; }
	}

	for (int r = 0; r < ds_dim.r; r++)
	{
		int intersections = 0;
		for (int c = 0; c < ds_dim.c; c++)
		{
			if (d[r][c][1] == 255)
			{
				intersections++;
			}
		}


		int i = 0;
		if (intersections % 2 == 0)
		for (int c = 0; c < ds_dim.c; c++)
		{
			if (d[r][c][1] == 255) { i++; }
			if (i % 2)
			{
				d[r][c][1] = 255;
			}
		}
	}


	me_upsample(ds_dim, d, f_dim, full);
	me_MTYPE_to_rgb(f_dim, full, frame);

	memcpy(LAST, o, sizeof(LAST));
}


int main (int argc, const char* argv[])
{
	camera_loop(480, 640, argv[1], process);

	return 0;
}
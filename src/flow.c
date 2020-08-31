#include "camera_loop.h"

// #define MTYPE uint8_t

#include "common.h"

MTYPE LAST_FRAME[480][640][3];
dim_t fd = {480 >> 3, 640 >> 3, 3};
flow_t flow[480 >> 3][640 >> 3];

void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{
	static int frame_num;
	MTYPE o[r][c][3];
	MTYPE f[r][c][3];

	dim_t frame_dim = {r, c, 3};

	me_rgb_to_MTYPE(frame_dim, frame, o);
	// memset(f, 0, sizeof(MTYPE) * 3 * r * c);

	dim_t k_s = {3, 3, 1};
	MTYPE k_gauss[3][3][1] = {
		{  1,  2,  1 },
		{  2,  4,  2 },
		{  1,  2,  1 },
	};
	float gauss_norm[3] = { 1/16.f, 1/16.f, 1/16.f, };
	// me_convolve(frame_dim, o, f, k_s, k_gauss);
	// me_scl_f(frame_dim, f, f, gauss_norm);

	// me_convolve(frame_dim, f, o, k_s, k_gauss);
	// me_scl_f(frame_dim, o, o, gauss_norm);

	if (frame_num++)
	{
		int ks = 3;
		me_flow(frame_dim, LAST_FRAME, o, fd, flow, (flow_opts_t) {
			.kernel_size = ks,
			.search_win = (win_t){ 0, 0, ks*3, ks*3 },
		});

		MTYPE flow_map[fd.r][fd.c][3];
		for (int ri = 0; ri < fd.r; ri++)
		for (int ci = 0; ci < fd.c; ci++)
		{
			if (flow[ri][ci].score < 3)
			{
				flow_map[ri][ci][0] = 0;
				flow_map[ri][ci][1] = 0;
				flow_map[ri][ci][2] = 0;					
			}
			else
			{
				flow_map[ri][ci][0] = 0 + 10 * flow[ri][ci].dc;
				flow_map[ri][ci][1] = 0 + 10 * flow[ri][ci].dr;
				flow_map[ri][ci][2] = flow[ri][ci].m.score;				
			}
		}

		me_upsample(fd, flow_map, frame_dim, f);
	}
	memcpy(LAST_FRAME, o, sizeof(MTYPE) * 3 * r * c);

	// me_add(frame_dim, o, f, o);

	// for (int ri = 0; ri < fd.r; ri++)
	// for (int ci = 0; ci < fd.c; ci++)
	// {
	// 	me_bias(frame_dim, o, o, (MTYPE[3]){ 0, 64, 0}, flow[ri][ci].m.win); 
	// }

	me_MTYPE_to_rgb(frame_dim, f, frame);
}


int main (int argc, const char* argv[])
{
	camera_loop(480, 640, argv[1], process);

	return 0;
}
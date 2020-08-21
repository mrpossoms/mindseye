#include "camera_loop.h"
#include "common.h"

short LAST[640>>1][480>>1][3];

void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{
	short o[r>>1][c>>1][3];
	short f[r>>1][c>>1][3];
	short m[r>>1][c>>1][3];

	dim_t f_dim = {r, c, 3};
	dim_t hf_dim = {r>>1, c>>1, 3};

	short full[r][c][3];
	me_rgb_to_short(f_dim, frame, full);
	me_downsample(f_dim, full, hf_dim, o);	

	short g[r>>1][c>>1][3];
	memset(g, 0, (sizeof(short) * 3) * hf_dim.r * hf_dim.c);
	me_dc_dx(hf_dim, o, g);
	me_dc_dy(hf_dim, o, g);

	me_sub(hf_dim, LAST, g, o);
	
	me_abs(hf_dim, o, m);
	me_mask(hf_dim, o, f, m, 32);

	// me_bias(hf_dim, f, f, (short[3]){ 128, 128, 128}, (win_t){ 0, 0, c, r}); 


	me_upsample(hf_dim, f, f_dim, full);
	me_short_to_rgb(f_dim, full, frame);

	memcpy(LAST, g, sizeof(LAST));
}


int main (int argc, const char* argv[])
{
	camera_loop(480, 640, argv[1], process);

	return 0;
}
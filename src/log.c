#include "camera_loop.h"
#include "common.h"

void gauss_kernel(dim_t kd, float kernel[kd.r][kd.c][kd.d], float t)
{
	int rh = (kd.r - 1) >> 1;
	int ch = (kd.c - 1) >> 1;

	for (int r = -rh; r <= rh; r++)
	for (int c = -ch; c <= ch; c++)
	{
		float x = c / (float)ch, y = r / (float)rh;
		float exp = -(x*x + y*y) / (2*t);
		kernel[r + rh][c + ch][0] = pow(M_E, exp) / (2 * M_PI * t);// / (float)(kd.r * kd.c);
	}
}

float K[5][5][1];
dim_t K_DIM = {5, 5, 1};

void process(size_t r, size_t c, vidi_rgb_t frame[r][c])
{

	short o[r][c][3];
	short f[r][c][3];

	dim_t frame_dim = {r, c, 3};

	me_rgb_to_short(frame_dim, frame, o);

	{ // compute first derivatives
		memset(f, 0, (sizeof(short) * 3) * frame_dim.r * frame_dim.c);
		me_convolve(frame_dim, o, f, K_DIM, K);
		
		// memcpy(f, o, (sizeof(short) * 3) * frame_dim.r * frame_dim.c);
		// memset(f, 0, (sizeof(short) * 3) * frame_dim.r * frame_dim.c);

		me_dc_dx(frame_dim, o, f);
		me_dc_dy(frame_dim, o, f);

		memcpy(o, f, (sizeof(short) * 3) * frame_dim.r * frame_dim.c);
		memset(f, 0, (sizeof(short) * 3) * frame_dim.r * frame_dim.c);
		
		me_dc_dx(frame_dim, o, f);
		me_dc_dy(frame_dim, o, f);
	}

	// me_bias(frame_dim, f, f, (short[3]){ 128, 128, 128}, (win_t){ 0, 0, c, r}); 

	me_short_to_rgb(frame_dim, f, frame);
}


int main (int argc, const char* argv[])
{
	float r = K_DIM.r / 2.f;
	float t = pow(r, 2)/2.f;
	gauss_kernel(K_DIM, K, t);

	for (int r = 0; r < K_DIM.r; r++)
	{
		for (int c = 0; c < K_DIM.c; c++)
		{
			printf("%d ", (int)K[r][c][0]);
		}
		printf("\n");		
	}printf("\n");

	// r = sqrt(d*t)
	// r^2 = d*t
	// (r^2)/2 = t

	{
		float kd[K_DIM.r][K_DIM.c][K_DIM.d];

		// memset(kd, 0, sizeof(float) * K_DIM.r * K_DIM.c);
		// me_dc_dx_f(K_DIM, K, kd);
		// me_dc_dy_f(K_DIM, K, kd);
		// memcpy(K, kd, sizeof(float) * K_DIM.r * K_DIM.c);

	for (int r = 0; r < K_DIM.r; r++)
	{
		for (int c = 0; c < K_DIM.c; c++)
		{
			printf("%d ", (int)K[r][c][0]);
		}
		printf("\n");		
	}printf("\n");

		// memset(kd, 0, sizeof(float) * K_DIM.r * K_DIM.c);
		// me_dc_dx_f(K_DIM, K, kd);
		// me_dc_dy_f(K_DIM, K, kd);
		// memcpy(K, kd, sizeof(float) * K_DIM.r * K_DIM.c);

	for (int r = 0; r < K_DIM.r; r++)
	{
		for (int c = 0; c < K_DIM.c; c++)
		{
			printf("%d ", (int)K[r][c][0]);
		}
		printf("\n");		
	}printf("\n");

	}

	camera_loop(480, 640, argv[1], process);

	return 0;
}
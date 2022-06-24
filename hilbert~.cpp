#include "m_pd.h"
#include "hiir/PolyphaseIir2Designer.h"

typedef struct biquad {
	double x_mem[2];
	double y_mem[2];
} biquad;

static void biquad_init(biquad *f) {
	f->x_mem[0] = 0.;
	f->x_mem[1] = 0.;
	f->y_mem[0] = 0.;
	f->y_mem[1] = 0.;
}

/* y[n] = a_0*x[n] + a_1*x[n-1] + a_2*x[n-2] - b_1*y[n-1] - b_2*y[n-2]
 * If a_1 = b_1 = 0, a_2 = 1 and b_2 = -a_0:
 *   y[n] = a_0*x[n] + x[n-2] + a_0*y[n-2]
 *        = a_0*(x[n] + y[n-2]) + x[n-2]
 */
static double allpass_tick(biquad *f, double a0, double x) {
	float y = a0*(x + f->y_mem[1]) - f->x_mem[1];
	f->x_mem[1] = f->x_mem[0]; f->x_mem[0] = x;
	f->y_mem[1] = f->y_mem[0]; f->y_mem[0] = y;
	return y;
}

static void calc_coeff(double *cR, double *cI, double r_sr) {
	double coeffs[8];
	hiir::PolyphaseIir2Designer::compute_coefs_spec_order_tbw(coeffs, 8, 40.*r_sr);
	cR[0] = coeffs[0];
	cR[1] = coeffs[2];
	cR[2] = coeffs[4];
	cR[3] = coeffs[6];
	cI[0] = coeffs[1];
	cI[1] = coeffs[3];
	cI[2] = coeffs[5];
	cI[3] = coeffs[7];
}

static t_class *hilbert_class;
typedef struct hilbert {
	t_object x_obj;
	biquad blockR[4];
	biquad blockI[4];
	float memI;
	float f;
	double coeffR[4];
	double coeffI[4];
	t_outlet *x_outR;
	t_outlet *x_outI;
} t_hilbert;

static void *hilbert_new() {
	t_hilbert *x = (t_hilbert *)pd_new(hilbert_class);
	biquad_init(&x->blockR[0]);
	biquad_init(&x->blockI[0]);
	biquad_init(&x->blockR[1]);
	biquad_init(&x->blockI[1]);
	biquad_init(&x->blockR[2]);
	biquad_init(&x->blockI[2]);
	biquad_init(&x->blockR[3]);
	biquad_init(&x->blockI[3]);
	calc_coeff(x->coeffR, x->coeffI, 1. / 44100.);
	x->memI = 0.f;
	x->x_outR = outlet_new(&x->x_obj, &s_signal);
	x->x_outI = outlet_new(&x->x_obj, &s_signal);
	return (void*)x;
}

static void hilbert_free(t_hilbert *x) {
	outlet_free(x->x_outR);
	outlet_free(x->x_outI);
}

static t_int *hilbert_perform(t_int *w) {
	t_hilbert *x = (t_hilbert *)(w[1]);
	t_sample *in = (t_sample *)(w[2]);
	t_sample *outR = (t_sample *)(w[3]);
	t_sample *outI = (t_sample *)(w[4]);
	int n = (int)(w[5]);

	for(int i = 0; i < n; i++) {
		allpass_tick(&x->blockR[0], x->coeffR[0], (double)in[i]);
		allpass_tick(&x->blockR[1], x->coeffR[1], x->blockR[0].y_mem[0]);
		allpass_tick(&x->blockR[2], x->coeffR[2], x->blockR[1].y_mem[0]);
		allpass_tick(&x->blockR[3], x->coeffR[3], x->blockR[2].y_mem[0]);
		
		allpass_tick(&x->blockI[0], x->coeffI[0], (double)x->memI);
		allpass_tick(&x->blockI[1], x->coeffI[1], x->blockI[0].y_mem[0]);
		allpass_tick(&x->blockI[2], x->coeffI[2], x->blockI[1].y_mem[0]);
		allpass_tick(&x->blockI[3], x->coeffI[3], x->blockI[2].y_mem[0]);

		x->memI = in[i];
		outR[i] = (t_sample)x->blockR[3].y_mem[0];
		outI[i] = (t_sample)x->blockI[3].y_mem[0];
	}

	return w + 6;
}

static void hilbert_dsp(t_hilbert *x, t_signal **sp) {
	calc_coeff(x->coeffR, x->coeffI, 1. / (double)sp[0]->s_sr);
	dsp_add(hilbert_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

#if defined(_WIN32)
# define PDEX_API extern "C" __declspec(dllexport)
#elif defined(__GNUC__)
# define PDEX_API extern "C" [[gnu::visibility("default")]]
#else
# define PDEX_API extern "C"
#endif

PDEX_API
void hilbert_tilde_setup(void) {
	hilbert_class = class_new(gensym("hilbert~"),
			(t_newmethod)hilbert_new,
			(t_method)hilbert_free,
			sizeof(t_hilbert),
			CLASS_DEFAULT, 0);
	class_addmethod(hilbert_class, (t_method)hilbert_dsp, gensym("dsp"), 0);
	CLASS_MAINSIGNALIN(hilbert_class, t_hilbert, f);
}

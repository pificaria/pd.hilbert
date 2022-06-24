#include "m_pd.h"

static t_class *prod_class;
typedef struct prod {
	t_object x_obj;
	float f;
	t_inlet *x_in[3];
	t_outlet *x_out[2];
} t_prod;

static void *prod_new(void) {
	t_prod *x = (t_prod *)pd_new(prod_class);
	x->x_in[0] = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	x->x_in[1] = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	x->x_in[2] = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	x->x_out[0] = outlet_new(&x->x_obj, &s_signal);
	x->x_out[1] = outlet_new(&x->x_obj, &s_signal);
	return (void*)x;
}

static void prod_free(t_prod *x) {
	inlet_free(x->x_in[0]);
	inlet_free(x->x_in[1]);
	inlet_free(x->x_in[2]);
	outlet_free(x->x_out[0]);
	outlet_free(x->x_out[1]);
	return ;
}

static t_int prod_perform(t_int *w) {
	t_prod *x = (t_prod *)(w[1]);
	t_sample *in[4] = {
		(t_sample *)(w[2]),
		(t_sample *)(w[3]),
		(t_sample *)(w[4]),
		(t_sample *)(w[5])
	};
	t_sample *out[2] = {
		(t_sample *)(w[6]),
		(t_sample *)(w[7])
	};
	int n = (int)(w[8]);

	/* x = a + bi, y = c + di
	 * x*y = ac + adi + bci - bd = (ac - bd) + (ad + bc)i
	 */
	for(int i = 0; i < n; i++) {
		out[0][i] = in[0][i]*in[2][i] - in[1][i]*in[3][i];
		out[1][i] = in[0][i]*in[3][i] + in[1][i]*in[2][i];
	}

	return w + 9;
}

static void prod_dsp(t_prod *x, t_signal **sp) {
	dsp_add(prod_perform, 8, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec,
			sp[4]->s_vec, sp[5]->s_vec, sp[0]->s_n);
}

void setup_complex0x2emul_tilde(void) {
	prod_class = class_new(gensym("complex.mul~"),
			(t_newmethod)prod_new,
			(t_method)prod_free,
			sizeof(t_prod),
			CLASS_DEFAULT, 0);
	class_addmethod(prod_class, (t_method)prod_dsp, gensym("dsp"), 0);
	CLASS_MAINSIGNALIN(prod_class, t_prod, f);
}

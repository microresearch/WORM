/* nrutil.h								*/

/* From W. H.Press, S. A. Teukolsky, W. T. Vettering and B. P. Flannery */
/* (1992) Numerical Recipes in C: The Art of Scientific Computing	*/
/* (2nd edition). Cambridge University Press.				*/

/* Non-copyright 							*/

/* pp. 941-2: This file only contains selected portions, however.	*/

static float sqrarg;
#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)

static float maxarg1,maxarg2;
#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ? (maxarg1) : (maxarg2))

void nrerror(char error_text[]);
float *vector(long nl, long nh);
void free_vector(float *v, long nl, long nh);


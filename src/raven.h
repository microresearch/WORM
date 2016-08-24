void donoise(float *out, int numSamples);


void RavenTube_init(void);
void RavenTube_next(float *inn, float *outt, int inNumSamples);

void single_tube_init(int len);
void single_tube(short *inn, int inNumSamples, int length);

typedef struct mindlin {
float x, xprime,xdobleprime, k, b, c, f0, T, p0;
}Mindlin;

void init_mindlin(Mindlin* mind, float b, float k, float c);
float mindlin_oscillate(Mindlin* mind);



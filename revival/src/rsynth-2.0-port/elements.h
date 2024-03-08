/* elements.h
*/
typedef struct
 {float stdy;
  float fixd;
  char  prop;
  char  ed;
  char  id;
 } interp_t, *interp_ptr;

enum Eparm_e
 {
  fn, f1, f2, f3, b1, b2, b3, an, a1, a2, a3, a4, a5, a6, ab, av, avc, asp, af,
  nEparm
 };

extern char *Ep_name[nEparm];

typedef struct Elm_s
 {
  char *name;
  char rk;
  char du;
  char ud;
  unsigned char font;
  char  *dict;
  char  *ipa;
  long  feat;
  interp_t p[nEparm];
 } Elm_t, *Elm_ptr;

extern Elm_t Elements[];
extern unsigned num_Elements;



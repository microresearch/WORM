#include <config.h>
/* elements.c
 */
#include <stdio.h>
#include <math.h>
#include "elements.h"
#include "phfeat.h"

Elm_t Elements[]  __attribute__ ((section (".flash"))) =
{
#include "Elements.def"
};

unsigned num_Elements = (sizeof(Elements) / sizeof(Elm_t));

char *Ep_name[nEparm] =
{
 "fn", "f1", "f2", "f3",
 "b1", "b2", "b3", "an",
 "a1", "a2", "a3", "a4",
 "a5", "a6", "ab", "av",
 "avc", "asp", "af"
};

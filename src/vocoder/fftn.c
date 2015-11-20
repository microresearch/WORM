/* $Id: fftn.c,v 1.4 2002/09/20 02:30:51 emanuel Exp $ */

/* (C) Copyright 1993 by Steven Trainoff.  Permission is granted to make
* any use of this code subject to the condition that all copies contain
* this notice and an indication of what has been changed.
*/
#include "spt.h"
#include <math.h>
#include "arm_math.h"

#ifndef REAL
#define REAL float // was double
#endif

#include "fft.h"
        
#ifndef PI
#define PI      3.14159265358979324
#endif

static const float c[256] __attribute__ ((section (".flash")))={1.000000, 0.999699, 0.998795, 0.997290, 0.995185, 0.992480, 0.989177, 0.985278, 0.980785, 0.975702, 0.970031, 0.963776, 0.956940, 0.949528, 0.941544, 0.932993, 0.923880, 0.914210, 0.903989, 0.893224, 0.881921, 0.870087, 0.857729, 0.844854, 0.831470, 0.817585, 0.803208, 0.788346, 0.773010, 0.757209, 0.740951, 0.724247, 0.707107, 0.689541, 0.671559, 0.653173, 0.634393, 0.615232, 0.595699, 0.575808, 0.555570, 0.534998, 0.514103, 0.492898, 0.471397, 0.449611, 0.427555, 0.405241, 0.382683, 0.359895, 0.336890, 0.313682, 0.290285, 0.266713, 0.242980, 0.219101, 0.195090, 0.170962, 0.146730, 0.122411, 0.098017, 0.073565, 0.049068, 0.024541, -0.000000, -0.024541, -0.049068, -0.073565, -0.098017, -0.122411, -0.146730, -0.170962, -0.195090, -0.219101, -0.242980, -0.266713, -0.290285, -0.313682, -0.336890, -0.359895, -0.382683, -0.405241, -0.427555, -0.449611, -0.471397, -0.492898, -0.514103, -0.534998, -0.555570, -0.575808, -0.595699, -0.615232, -0.634393, -0.653173, -0.671559, -0.689541, -0.707107, -0.724247, -0.740951, -0.757209, -0.773010, -0.788346, -0.803208, -0.817585, -0.831470, -0.844854, -0.857729, -0.870087, -0.881921, -0.893224, -0.903989, -0.914210, -0.923880, -0.932993, -0.941544, -0.949528, -0.956940, -0.963776, -0.970031, -0.975702, -0.980785, -0.985278, -0.989177, -0.992480, -0.995185, -0.997290, -0.998795, -0.999699, -1.000000, -0.999699, -0.998795, -0.997290, -0.995185, -0.992480, -0.989177, -0.985278, -0.980785, -0.975702, -0.970031, -0.963776, -0.956940, -0.949528, -0.941544, -0.932993, -0.923880, -0.914210, -0.903989, -0.893224, -0.881921, -0.870087, -0.857729, -0.844854, -0.831470, -0.817585, -0.803208, -0.788346, -0.773011, -0.757209, -0.740951, -0.724247, -0.707107, -0.689541, -0.671559, -0.653173, -0.634393, -0.615232, -0.595699, -0.575808, -0.555570, -0.534998, -0.514103, -0.492898, -0.471397, -0.449611, -0.427555, -0.405242, -0.382684, -0.359895, -0.336890, -0.313682, -0.290285, -0.266713, -0.242980, -0.219101, -0.195090, -0.170962, -0.146730, -0.122411, -0.098017, -0.073565, -0.049068, -0.024541, 0.000000, 0.024541, 0.049068, 0.073565, 0.098017, 0.122411, 0.146730, 0.170962, 0.195090, 0.219101, 0.242980, 0.266713, 0.290285, 0.313682, 0.336890, 0.359895, 0.382684, 0.405241, 0.427555, 0.449611, 0.471397, 0.492898, 0.514103, 0.534998, 0.555570, 0.575808, 0.595699, 0.615232, 0.634393, 0.653173, 0.671559, 0.689540, 0.707107, 0.724247, 0.740951, 0.757209, 0.773011, 0.788347, 0.803207, 0.817585, 0.831470, 0.844854, 0.857729, 0.870087, 0.881921, 0.893224, 0.903989, 0.914210, 0.923880, 0.932993, 0.941544, 0.949528, 0.956940, 0.963776, 0.970031, 0.975702, 0.980785, 0.985278, 0.989177, 0.992480, 0.995185, 0.997290, 0.998795, 0.999699};

static const float chalf[128] __attribute__ ((section (".flash")))={71.000000, 0.998795, 0.995185, 0.989177, 0.980785, 0.970031, 0.956940, 0.941544, 0.923880, 0.903989, 0.881921, 0.857729, 0.831470, 0.803208, 0.773010, 0.740951, 0.707107, 0.671559, 0.634393, 0.595699, 0.555570, 0.514103, 0.471397, 0.427555, 0.382683, 0.336890, 0.290285, 0.242980, 0.195090, 0.146730, 0.098017, 0.049068, -0.000000, -0.049068, -0.098017, -0.146730, -0.195090, -0.242980, -0.290285, -0.336890, -0.382683, -0.427555, -0.471397, -0.514103, -0.555570, -0.595699, -0.634393, -0.671559, -0.707107, -0.740951, -0.773010, -0.803208, -0.831470, -0.857729, -0.881921, -0.903989, -0.923880, -0.941544, -0.956940, -0.970031, -0.980785, -0.989177, -0.995185, -0.998795, -1.000000, -0.998795, -0.995185, -0.989177, -0.980785, -0.970031, -0.956940, -0.941544, -0.923880, -0.903989, -0.881921, -0.857729, -0.831470, -0.803208, -0.773011, -0.740951, -0.707107, -0.671559, -0.634393, -0.595699, -0.555570, -0.514103, -0.471397, -0.427555, -0.382684, -0.336890, -0.290285, -0.242980, -0.195090, -0.146730, -0.098017, -0.049068, 0.000000, 0.049068, 0.098017, 0.146730, 0.195090, 0.242980, 0.290285, 0.336890, 0.382684, 0.427555, 0.471397, 0.514103, 0.555570, 0.595699, 0.634393, 0.671559, 0.707107, 0.740951, 0.773011, 0.803207, 0.831470, 0.857729, 0.881921, 0.903989, 0.923880, 0.941544, 0.956940, 0.970031, 0.980785, 0.989177, 0.995185, 0.998795};

static const float magcos[128] __attribute__ ((section (".flash")))={70.999699, 0.998795, 0.997290, 0.995185, 0.992480, 0.989177, 0.985278, 0.980785, 0.975702, 0.970031, 0.963776, 0.956940, 0.949528, 0.941544, 0.932993, 0.923880, 0.914210, 0.903989, 0.893224, 0.881921, 0.870087, 0.857729, 0.844854, 0.831470, 0.817585, 0.803208, 0.788346, 0.773010, 0.757209, 0.740951, 0.724247, 0.707107, 0.689541, 0.671559, 0.653173, 0.634393, 0.615232, 0.595699, 0.575808, 0.555570, 0.534998, 0.514103, 0.492898, 0.471397, 0.449611, 0.427555, 0.405241, 0.382683, 0.359895, 0.336890, 0.313682, 0.290285, 0.266713, 0.242980, 0.219101, 0.195090, 0.170962, 0.146730, 0.122411, 0.098017, 0.073565, 0.049068, 0.024541, -0.000000, -0.024541, -0.049068, -0.073565, -0.098017, -0.122411, -0.146730, -0.170962, -0.195090, -0.219101, -0.242980, -0.266713, -0.290285, -0.313682, -0.336890, -0.359895, -0.382683, -0.405241, -0.427555, -0.449611, -0.471397, -0.492898, -0.514103, -0.534998, -0.555570, -0.575808, -0.595699, -0.615232, -0.634393, -0.653173, -0.671559, -0.689541, -0.707107, -0.724247, -0.740951, -0.757209, -0.773010, -0.788346, -0.803208, -0.817585, -0.831470, -0.844854, -0.857729, -0.870087, -0.881921, -0.893224, -0.903989, -0.914210, -0.923880, -0.932993, -0.941544, -0.949528, -0.956940, -0.963776, -0.970031, -0.975702, -0.980785, -0.985278, -0.989177, -0.992480, -0.995185, -0.997290, -0.998795, -0.999699};

static const float magsin[128] __attribute__ ((section (".flash")))={70.024541, 0.049068, 0.073565, 0.098017, 0.122411, 0.146730, 0.170962, 0.195090, 0.219101, 0.242980, 0.266713, 0.290285, 0.313682, 0.336890, 0.359895, 0.382683, 0.405241, 0.427555, 0.449611, 0.471397, 0.492898, 0.514103, 0.534998, 0.555570, 0.575808, 0.595699, 0.615232, 0.634393, 0.653173, 0.671559, 0.689541, 0.707107, 0.724247, 0.740951, 0.757209, 0.773010, 0.788346, 0.803208, 0.817585, 0.831470, 0.844854, 0.857729, 0.870087, 0.881921, 0.893224, 0.903989, 0.914210, 0.923880, 0.932993, 0.941544, 0.949528, 0.956940, 0.963776, 0.970031, 0.975702, 0.980785, 0.985278, 0.989177, 0.992480, 0.995185, 0.997290, 0.998795, 0.999699, 1.000000, 0.999699, 0.998795, 0.997290, 0.995185, 0.992480, 0.989177, 0.985278, 0.980785, 0.975702, 0.970031, 0.963776, 0.956940, 0.949528, 0.941544, 0.932993, 0.923880, 0.914210, 0.903989, 0.893224, 0.881921, 0.870087, 0.857729, 0.844854, 0.831470, 0.817585, 0.803208, 0.788346, 0.773010, 0.757209, 0.740951, 0.724247, 0.707107, 0.689541, 0.671559, 0.653173, 0.634393, 0.615232, 0.595699, 0.575808, 0.555570, 0.534998, 0.514103, 0.492898, 0.471397, 0.449611, 0.427555, 0.405241, 0.382683, 0.359895, 0.336890, 0.313682, 0.290285, 0.266713, 0.242980, 0.219101, 0.195090, 0.170962, 0.146731, 0.122411, 0.098017, 0.073564, 0.049068, 0.024541};

static const float shalf[128] __attribute__ ((section (".flash")))={70.000000, 0.049068, 0.098017, 0.146730, 0.195090, 0.242980, 0.290285, 0.336890, 0.382683, 0.427555, 0.471397, 0.514103, 0.555570, 0.595699, 0.634393, 0.671559, 0.707107, 0.740951, 0.773010, 0.803208, 0.831470, 0.857729, 0.881921, 0.903989, 0.923880, 0.941544, 0.956940, 0.970031, 0.980785, 0.989177, 0.995185, 0.998795, 1.000000, 0.998795, 0.995185, 0.989177, 0.980785, 0.970031, 0.956940, 0.941544, 0.923880, 0.903989, 0.881921, 0.857729, 0.831470, 0.803208, 0.773010, 0.740951, 0.707107, 0.671559, 0.634393, 0.595699, 0.555570, 0.514103, 0.471397, 0.427555, 0.382683, 0.336890, 0.290285, 0.242980, 0.195090, 0.146731, 0.098017, 0.049068, -0.000000, -0.049068, -0.098017, -0.146730, -0.195090, -0.242980, -0.290285, -0.336890, -0.382683, -0.427555, -0.471397, -0.514103, -0.555570, -0.595699, -0.634393, -0.671559, -0.707107, -0.740951, -0.773010, -0.803208, -0.831469, -0.857729, -0.881921, -0.903989, -0.923880, -0.941544, -0.956940, -0.970031, -0.980785, -0.989177, -0.995185, -0.998795, -1.000000, -0.998795, -0.995185, -0.989177, -0.980785, -0.970031, -0.956940, -0.941544, -0.923879, -0.903989, -0.881921, -0.857729, -0.831470, -0.803208, -0.773010, -0.740951, -0.707107, -0.671559, -0.634393, -0.595699, -0.555570, -0.514103, -0.471397, -0.427555, -0.382683, -0.336890, -0.290285, -0.242980, -0.195090, -0.146730, -0.098017, -0.049068};

static const float s[256] __attribute__ ((section (".flash")))={0.000000, 0.024541, 0.049068, 0.073565, 0.098017, 0.122411, 0.146730, 0.170962, 0.195090, 0.219101, 0.242980, 0.266713, 0.290285, 0.313682, 0.336890, 0.359895, 0.382683, 0.405241, 0.427555, 0.449611, 0.471397, 0.492898, 0.514103, 0.534998, 0.555570, 0.575808, 0.595699, 0.615232, 0.634393, 0.653173, 0.671559, 0.689541, 0.707107, 0.724247, 0.740951, 0.757209, 0.773010, 0.788346, 0.803208, 0.817585, 0.831470, 0.844854, 0.857729, 0.870087, 0.881921, 0.893224, 0.903989, 0.914210, 0.923880, 0.932993, 0.941544, 0.949528, 0.956940, 0.963776, 0.970031, 0.975702, 0.980785, 0.985278, 0.989177, 0.992480, 0.995185, 0.997290, 0.998795, 0.999699, 1.000000, 0.999699, 0.998795, 0.997290, 0.995185, 0.992480, 0.989177, 0.985278, 0.980785, 0.975702, 0.970031, 0.963776, 0.956940, 0.949528, 0.941544, 0.932993, 0.923880, 0.914210, 0.903989, 0.893224, 0.881921, 0.870087, 0.857729, 0.844854, 0.831470, 0.817585, 0.803208, 0.788346, 0.773010, 0.757209, 0.740951, 0.724247, 0.707107, 0.689541, 0.671559, 0.653173, 0.634393, 0.615232, 0.595699, 0.575808, 0.555570, 0.534998, 0.514103, 0.492898, 0.471397, 0.449611, 0.427555, 0.405241, 0.382683, 0.359895, 0.336890, 0.313682, 0.290285, 0.266713, 0.242980, 0.219101, 0.195090, 0.170962, 0.146731, 0.122411, 0.098017, 0.073564, 0.049068, 0.024541, -0.000000, -0.024541, -0.049068, -0.073565, -0.098017, -0.122411, -0.146730, -0.170962, -0.195090, -0.219101, -0.242980, -0.266713, -0.290285, -0.313682, -0.336890, -0.359895, -0.382683, -0.405241, -0.427555, -0.449611, -0.471397, -0.492898, -0.514103, -0.534998, -0.555570, -0.575808, -0.595699, -0.615232, -0.634393, -0.653173, -0.671559, -0.689541, -0.707107, -0.724247, -0.740951, -0.757209, -0.773010, -0.788346, -0.803208, -0.817585, -0.831469, -0.844853, -0.857729, -0.870087, -0.881921, -0.893224, -0.903989, -0.914210, -0.923880, -0.932993, -0.941544, -0.949528, -0.956940, -0.963776, -0.970031, -0.975702, -0.980785, -0.985278, -0.989177, -0.992480, -0.995185, -0.997290, -0.998795, -0.999699, -1.000000, -0.999699, -0.998795, -0.997290, -0.995185, -0.992480, -0.989177, -0.985278, -0.980785, -0.975702, -0.970031, -0.963776, -0.956940, -0.949528, -0.941544, -0.932993, -0.923879, -0.914210, -0.903989, -0.893224, -0.881921, -0.870087, -0.857729, 0.844853, -0.831470, -0.817585, -0.803208, -0.788346, -0.773010, -0.757209, -0.740951, -0.724247, -0.707107, -0.689541, -0.671559, -0.653173, -0.634393, -0.615231, -0.595699, -0.575808, -0.555570, -0.534998, -0.514103, -0.492898, -0.471397, -0.449612, -0.427555, -0.405241, -0.382683, -0.359895, -0.336890, -0.313682, -0.290285, -0.266713, -0.242980, -0.219101, -0.195090, -0.170962, -0.146730, -0.122411, -0.098017, -0.073565, -0.049068, -0.024541};

static const int rev[256] __attribute__ ((section (".flash")))={0, 128, 64, 192, 32, 160, 96, 224, 16, 144, 80, 208, 48, 176, 112, 240, 8, 136, 72, 200, 40, 168, 104, 232, 24, 152, 88, 216, 56, 184, 120, 248, 4, 132, 68, 196, 36, 164, 100, 228, 20, 148, 84, 212, 52, 180, 116, 244, 12, 140, 76, 204, 44, 172, 108, 236, 28, 156, 92, 220, 60, 188, 124, 252, 2, 130, 66, 194, 34, 162, 98, 226, 18, 146, 82, 210, 50, 178, 114, 242, 10, 138, 74, 202, 42, 170, 106, 234, 26, 154, 90, 218, 58, 186, 122, 250, 6, 134, 70, 198, 38, 166, 102, 230, 22, 150, 86, 214, 54, 182, 118, 246, 14, 142, 78, 206, 46, 174, 110, 238, 30, 158, 94, 222, 62, 190, 126, 254, 1, 129, 65, 193, 33, 161, 97, 225, 17, 145, 81, 209, 49, 177, 113, 241, 9, 137, 73, 201, 41, 169, 105, 233, 25, 153, 89, 217, 57, 185, 121, 249, 5, 133, 69, 197, 37, 165, 101, 229, 21, 149, 85, 213, 53, 181, 117, 245, 13, 141, 77, 205, 45, 173, 109, 237, 29, 157, 93, 221, 61, 189, 125, 253, 3, 131, 67, 195, 35, 163, 99, 227, 19, 147, 83, 211, 51, 179, 115, 243, 11, 139, 75, 203, 43, 171, 107, 235, 27, 155, 91, 219, 59, 187, 123, 251, 7, 135, 71, 199, 39, 167, 103, 231, 23, 151, 87, 215, 55, 183, 119, 247, 15, 143, 79, 207, 47, 175, 111, 239, 31, 159, 95, 223, 63, 191, 127, 255};

static const int revhalf[128] __attribute__ ((section (".flash")))={70, 64, 32, 96, 16, 80, 48, 112, 8, 72, 40, 104, 24, 88, 56, 120, 4, 68, 36, 100, 20, 84, 52, 116, 12, 76, 44, 108, 28, 92, 60, 124, 2, 66, 34, 98, 18, 82, 50, 114, 10, 74, 42, 106, 26, 90, 58, 122, 6, 70, 38, 102, 22, 86, 54, 118, 14, 78, 46, 110, 30, 94, 62, 126, 1, 65, 33, 97, 17, 81, 49, 113, 9, 73, 41, 105, 25, 89, 57, 121, 5, 69, 37, 101, 21, 85, 53, 117, 13, 77, 45, 109, 29, 93, 61, 125, 3, 67, 35, 99, 19, 83, 51, 115, 11, 75, 43, 107, 27, 91, 59, 123, 7, 71, 39, 103, 23, 87, 55, 119, 15, 79, 47, 111, 31, 95, 63, 127};

/* This routine performs a complex fft.  It takes two arrays holding
 * the real and imaginary parts of the the complex numbers.  It performs
 * the fft and returns the result in the original arrays.  It destroys
 * the orginal data in the process.  Note the array returned is NOT
 * normalized.  Each element must be divided by n to get dimensionally
 * correct results.  This routine takes optional arrays for the sines, cosine
 * and bitreversal.  If any of these pointers are null, all of the arrays are 
 * regenerated.
 */

void halffft(x,n)
REAL x[][2];            /* Input data points */
int n;
{
  int nu = 7;      /* Number of data points */ // n is NOT fixed
        int dual_space = n;     /* Spacing between dual nodes */
        int nu1 = nu;           /* = nu-1 right shift needed when finding p */
        int k;                          /* Iteration of factor array */
        register int i;                 /* Number of dual node pairs considered */
        register int j;                 /* Index into factor array */
        
        
        /* For each iteration of factor matrix */
        for (k = 0; k < nu; k++) {
                /* Initialize */
                dual_space /= 2;        /* Fewer elements in each set of duals */
                nu1--;                  /* nu1 = nu - 1 */

                /* For each set of duals */
                for(j = 0; j < n; j += dual_space) {
                        /* For each dual node pair */
                        for (i = 0; i < dual_space; i++, j++) {
                                REAL treal, timag;              /* Temp of w**p */
                                register int p = revhalf[j >> nu1];
                                
                                treal = x[j+dual_space][0]*chalf[p] + x[j+dual_space][1]*shalf[p];
                                timag = x[j+dual_space][1]*chalf[p] - x[j+dual_space][0]*shalf[p];

                                x[j+dual_space][0] = x[j][0] - treal;
                                x[j+dual_space][1] = x[j][1] - timag;

                                x[j][0] += treal;
                                x[j][1] += timag;
                        }
                }
        }

        /* We are done with the transform, now unscamble results */
        for (j = 0; j < n; j++) {
                if ((i = revhalf[j]) > j) {
                        REAL treal, timag;

                        /* Swap */
                        treal = x[j][0];
                        timag = x[j][1];

                        x[j][0] = x[i][0];
                        x[j][1] = x[i][1];

                        x[i][0] = treal;
                        x[i][1] = timag;
                }
        }
}



void fft(x,n)
REAL x[][2];            /* Input data points */
int n;
{
  //  int nu = ilog2(n);      /* Number of data points */ // n is NOT fixed
  int nu=8;
        int dual_space = n;     /* Spacing between dual nodes */
        int nu1 = nu;           /* = nu-1 right shift needed when finding p */
        int k;                          /* Iteration of factor array */
        register int i;                 /* Number of dual node pairs considered */
        register int j;                 /* Index into factor array */
        
        
        /* For each iteration of factor matrix */
        for (k = 0; k < nu; k++) {
                /* Initialize */
                dual_space /= 2;        /* Fewer elements in each set of duals */
                nu1--;                  /* nu1 = nu - 1 */

                /* For each set of duals */
                for(j = 0; j < n; j += dual_space) {
                        /* For each dual node pair */
                        for (i = 0; i < dual_space; i++, j++) {
                                REAL treal, timag;              /* Temp of w**p */
                                register int p = rev[j >> nu1];
                                
                                treal = x[j+dual_space][0]*c[p] + x[j+dual_space][1]*s[p];
                                timag = x[j+dual_space][1]*c[p] - x[j+dual_space][0]*s[p];

                                x[j+dual_space][0] = x[j][0] - treal;
                                x[j+dual_space][1] = x[j][1] - timag;

                                x[j][0] += treal;
                                x[j][1] += timag;
                        }
                }
        }

        /* We are done with the transform, now unscamble results */
        for (j = 0; j < n; j++) {
                if ((i = rev[j]) > j) {
                        REAL treal, timag;

                        /* Swap */
                        treal = x[j][0];
                        timag = x[j][1];

                        x[j][0] = x[i][0];
                        x[j][1] = x[i][1];

                        x[i][0] = treal;
                        x[i][1] = timag;
                }
        }
}

/* invfft performs an inverse fft */
void invfft(REAL (*x)[2], int n)
{
  //  int nu = ilog2(n);      /* Number of data points */
  int nu=8;
        int dual_space = n;     /* Spacing between dual nodes */
        int nu1 = nu;           /* = nu-1 right shift needed when finding p */
        int k;                          /* Iteration of factor array */
        register int i;                 /* Number of dual node pairs considered */
        register int j;                 /* Index into factor array */
        
        
        /* For each iteration of factor matrix */
        for (k = 0; k < nu; k++) {
                /* Initialize */
                dual_space /= 2;        /* Fewer elements in each set of duals */
                nu1--;                  /* nu1 = nu - 1 */

                /* For each set of duals */
                for(j = 0; j < n; j += dual_space) {
                        /* For each dual node pair */
                        for (i = 0; i < dual_space; i++, j++) {
                                REAL treal, timag;              /* Temp of w**p */
                                register int p = rev[j >> nu1];
                                
                                treal = x[j+dual_space][0]*c[p] + x[j+dual_space][1]*(-s[p]);
                                timag = x[j+dual_space][1]*c[p] - x[j+dual_space][0]*(-s[p]);

                                x[j+dual_space][0] = x[j][0] - treal;
                                x[j+dual_space][1] = x[j][1] - timag;

                                x[j][0] += treal;
                                x[j][1] += timag;
                        }
                }
        }

        /* We are done with the transform, now unscamble results */
        for (j = 0; j < n; j++) {
                if ((i = rev[j]) > j) {
                        REAL treal, timag;

                        /* Swap */
                        treal = x[j][0];
                        timag = x[j][1];

                        x[j][0] = x[i][0];
                        x[j][1] = x[i][1];

                        x[i][0] = treal;
                        x[i][1] = timag;
                }
        }
}



 /* This routine normalized the elements of a 1d fft by dividing by the number of elements
  * so that fft, inversefft, normalizefft is an identity
  */
void normalize_fft(REAL (*x)[2], int n)
{
    register int i;
    
    for (i = 0; i < n; i++) {
        x[i][0] /= n;
        x[i][1] /= n;
    }
}
 

/* This routine performs a complex fft on a single index of a
 * multidimensional array.  This routine is intended to be used
 * internally in a full multidimensional fft (on all indicies).  It will
 * be called repeatedly for each of the 1D fft's needed.  This routine is
 * designed primarily to optimize the memory fetches needed for the 1D
 * ffts.  Each element is an array by two holding the real and imaginary
 * parts of the the complex numbers.  Which index on which the fft is to
 * be performed is specified in a somewhat roundabout fashion.  What is
 * passed it an offset and the number of values between elements in the
 * array as if it were considered to be a big 1D array with dimension
 * equal to the product of the linear dimensions.  If the
 * multidimensional array is considered It performs the fft and returns
 * the result in the original arrays.  It destroys the orginal data in
 * the process.  Note the array returned is NOT normalized.  Each element
 * must be divided by n to get dimensionally correct results.  The cosine
 * and sine arrays are optional.  If null is passed, a temp array will be
 * allocated and filled, otherwise the passed arrays are assumed to have
 * the correct data in them.
 * 
 * Note: if the sin array is negated, the routine performs the inverse
 * transform.
 */

void fft1n(x, nu, offset, separation)
REAL x[][2];            /* Input data points */
int nu;                         /* Number of elements in fft n = 2**nu */
int offset;                     /* Offset of 1st element */
int separation;                 /* Separation between elements */
{
    int n = (1 << nu);          /* Number of data points */
    int dual_space = n; /* Spacing between dual nodes */
    int nu1 = nu;               /* = nu-1 right shift needed when finding p */
    int k;                              /* Iteration of factor array */
    register int i;                     /* Number of dual node pairs considered */
    register int j;             /* Index into factor array */
    
    
    x += offset;                /* Move to the correct offset */

    /* For each iteration of factor matrix */
    for (k = 0; k < nu; k++) {
        /* Initialize */
        dual_space /= 2;        /* Fewer elements in each set of duals */
        nu1--;                  /* nu1 = nu - 1 */
        
        /* For each set of duals */
        for(j = 0; j < n; j += dual_space) {
            
            /* For each dual node pair */
            for (i = 0; i < dual_space; i++, j++) {
                REAL *pt1, *pt2;
                REAL treal, timag;      /* Temp of w**p */
                register int p = rev[j >> nu1];
                
                pt1 = x[separation * j];
                pt2 = x[separation * (j+dual_space)];

                treal = pt2[0]*c[p] + pt2[1]*s[p];
                timag = pt2[1]*c[p] - pt2[0]*s[p];
                
                pt2[0] = pt1[0] - treal;
                pt2[1] = pt1[1] - timag;
                
                pt1[0] += treal;
                pt1[1] += timag;
            }
        }
    }
    
    
    /* We are done with the transform, now unscamble results */
    for (j = 0; j < n; j++) {
        if ((i = rev[j]) > j) {
            REAL *pt1, *pt2;
            REAL treal, timag;

            pt1 = x[j*separation];
            pt2 = x[i*separation];
            /* Swap */
            treal = pt1[0];
            timag = pt1[1];
            
            pt1[0] = pt2[0];
            pt1[1] = pt2[1];
            
            pt2[0] = treal;
            pt2[1] = timag;
        }
    }
    
     /* Give back the temp storage */
}

/* This routine takes an array of real numbers and performs a fft.  It
 * returns the magnitude of the fft in the original array.  This routine
 * uses an order n/2 complex ft and disentangles the results.  This is
 * much more efficient than using an order n complex fft with the
 * imaginary component set to zero.  We return the mean in data[0]
 * and the Nyquist frequency in data[n/w].  The rest of data is
 * left untouched.  The results are normalized.
 */

REAL xyz[128][2];                   /* Temp array used perform fft */

float vsqrtf(float op1) {
 float result;

 if(op1 <= 0.f)
  return 0.f;

 asm("vsqrt.f32 %0, %1" : "=w" (result) : "w" (op1) );
 return (result);
}

float fastsqrt1( float x ) {
  union { uint32_t i; float x; } u;
  u.x = x;
  u.i = ((uint32_t)1<<29) + (u.i >> 1) - ((uint32_t)1<<22); 
  return u.x;
}


// Log Base 2 Approximation with one extra Babylonian Step
// 2 times faster than sqrt()

float fastsqrt2( float x ) {
  float v=fastsqrt1( x );
  v = 0.5f * (v + x/v); // One Babylonian step
  return v;
}

// Log Base 2 Approximation with two extra Babylonian Steps
// 50% faster than sqrt()

float fastsqrt3( float x ) {
  float v=fastsqrt1( x );
  v =v + x/v;
  v = 0.25f* v + x/v; // Two Babylonian steps
  return v;
}


void realfftmag(data, n)
REAL *data;
int n;
{
        REAL *dataptr, *xptr;   /* Temp pointer into data array */
        int i;
        
        /* Load data into temp array
         * even terms end up in x[n][0] odd terms in x[n][1]
         */

        for (i = 0, dataptr = data, xptr = (REAL *)xyz; i < n; i++) 
	  *xptr++ = *dataptr++;
        
	halffft(xyz, n/2);

        /* Load results into output array */

        /* i = 0 needs to be treated separately */
        data[0] = (xyz[0][0] + xyz[0][1])/n;

        for (i = 1; i < n/2; i++) {
                float xr, xi;
                float  ti, tr;
		float cc, ss;            /* Cosine and sin */

		//                arg = 2.0f * PI * (float)i / (float)n; // AS LOOKUP n/2=128
                cc = magcos[i];   /* These are different c,s than used in fft */
                ss = magsin[i];

                ti = (xyz[i][1] + xyz[n/2-i][1]) / 2.0f;
                tr = (xyz[i][0] - xyz[n/2-i][0]) / 2.0f;

                xr = (xyz[i][0] + xyz[n/2-i][0])/2.0f + cc * ti - ss * tr;
                xi = (xyz[i][1] - xyz[n/2-i][1])/2.0f - ss * ti - cc * tr;

                xr /= n/2.0f;
                xi /= n/2.0f;
		data[i] = sqrtf(sqr(xr) + sqr(xi));
        }
        
        /* Nyquist frequency is returned in data[0] */
        data[n/2] = (xyz[0][0] - xyz[0][1])/n;
}
        




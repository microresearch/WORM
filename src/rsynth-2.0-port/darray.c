#include <config.h>
/* darray.c
 */
#ifndef LAP
#include "stm32f4xx.h"
#endif
#include "stdlib.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "darray.h"

void darray_free(darray_t *a)
{
	if (a->data)
	{
		free(a->data);
		a->data = NULL;
	}
	a->items = a->alloc = 0;
}

/*void *Darray_find(darray_t *a, unsigned int n)
{
	if (n >= a->alloc || n >= a->items)
	{
		unsigned int osize = a->items * a->esize;
		unsigned int nsize;
		if (!a->esize)
		  //			abort();
				return NULL;
		if (n >= a->alloc)
		{
			unsigned int add = (a->get) ? a->get : 1;
			char *ndata = (char *) malloc(nsize = (n + add) * a->esize);
			if (ndata)
			{
				if (osize)
					memcpy(ndata, a->data, osize);
				if (a->data)
					free(a->data);
				a->data = ndata;
				a->alloc = n + add;
			}
			else{
			  //				fprintf(stderr, "memory shortage (Darray_find)\n");
				return NULL;
			}
		}
		else
			nsize = (n + 1) * a->esize;
		if (n >= a->items)
		{
			memset(a->data + osize, 0, nsize - osize);
			a->items = n + 1;
		}
	}
	return (void *) (a->data + n * a->esize);
}
*/
int darray_delete(darray_t *a, unsigned int n)
{
	char *p = (char *) darray_find(a, n);
	if (p)
	{
		if (a->items)
		{
			a->items--;
			while (n++ < a->items)
			{
				memcpy(p, p + a->esize, a->esize);
				p += a->esize;
			}
			memset(p, 0, a->esize);
			return 1;
		}
		else
				return 0;
		  //			abort();
	}
	else
		return 0;
}

#include <config.h>

/* phtoelm.c
 */

#include "stm32f4xx.h"
//#include "platform_config.h"
//#include "hw_config.h"
#include "stdlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <useconfig.h>
#include <string.h>
#include "elements.h"
#include "phfeat.h"
#include "darray.h"
#include "trie.h"
#include "phtoelm.h"
//#include "hplay.h"
#include "holmes.h"
#include "nsynth.h"



trie_ptr phtoelm = NULL;

static Elm_ptr find_elm (char *s);

static Elm_ptr find_elm(char *s)
{
	Elm_ptr e = Elements;
	while (e < Elements + num_Elements)
	{
		if (!strcmp(s, e->name))
		{
			return e;
		}
		e++;
	}
	return NULL;
}

static void enter(char *p,...)
{
	va_list ap;
	
	char *s;
	char buf[20];
	char *x = buf + 1;
	
	va_start(ap, p);
	
	//printf("-- p %s\n", p);
	
	//while ((s = va_arg(ap, char *)))
	while (1)
	{
		s = va_arg(ap, char *);
		if(s==NULL) break;
		//printf("-- va_arg %s\n", s);
		
		Elm_ptr e = find_elm(s);
		if (e)
		{
			//printf("---- Find %s\n", s);
			
			*x++ = (e - Elements);
		}
		else
		{
			fprintf(stderr, "Cannot find %s\n", s);
		}
	}
	va_end(ap);
	buf[0] = (x - buf) - 1;
	x = malloc(buf[0] + 1);
	if(x){
		memcpy(x, buf, buf[0] + 1);
		trie_insert(&phtoelm, p, x);
	}else{
		fprintf(stderr, "memory shortage (enter)\n");
	}
}

static void enter_phonemes (void)
{
	#include "phtoelm.def"
}

int phone_append(darray_ptr p, int ch)
{
	char *s = (char *) darray_find(p, p->items);
	*s = ch;
	return ch;
}

//yama #if 0
#if 1
#define StressDur(e,s) ((e->ud + (e->du - e->ud) * s / 3)*speed)
#else
#define StressDur(e,s) (s,((e->du + e->ud)/2)*speed)
#endif

unsigned int phone_to_elm(char *phone, int n, darray_ptr elm)
{
	int stress = 0;
	char *s = phone;
	unsigned t = 0;
	char *limit = s + n;
	
	
	if (!phtoelm)
	  enter_phonemes();
	
	while (s < limit && *s)
	{
		char *e = trie_lookup(&phtoelm, &s);
		if (e)
		{
			int n = *e++;
			while (n-- > 0)
			{
				int x = *e++;
				Elm_ptr p = &Elements[x];
				/* This works because only vowels have ud != du,
				and we set stress just before a vowel
				*/
				phone_append(elm, x);
				if (!(p->feat & vwl))
					stress = 0;
				t += phone_append(elm,StressDur(p,stress));
				phone_append(elm, stress);
			}
		}
		else
		{
			char ch = *s++;
			switch (ch)
			{
				case '\'':                /* Primary stress */
				stress = 3;
				break;
				case ',':                 /* Secondary stress */
				stress = 2;
				break;
				case '+':                 /* Tertiary stress */
				stress = 1;
				break;
				case '-':                 /* hyphen in input */
				break;
				default:
				fprintf(stderr, "Ignoring %c in '%.*s'\n", ch, n, phone);
				break;
			}
		}
	}
	return t;
}





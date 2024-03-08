#include <stdio.h>
#include <ctype.h>
#include <string.h>


#define FALSE (0)
#define TRUE (!0)

/*
**	English to Phoneme translation.
**
**	Rules are made up of four parts:
**	
**		The left context.
**		The text to match.
**		The right context.
**		The phonemes to substitute for the matched text.
**
**	Procedure:
**
**		Seperate each block of letters (apostrophes included) 
**		and add a space on each side.  For each unmatched 
**		letter in the word, look through the rules where the 
**		text to match starts with the letter in the word.  If 
**		the text to match is found and the right and left 
**		context patterns also match, output the phonemes for 
**		that rule and skip to the next unmatched letter.
**
**
**	Special Context Symbols:
**
**		#	One or more vowels
**		:	Zero or more consonants
**		^	One consonant.
**		.	One of B, D, V, G, J, L, M, N, R, W or Z (voiced 
**			consonants)
**		%	One of ER, E, ES, ED, ING, ELY (a suffix)
**			(Right context only)
**		+	One of E, I or Y (a "front" vowel)
*/

//typedef char *Rule[4];	/* A rule is four character pointers */

typedef struct{
  char *rulee[3];
  char oot[8];
} Rule;

#define MAX_LENGTH 128

extern Rule *Rules[];	/* An array of pointers to rules */

extern char output_array[MAX_LENGTH]; /*Array of ints to output*/
extern int output_count; /*Count of ints currently in output array*/

int leftmatch(char* pattern, char* context);
int rightmatch(char* pattern, char* context);
int find_rule(char word[], int index, Rule *rules);


int isvowel(chr)
	char chr;
	{
	return (chr == 'A' || chr == 'E' || chr == 'I' || 
		chr == 'O' || chr == 'U');
	}

int isconsonant(chr)
	char chr;
	{
	return (chr >= 'A' && chr <= 'Z' && !isvowel(chr));
	}

void xlate_word(char word[])
	{
	int index;	/* Current position in word */
	int type;	/* First letter of match part */

	index = 1;	/* Skip the initial blank */
	do
		{
//		if (isupper(word[index]))
  if (word[index] >= 'A' && word[index] <= 'Z')
    			type = word[index] - 'A' + 1;
		else
			type = 0;

		index = find_rule(word, index, Rules[type]);
		}
	while (word[index] != '\0');
	}

void outnum(const char* ooo){
  // run through until we get to -1 end
  while (*ooo != -1){
//    printf("%d ", *ooo);
    output_array[output_count]=*ooo;
    output_count++;
   ooo++;
  }
}


int find_rule(word, index, rules)
	char word[];
	int index;
	Rule *rules;
	{
	Rule *rule;
	char *left, *match, *right, *output;
	int remainder;

	for (;;)	/* Search for the rule */
		{
		rule = rules++;
		match = (*rule).rulee[1];

		if (match == 0)	/* bad symbol! */
			{
//			fprintf(stderr,"Error: Can't find rule for: '%c' in \"%s\"\n", word[index], word);
			return index+1;	/* Skip it! */
			}

		for (remainder = index; *match != '\0'; match++, remainder++)
			{
			if (*match != word[remainder])
				break;
			}

		if (*match != '\0')	/* found missmatch */
			continue;
/*
oprintf("\nWord: \"%s\", Index:%4d, Trying: \"%s/%s/%s\" = \"%s\"\n",
    word, index, (*rule)[0], (*rule)[1], (*rule)[2], (*rule)[3]);
*/
		left = (*rule).rulee[0];
		right = (*rule).rulee[2];

		if (!leftmatch(left, &word[index-1]))
			continue;
/*
printf("leftmatch(\"%s\",\"...%c\") succeded!\n", left, word[index-1]);
*/
		if (!rightmatch(right, &word[remainder]))
			continue;
/*
printf("rightmatch(\"%s\",\"%s\") succeded!\n", right, &word[remainder]);
*/
//		output = (*rule)[3];
/*
printf("Success: ");
*/
//		outstring(output);
		outnum((*rule).oot);
		return remainder;
		}
	}


int leftmatch(pattern, context)
	char *pattern;	/* first char of pattern to match in text */
	char *context;	/* last char of text to be matched */
	{
	char *pat;
	char *text;
	int count;

	if (*pattern == '\0')	/* null string matches any context */
		{
		return TRUE;
		}

	/* point to last character in pattern string */
	count = strlen(pattern);
	pat = pattern + (count - 1);

	text = context;

	for (; count > 0; pat--, count--)
		{
		/* First check for simple text or space */
		if (isalpha(*pat) || *pat == '\'' || *pat == ' ')
			if (*pat != *text)
				return FALSE;
			else
				{
				text--;
				continue;
				}

		switch (*pat)
			{
		case '#':	/* One or more vowels */
			if (!isvowel(*text))
				return FALSE;

			text--;

			while (isvowel(*text))
				text--;
			break;

		case ':':	/* Zero or more consonants */
			while (isconsonant(*text))
				text--;
			break;

		case '^':	/* One consonant */
			if (!isconsonant(*text))
				return FALSE;
			text--;
			break;

		case '.':	/* B, D, V, G, J, L, M, N, R, W, Z */
			if (*text != 'B' && *text != 'D' && *text != 'V'
			   && *text != 'G' && *text != 'J' && *text != 'L'
			   && *text != 'M' && *text != 'N' && *text != 'R'
			   && *text != 'W' && *text != 'Z')
				return FALSE;
			text--;
			break;

		case '+':	/* E, I or Y (front vowel) */
			if (*text != 'E' && *text != 'I' && *text != 'Y')
				return FALSE;
			text--;
			break;

		case '%':
		default:
//			fprintf(stderr, "Bad char in left rule: '%c'\n", *pat);
			return FALSE;
			}
		}

	return TRUE;
	}


int rightmatch(pattern, context)
	char *pattern;	/* first char of pattern to match in text */
	char *context;	/* last char of text to be matched */
	{
	char *pat;
	char *text;

	if (*pattern == '\0')	/* null string matches any context */
		return TRUE;

	pat = pattern;
	text = context;

	for (pat = pattern; *pat != '\0'; pat++)
		{
		/* First check for simple text or space */
		if (isalpha(*pat) || *pat == '\'' || *pat == ' ')
			if (*pat != *text)
				return FALSE;
			else
				{
				text++;
				continue;
				}

		switch (*pat)
			{
		case '#':	/* One or more vowels */
			if (!isvowel(*text))
				return FALSE;

			text++;

			while (isvowel(*text))
				text++;
			break;

		case ':':	/* Zero or more consonants */
			while (isconsonant(*text))
				text++;
			break;

		case '^':	/* One consonant */
			if (!isconsonant(*text))
				return FALSE;
			text++;
			break;

		case '.':	/* B, D, V, G, J, L, M, N, R, W, Z */
			if (*text != 'B' && *text != 'D' && *text != 'V'
			   && *text != 'G' && *text != 'J' && *text != 'L'
			   && *text != 'M' && *text != 'N' && *text != 'R'
			   && *text != 'W' && *text != 'Z')
				return FALSE;
			text++;
			break;

		case '+':	/* E, I or Y (front vowel) */
			if (*text != 'E' && *text != 'I' && *text != 'Y')
				return FALSE;
			text++;
			break;

		case '%':	/* ER, E, ES, ED, ING, ELY (a suffix) */
			if (*text == 'E')
				{
				text++;
				if (*text == 'L')
					{
					text++;
					if (*text == 'Y')
						{
						text++;
						break;
						}
					else
						{
						text--; /* Don't gobble L */
						break;
						}
					}
				else
				if (*text == 'R' || *text == 'S' 
				   || *text == 'D')
					text++;
				break;
				}
			else
			if (*text == 'I')
				{
				text++;
				if (*text == 'N')
					{
					text++;
					if (*text == 'G')
						{
						text++;
						break;
						}
					}
				return FALSE;
				}
			else
			return FALSE;

		default:
//			fprintf(stderr, "Bad char in right rule:'%c'\n", *pat);
			return FALSE;
			}
		}

	return TRUE;
	}

#include <stdio.h>

/*
**              Integer to Readable ASCII Conversion Routine.
**
** Synopsis:
**
**      say_cardinal(value)
**      	long int     value;          -- The number to output
**
**	The number is translated into a string of phonemes
**
*/

void outnum(const char* ooo);

static const char mayn[20] __attribute__ ((section (".flash"))) ={31,13,32,12,26,41,-1};

static const char infin[20] __attribute__ ((section (".flash"))) ={1,32,22,1,32,1,18,0,41,-1};

static const char bihl[20] __attribute__ ((section (".flash"))) ={17,1,34,0,11,32,41,-1};

static const char end[20] __attribute__ ((section (".flash"))) ={4,32,19,41,-1};

static const char mil[20] __attribute__ ((section (".flash"))) ={31,1,34,0,11,32,-1};

static const char thaw[20] __attribute__ ((section (".flash"))) ={24,14,27,4,32,19,41,-1};

static const char hahn[20] __attribute__ ((section (".flash"))) ={30,12,32,19,37,3,19,41,-1};

static const char Cardinals[20][20] __attribute__ ((section (".flash"))) ={
  {27,1,37,7,41,-1},{35,12,32,41,-1},{18,9,41,-1},{24,37,0,41,-1},{22,7,37,41,-1},{22,13,23,41,-1},{26,1,20,26,41,-1},{26,3,23,11,32,41,-1},{2,18,41,-1},{32,13,32,41,-1},{18,3,32,41,-1},{0,34,3,23,11,32,41,-1},{18,35,3,34,23,41,-1},{24,10,18,0,32,41,-1},{22,7,37,18,0,32,41,-1},{22,1,22,18,0,32,41,-1},{26,1,20,26,18,0,32,41,-1},{26,3,23,3,32,18,0,32,41,-1},{2,18,0,32,41,-1},{32,13,32,18,0,32,41,-1}
};

static const char Twenties[8][20] __attribute__ ((section (".flash"))) = {
  {18,35,3,32,18,0,41,-1},{24,10,18,0,41,-1},{22,6,37,18,0,41,-1},{22,1,22,18,0,41,-1},{26,1,20,26,18,0,41,-1},{26,3,23,3,32,18,0,41,-1},{2,18,0,41,-1},{32,13,32,18,0,41,-1}
  };

static const char Ordinals[20][20]  __attribute__ ((section (".flash"))) = {
  {27,1,37,7,3,24,41,-1},{22,10,26,18,41,-1},{26,3,20,12,32,19,41,-1},{24,10,19,41,-1},{22,7,37,24,41,-1},{22,1,22,24,41,-1},{26,1,20,26,24,41,-1},{26,3,23,3,32,24,41,-1},{2,18,24,41,-1},{32,13,32,24,41,-1},{18,3,32,24,41,-1},{0,34,3,23,3,32,24,41,-1},{18,35,3,34,23,24,41,-1},{24,10,18,0,32,24,41,-1},{22,6,37,18,0,32,24,41,-1},{22,1,22,18,0,32,24,41,-1},{26,1,20,26,18,0,32,24,41,-1},{26,3,23,3,32,18,0,32,24,41,-1},{2,18,0,32,24,41,-1},{32,13,32,18,0,32,24,41,-1}
};

static const char Ord_twenties[8][20]  __attribute__ ((section (".flash"))) = {
  {18,35,3,32,18,0,3,24,41,-1},{24,10,18,0,3,24,41,-1},{22,7,37,18,0,3,24,41,-1},{22,1,22,18,0,3,24,41,-1},{26,1,20,26,18,0,3,24,41,-1},{26,3,23,3,32,18,0,3,24,41,-1},{2,18,0,3,24,41,-1},{32,13,32,18,0,3,24,41,-1}
};


/*
** Translate a number to phonemes.  This version is for CARDINAL numbers.
**	 Note: this is recursive.
*/
void say_cardinal(long int value)
	{
	if (value < 0)
		{
		  //		outstring("mAYnAHs ");
		  outnum(mayn);
		value = (-value);
		if (value < 0)	/* Overflow!  -32768 */
			{
			  //			outstring("IHnfIHnIHtIY ");
			  outnum(infin);
			  return;
			}
		}

	if (value >= 1000000000L)	/* Billions */
		{
		say_cardinal(value/1000000000L);
		//		outstring("bIHlIYAXn ");
		  outnum(bihl);
		value = value % 1000000000;
		if (value == 0)
			return;		/* Even billion */
				if (value < 100)	/* as in THREE BILLION AND FIVE */
		//			outstring("AEnd ");
				  outnum(end);
		}

	if (value >= 1000000L)	/* Millions */
		{
		say_cardinal(value/1000000L);
		//		outstring("mIHlIYAXn ");
		outnum(mil);
		value = value % 1000000L;
		if (value == 0)
			return;		/* Even million */
		if (value < 100)	/* as in THREE MILLION AND FIVE */
		  outnum(end);
		  //			outstring("AEnd ");
		}

	/* Thousands 1000..1099 2000..99999 */
	/* 1100 to 1999 is eleven-hunderd to ninteen-hunderd */
	if ((value >= 1000L && value <= 1099L) || value >= 2000L)
		{
		say_cardinal(value/1000L);
		//		outstring("THAWzAEnd ");
		outnum(thaw);
		value = value % 1000L;
		if (value == 0)
			return;		/* Even thousand */
				if (value < 100)	/* as in THREE THOUSAND AND FIVE */
				  outnum(end);
		  //	outstring("AEnd ");
		}

	if (value >= 100L)
		{
		  outnum(Cardinals[value/100]);
		//		outstring("hAHndrEHd ");
		  outnum(hahn);
		value = value % 100;
		if (value == 0)
			return;		/* Even hundred */
		}

	if (value >= 20)
		{
		  outnum(Twenties[(value-20)/ 10]);
		value = value % 10;
		if (value == 0)
			return;		/* Even ten */
		}

	outnum(Cardinals[value]);
	return;
	} 


/*
** Translate a number to phonemes.  This version is for ORDINAL numbers.
**	 Note: this is recursive.
*/
void say_ordinal(long int value)
	{

	if (value < 0)
		{
		  //		outstring("mAHnAXs ");
		  outnum(mayn);
		value = (-value);
		if (value < 0)	/* Overflow!  -32768 */
			{
			  //			outstring("IHnfIHnIHtIY ");
		  outnum(infin);
			return;
			}
		}

	if (value >= 1000000000L)	/* Billions */
		{
		say_cardinal(value/1000000000L);
		value = value % 1000000000;
		if (value == 0)
			{
			  //		outstring("bIHlIYAXnTH ");
			  outnum(bihl);
			return;		/* Even billion */
			}
		  outnum(bihl);
		//		outstring("bIHlIYAXn ");
				if (value < 100)	/* as in THREE BILLION AND FIVE */
				  		  outnum(end);
		  //	  //			outstring("AEnd ");
		}

	if (value >= 1000000L)	/* Millions */
		{
		say_cardinal(value/1000000L);
		value = value % 1000000L;
		if (value == 0)
			{
			  //			outstring("mIHlIYAXnTH ");
		  outnum(mil);
			return;		/* Even million */
			}
		//		outstring("mIHlIYAXn ");
		outnum(mil);
				if (value < 100)	/* as in THREE MILLION AND FIVE */
				  		  outnum(end);
		  //	outstring("AEnd ");
		}

	/* Thousands 1000..1099 2000..99999 */
	/* 1100 to 1999 is eleven-hunderd to ninteen-hunderd */
	if ((value >= 1000L && value <= 1099L) || value >= 2000L)
		{
		say_cardinal(value/1000L);
		value = value % 1000L;
		if (value == 0)
			{
			  //	outstring("THAWzAEndTH ");
		  outnum(thaw);
			return;		/* Even thousand */
			}
		//		outstring("THAWzAEnd ");
				if (value < 100)	/* as in THREE THOUSAND AND FIVE */
		  outnum(end);
		  //			outstring("AEnd ");
		}

	if (value >= 100L)
		{
		  outnum(Cardinals[value/100]);
		value = value % 100;
		if (value == 0)
			{
			  //	outstring("hAHndrEHdTH ");
			  		  outnum(hahn);
			return;		/* Even hundred */
			}
		//		outstring("hAHndrEHd ");
		  outnum(hahn);
		}

	if (value >= 20)
		{
		if ((value%10) == 0)
			{
			  outnum(Ord_twenties[(value-20)/ 10]);
			return;		/* Even ten */
			}
		outnum(Twenties[(value-20)/ 10]);
		value = value % 10;
		}

	outnum(Ordinals[value]);
	return;
	} 

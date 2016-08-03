#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_LENGTH 128

//static FILE *In_file;
//static FILE *Out_file;

static int Char, Char1, Char2, Char3;
static int input_count;
static int input_length;
static char *input_array;
char output_array[MAX_LENGTH];
int output_count = 0;

static const char poynt[5]  __attribute__ ((section (".flash"))) ={16, 15, 32, 18, 41};

// remap this index 0->42 to the 256 phonemes which are

/*NRLIPAtoSPO256 = { 'AA':'AA', 'AE':'AE', 'AH':'AX AX', 'AO':'AO', 'AW':'AW',  'AX':'AX',
                   'AY':'AY', 'b':'BB1', 'CH':'CH',  'd':'DD1', 'DH':'DH1', 'EH':'EH',
                   'ER':'ER1','EY':'EY', 'f':'FF',   'g':'GG2', 'h':'HH1',  'IH':'IH',
                   'IY':'IY', 'j':'JH',  'k':'KK1',  'l':'LL',  'm':'MM',   'n':'NN1',
                   'NG':'NG', 'OW':'OW', 'OY':'OY',  'p':'PP',  'r':'RR1',  's':'SS',
                   'SH':'SH', 't':'TT1', 'TH':'TH',  'UH':'UH',  'UW':'UW2','v':'VV',
                   'w':'WW', 'WH':'WH', 'y':'YY1', 'z':'ZZ', 'ZH':'ZH', 'PAUSE':'PA4' };*/

// but what are numbers for 256
/* 


IY, 0, IH, 1, EY, 2, EH, 3, AE, 4, AA, 5, AO, 6, OW, 7, UH, 8, UW, 9, ER, 10, AX, 11, AH, 12, AY, 13, AW, 14, OY, 15, p, 16, b, 17, t, 18, d, 19, k, 20, g, 21, f, 22, v, 23, TH, 24, DH, 25, s, 26, z, 27, SH, 28, ZH, 29, h, 30, m, 31, n, 32, NG, 33, l, 34, w, 35, y, 36, r, 37, CH, 38, j, 39, WH, 40, PAUSE, 41, "", 42

*/

/*    _allophones = { 'PA1':0, 'PA2':1, 'PA3':2, 'PA4':3, 'PA5':4, 'OY':5, 'AY':6, 'EH':7, 
             'KK3':8, 'PP':9, 'JH':10, 'NN1':11, 'IH':12, 'TT2':13, 'RR1':14, 'AX':15, 
             'MM':16, 'TT1':17, 'DH1':18, 'IY':19, 'EY':20, 'DD1':21, 'UW1':22, 'AO':23, 
             'AA':24, 'YY2':25, 'AE':26, 'HH1':27, 'BB1':28, 'TH':29, 'UH':30, 'UW2':31, 
             'AW':32, 'DD2':33, 'GG3':34, 'VV':35, 'GG1':36, 'SH':37, 'ZH':38, 'RR2':39, 
             'FF':40, 'KK2':41, 'KK1':42, 'ZZ':43, 'NG':44, 'LL':45, 'WW':46, 'XR':47, 
             'WH':48, 'YY1':49, 'CH':50, 'ER1':51, 'ER2':52, 'OW':53, 'DH2':54, 'SS':55, 
             'NN2':56, 'HH2':57, 'OR':58, 'AR':59, 'YR':60, 'GG2':61, 'EL':62, 'BB2':63 };*/

static const char remap256[43]  __attribute__ ((section (".flash"))) ={19, 12, 20, 7, 26, 24, 23, 53, 30, 31, 51, 15, 15, 6, 32, 5, 9, 28, 17, 21, 42, 61, 40, 35, 29, 18, 55, 43, 37, 38, 27, 16, 11, 44, 45, 46, 49, 14, 50, 10, 48, 3, 3}; // what about silence and case 12=AX AX?

void xlate_word(char word[]);
void spell_word(char word[]);
void say_ascii(int character);
void xlate_file();
int text2speech(int input_len, char *input, char *output);
void have_number();
void have_letter();
void have_special();
void say_cardinal(long int value);
void say_ordinal(long int value);
void outnum(const char* ooo);

/*
void main(argc, argv)
	int argc;
	char *argv[];
	{
 
  //allocate space for the output
	  char output[MAX_LENGTH];
	  //	  char output[MAX_LENGTH*MAX_PHONEME_LENGTH];

	  //	  for(int i = 0; i < MAX_LENGTH; i++){
	  //    output_array[i] = output[i];
	  //    }
  
  //allocate space for the input
  int num_words = argc - 1;
  int input_size = 0;
  int space_count = 0;
  for(int i = 0; i < num_words; i++){
      input_size += strlen(argv[1 + i]);
      space_count += 1;
  }
  char input[input_size + space_count];
  
  //initialize input array to given arguments
  int index = 0;

  //  input[index] = EOF;

  static char TTSinarray[64]={"testing "};
 static char TTSoutarray[128];

 TTSinarray[8]=EOF;
  //  u8 TTSlength= text2speechfor256(9,TTSinarray,TTSoutarray); // 7 is length how? or is fixed?


  //  printf("INEDX %d in %c\n\n", index, input[4]);

  
  //transform text to integer code phonemes
  int output_count = text2speechfor256(9,TTSinarray,TTSoutarray);
  for(int i = 0; i < output_count; i++){
    //    for(int j = 0; j < strlen(output[i]); j++){
             printf("%d, ", TTSoutarray[i]);
	     //	      }
	     //      printf("\n");
  }
  //  return output_count;
  }
*/

/*
** Transforms text to integer code phonemes.
*/
int text2speech(int input_len, char *input, char *output){
  input_array = input;
  input_length = input_len;
  input_count = 0; output_count=0;
  xlate_file();
  for (char i=0;i<output_count;i++){
    output[i]=output_array[i]; 
  }
  return output_count;
}

int text2speechfor256(int input_len, char *input, char *output){
  input_array = input;
  input_length = input_len;
  input_count = 0; output_count=0;
  xlate_file();
  //      output_count=10;
  for (char i=0;i<output_count;i++){
        output[i]=remap256[output_array[i]];
	if (output_array[i]==12){
	  output_array[++i]=12;
	  output_count++;
	}
	//       output[i]=remap256[rand()%43];
  }
  return output_count;
}

int makeupper(character)
	int character;
	{
	  //	  if (islower(character)){
	  if (character >= 'a' && character <= 'z'){
	      character -= ('a' - 'A');
	return character;

		  }
	else
		return character;
	}

int new_char()
	{
	/*
	If the cache is full of newline, time to prime the look-ahead
	again.  If an EOF is found, fill the remainder of the queue with
	EOF's. Read chars from input array.
	*/
	if (Char == '\n'  && Char1 == '\n' && Char2 == '\n' && Char3 == '\n')
		{	/* prime the pump again */
		
		if (input_count >= input_length)
			{
      Char = EOF;
			Char1 = EOF;
			Char2 = EOF;
			Char3 = EOF;
			return Char;
			}
    Char = input_array[input_count];
    input_count++;
		if (Char == '\n')
			return Char;

		if (input_count >= input_length)
			{
      Char1 = EOF;
			Char2 = EOF;
			Char3 = EOF;
			return Char;
			}
    Char1 = input_array[input_count];
    input_count++;
		if (Char1 == '\n')
			return Char;

		if (input_count >= input_length)
			{
      Char2 = EOF;
			Char3 = EOF;
			return Char;
			}
    Char2 = input_array[input_count];
    input_count++;
		if (Char2 == '\n')
			return Char;

		Char3 = input_array[input_count];
    input_count++;
		}
	else
		{
		/*
		Buffer not full of newline, shuffle the characters and
		either get a new one or propagate a newline or EOF.
		*/
		Char = Char1;
		Char1 = Char2;
		Char2 = Char3;
		if (Char3 != '\n' && Char3 != EOF)
			Char3 = input_array[input_count];
      input_count++;
		}
	return Char;
}

/*
** xlate_file()
**
**	This is the input file (now input array) translator.  It sets up the first character
**	and uses it to determine what kind of text follows.
*/
void xlate_file()
	{
	/* Prime the queue */
	Char = '\n';
	Char1 = '\n';
	Char2 = '\n';
	Char3 = '\n';
	new_char();	/* Fill Char, Char1, Char2 and Char3 */

	while (Char != EOF)	/* All of the words in the file */
		{
		if (isdigit(Char))
			have_number();
		else
		if (isalpha(Char) || Char == '\'')
			have_letter();
		//		else
		  //	if (Char == '$' && isdigit(Char1))
		  //	have_dollars();
		else
			have_special();
		}
	}


void have_special()
	{
	/*if (Char == '\n')
		outchar('\n');
	else*/
	if (!isspace(Char))
		say_ascii(Char);

	new_char();
	return;
	}


void have_number()
	{
	  //  fprintf(stderr, "Cannot read numerical values. Please enter alpha text.\n");
	  //  exit(1);
	long int value;
	int lastdigit;

	value = Char - '0';
	lastdigit = Char;

	for (new_char() ; isdigit(Char) ; new_char())
		{
		value = 10 * value + (Char-'0');
		lastdigit = Char;
		}

	/* Recognize ordinals based on last digit of number */
	switch (lastdigit)
		{
	case '1':	/* ST */
		if (makeupper(Char) == 'S' && makeupper(Char1) == 'T' &&
		    !isalpha(Char2) && !isdigit(Char2))
			{
			say_ordinal(value);
			new_char();	/* Used Char */
			new_char();	/* Used Char1 */
			return;
			}
		break;

	case '2':	/* ND */
		if (makeupper(Char) == 'N' && makeupper(Char1) == 'D' &&
		    !isalpha(Char2) && !isdigit(Char2))
			{
			say_ordinal(value);
			new_char();	/* Used Char */
			new_char();	/* Used Char1 */
			return;
			}
		break;

	case '3':	/* RD */
		if (makeupper(Char) == 'R' && makeupper(Char1) == 'D' &&
		    !isalpha(Char2) && !isdigit(Char2))
			{
			say_ordinal(value);
			new_char();	/* Used Char */
			new_char();	/* Used Char1 */
			return;
			}
		break;

	case '0':	/* TH */
	case '4':	/* TH */
	case '5':	/* TH */
	case '6':	/* TH */
	case '7':	/* TH */
	case '8':	/* TH */
	case '9':	/* TH */
		if (makeupper(Char) == 'T' && makeupper(Char1) == 'H' &&
		    !isalpha(Char2) && !isdigit(Char2))
			{
			say_ordinal(value);
			new_char();	/* Used Char */
			new_char();	/* Used Char1 */
			return;
			}
		break;
		}

	say_cardinal(value);

	/* Recognize decimal points */
	if (Char == '.' && isdigit(Char1))
		{
		  //		outstring("pOYnt ");
		  outnum(poynt);
		for (new_char() ; isdigit(Char) ; new_char())
			{
			say_ascii(Char);
			}
		}

	/* Spell out trailing abbreviations */
	if (isalpha(Char))
		{
		while (isalpha(Char))
			{
			say_ascii(Char);
			new_char();
			}
		}

	return;
	}

void abbrev(char buff[]);

void have_letter()
	{
	char buff[MAX_LENGTH];
	int count;

	count = 0;
	buff[count++] = ' ';	/* Required initial blank */

	buff[count++] = makeupper(Char);

	for (new_char() ; isalpha(Char) || Char == '\'' ; new_char())
		{
		buff[count++] = makeupper(Char);
		if (count > MAX_LENGTH-2)
			{
			buff[count++] = ' ';
			buff[count++] = '\0';
			xlate_word(buff);
			count = 1;
			}
		}

	buff[count++] = ' ';	/* Required terminating blank */
	buff[count++] = '\0';

	/* Check for AAANNN type abbreviations */
	if (isdigit(Char))
		{
		spell_word(buff);
		return;
		}
	else
	if (strlen(buff) == 3)	 /* one character, two spaces */
		say_ascii(buff[1]);
	else
	if (Char == '.')		/* Possible abbreviation */
		abbrev(buff);
	else
		xlate_word(buff);

	if (Char == '-' && isalpha(Char1))
		new_char();	/* Skip hyphens */

	}

/* Handle abbreviations.  Text in buff was followed by '.' */
void abbrev(char buff[])
	{
	if (strcmp(buff, " DR ") == 0)
		{
		xlate_word(" DOCTOR ");
		new_char();
		}
	else
	if (strcmp(buff, " MR ") == 0)
		{
		xlate_word(" MISTER ");
		new_char();
		}
	else
	if (strcmp(buff, " MRS ") == 0)
		{
		xlate_word(" MISSUS ");
		new_char();
		}
	else
	if (strcmp(buff, " PHD ") == 0)
		{
		spell_word(" PHD ");
		new_char();
		}
	else
		xlate_word(buff);
	}

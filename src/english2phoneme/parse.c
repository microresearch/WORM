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
unsigned char output_array[MAX_LENGTH];
unsigned char  output_count = 0;

typedef struct{
  unsigned char length;
  unsigned char mmm[5];
} vottts;

int text2speechforvotrax(int input_len, unsigned char *input, unsigned char *output);

static const vottts ourvot[] __attribute__ ((section (".flash")))=  {{1, {0x05}}, //IY
				{1, {0x07}},//IH
				{3,{0x08,0x05,0x03}},//EY
				{1, {0x0A}},//eH
				{1, {0x0C}},//AE
				{1, {0x0E}},  //aa
				{3, {0x12,0x11,0x11}},//ao
				{2, {0x11,0x16}}, //ow
				{1, {0x15}}, //uh
				{3, {0x14,0x16,0x16}}, //uw
				{1, {0x1c}}, //er
				{2, {0x0C,0x23}},//ax
				{1, {0x1B}},//ah
				{4, {0x0F,0x0D,0x0B,0x03}},//ay
				{4, {0x0F,0x10,0x11,0x16}},//aw
				{5, {0x11,0x19,0x0F,0x07,0x06}}, //oy
				{1, {0x27 }}, //p
				{1, {0x24}}, //b
				{1, {0x28}}, //t
				{1, {0x25}}, //d
				{2, {0x29,0x2c}},//k
				{1, {0x26}}, // g
				{1, {0x34}}, //f
				{1, {0x33}}, //v
				{1, {0x35}}, //th
				{1, {0x36}}, //dh
				{1, {0x30}}, //s
				{1, {0x2f}}, //z
				{1, {0x32}}, //sh
				{1, {0x2f}}, //zh
				{1, {0x2c}}, //h
				{1, {0x37}},//m
				{1, {0x38}}, //n
				{1, {0x39}}, //ng
				{1, {0x20}}, //l
				{1, {0x23}}, //w
				{1, {0x04}}, //y
				{1, {0x1d}}, //r
				{3, {0x28,0x2D,0x32}},//ch 
				{2, {0x25,0x31}},//j
				{1, {0x23}}, //wh
				{1, {0}}, //pause
				{1, {0}}//''
  };


static const unsigned char poynt[5]  __attribute__ ((section (".flash"))) ={16, 15, 32, 18, 41};

// remap this index 0->42 to the 256 phonemes which are

/*NRLIPAtoSPO256 = { 'AA':'AA', 'AE':'AE', 'AH':'AX AX', 'AO':'AO', 'AW':'AW',  'AX':'AX',
                   'AY':'AY', 'b':'BB1', 'CH':'CH',  'd':'DD1', 'DH':'DH1', 'EH':'EH',
                   'ER':'ER1','EY':'EY', 'f':'FF',   'g':'GG2', 'h':'HH1',  'IH':'IH',
                   'IY':'IY', 'j':'JH',  'k':'KK1',  'l':'LL',  'm':'MM',   'n':'NN1',
                   'NG':'NG', 'OW':'OW', 'OY':'OY',  'p':'PP',  'r':'RR1',  's':'SS',
                   'SH':'SH', 't':'TT1', 'TH':'TH',  'UH':'UH',  'UW':'UW2','v':'VV',
                   'w':'WW', 'WH':'WH', 'y':'YY1', 'z':'ZZ', 'ZH':'ZH', 'PAUSE':'PA4' };*/

/* 

// but what are numbers for sp0256

    _allophones = { 'PA1':0, 'PA2':1, 'PA3':2, 'PA4':3, 'PA5':4, 'OY':5, 'AY':6, 'EH':7, 
             'KK3':8, 'PP':9, 'JH':10, 'NN1':11, 'IH':12, 'TT2':13, 'RR1':14, 'AX':15, 
             'MM':16, 'TT1':17, 'DH1':18, 'IY':19, 'EY':20, 'DD1':21, 'UW1':22, 'AO':23, 
             'AA':24, 'YY2':25, 'AE':26, 'HH1':27, 'BB1':28, 'TH':29, 'UH':30, 'UW2':31, 
             'AW':32, 'DD2':33, 'GG3':34, 'VV':35, 'GG1':36, 'SH':37, 'ZH':38, 'RR2':39, 
             'FF':40, 'KK2':41, 'KK1':42, 'ZZ':43, 'NG':44, 'LL':45, 'WW':46, 'XR':47, 
             'WH':48, 'YY1':49, 'CH':50, 'ER1':51, 'ER2':52, 'OW':53, 'DH2':54, 'SS':55, 
             'NN2':56, 'HH2':57, 'OR':58, 'AR':59, 'YR':60, 'GG2':61, 'EL':62, 'BB2':63 };*/

/* SAM:

const char* phoneme_list[54]={0"IY", 1"IH", 2"EH", 3"AE", 4"AA", 5"AH", 6"AO", 7"OH", 8"UH", 9"UX", 10"ER", 11"AX", 12"IX", 13"EY", 14"AY", 15"OY", 16"AW", 17"OW", 18"UW", 19"R", 20"L", 21"W", 22"WH", 23"Y", 24"M", 25"N", 26"NX", 27"B", 28"D", 29"G", 30"J", 31"Z", 32"ZH", 33"V", 34"DH", 35"S", 36"SH", 37"F", 38"TH", 39"P", 40"T", 41"K", 42"CH", 43"/H", 44"YX", 45"WX", 46"RX", 47"LX", 48"/X", 49"DX", 50"UL", 51"UM", 52"UN", 53"Q"}; 

most *X are all special cases?

 */


//NRL: IY, 0, IH, 1, EY, 2, EH, 3, AE, 4, AA, 5, AO, 6, OW, 7, UH, 8, UW, 9, ER, 10, AX, 11, AH, 12, AY, 13, AW, 14, OY, 15, p, 16, b, 17, t, 18, d, 19, k, 20, g, 21, f, 22, v, 23, TH, 24, DH, 25, s, 26, z, 27, SH, 28, ZH, 29, h, 30, m, 31, n, 32, NG, 33, l, 34, w, 35, y, 36, r, 37, CH, 38, j, 39, WH, 40, PAUSE, 41, "", 42

//NRL: IY, IH, EY, EH, AE, AA, AO, OW, UH, UW, ER, AX, AH, AY, AW, OY, p, b, t, d, k, g, f, v, TH, DH, s, z, SH, ZH, h, m, n, NG, l, w, y, r, CH, j, WH, PAUSE, ""

//KLATT: END 0,  Q 1,  P 2,  PY 3,  PZ 4,  T 5,  TY 6,  TZ 7,  K 8,  KY 9,  KZ 10,  B 11,  BY 12,  BZ 13,  D 14,  DY 15,  DZ 16,  G 17,  GY 18,  GZ 19,  M 20,  N 21,  NG 22,  F 23,  TH 24,  S 25,  SH 26,  X 27,  H 28,  V 29,  QQ 30,  DH 31,  DI 32,  Z 33,  ZZ 34,  ZH 35,  CH 36,  CI 37,  J 38,  JY 39,  L 40,  LL 41,  RX 42,  R 43,  W 44,  Y 45,  I 46,  E 47,  AA 48,  U 49,  O 50,  OO 51,  A 52,  EE 53,  ER 54,  AR 55,  AW 56,  UU 57,  AI 58,  IE 59,  OI 60,  OU 61,  OV 62,  OA 63,  IA 64,  IB 65,  AIR 66,  OOR 67,  OR 68

static const char remap256[43]  __attribute__ ((section (".flash"))) ={19, 12, 20, 7, 26, 24, 23, 53, 30, 31, 51, 15, 15, 6, 32, 5, 9, 28, 17, 21, 42, 61, 40, 35, 29, 18, 55, 43, 37, 38, 27, 16, 11, 44, 45, 46, 49, 14, 50, 10, 48, 3, 3}; // what about silence and case 12=AX AX? DONE but silence?

static const char remapsam[43]  __attribute__ ((section (".flash"))) ={0, 1, 13, 2, 3, 4, 6, 17, 8, 18, 10, 11, 5, 14, 16, 15, 39, 27, 40, 28, 41, 29, 37, 33, 38, 34, 35, 31, 36, 32, 43, 24, 25, 26, 20, 21, 23, 19, 42, 30, 22, 54, 54};

//static const char remapklatt[43]  __attribute__ ((section (".flash"))) ={IY, IH, EY, EH, AE, 48, AO, OW, UH, UW, 54, AX, AH, AY, 56, OY, 2, 11, 5, 14, 8, 17, 23, 29 , 24, 31, 25, 33, 26, 35, 28, 20, 21, 22, 40, 44, 45, 43, 36, 38, WH, PAUSE, 0};

// 0 = ʃ 1 = ʍ 2 = a 3 = ɐ 4 = ɒ 5 = ɔ 6 = ɜ 7 = b 8 = d 9 = f 10 = ɪ 11 = t(3 12 = l 13 = n 14 = p 15 = t 16 = v 17 = z 18 = ɾ 19 = j 20 = ʊ 21 = ʌ 22 = ʒ 23 = ɔj 24 = ʔ 25 = d͡ʒ 26 = θ 27 = ɑw 28 = I 29 = ŋ 30 = t͡ʃ 31 = ɑ 32 = ə 33 = ɛ 34 = ɑj 35 = ɡ 36 = e 37 = g 38 = æ 39 = i 40 = k 41 = m 42 = o 43 = ð 44 = s 45 = u 46 = w 47 = ɹ //????

//static const char remapnvp[43]  __attribute__ ((section (".flash"))) ={IY, IH, EY, EH, AE, AA, AO, OW, UH, UW, ER, AX, AH, AY, AW, OY, p, b, t, d, k, g, f, v, TH, DH, s, z, SH, ZH, h, m, n, NG, l, w, y, r, CH, j, WH, PAUSE, ""};

//static const char remaptubes[43]  __attribute__ ((section (".flash"))) ={IY, IH, EY, EH, AE, AA, AO, OW, UH, UW, ER, AX, AH, AY, AW, OY, p, b, t, d, k, g, f, v, TH, DH, s, z, SH, ZH, h, m, n, NG, l, w, y, r, CH, j, WH, PAUSE, ""}; // see diphones.degas

// klatt/nvp

// tubes

// for VOTRAX

typedef struct{
  unsigned char howmany;
  unsigned char its[3];
} votmap;


static votmap remapvotrax[] = {{1, 0x01,0x02}}; //example


void xlate_word(char word[]);
void spell_word(char word[]);
void say_ascii(int character);
void xlate_file();
int text2speech(int input_len, char *input, char *output);
int text2speechfor256(int input_len, unsigned char *input, unsigned char *output);
int text2speechforSAM(int input_len, char *input, char *output);
void have_letter();
void have_special();
void say_cardinal(long int value);
void say_ordinal(long int value);
void outnum(const char* ooo);
void have_number();

#ifdef LAP
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

  //char input[input_size + space_count];
  
  //initialize input array to given arguments
  int index = 0;

  //  static char TTSinarray[64]={"testing "};
    static char TTSinarray[64];
    static unsigned char TTSoutarray[128];

    strcpy(TTSinarray,argv[1]);

    //    TTSinarray[input_size] = EOF; // place in text2speech

          printf("%s %d\n",TTSinarray, input_size);

  //  u8 TTSlength= text2speechfor256(9,TTSinarray,TTSoutarray); // 7 is length how? or is fixed?


  //  printf("INEDX %d in %c\n\n", index, input[4]);

  
  //transform text to integer code phonemes
	  //  int output_count = text2speechfor256(input_size,TTSinarray,TTSoutarray);
	    int output_count = text2speechforvotrax(input_size,TTSinarray,TTSoutarray);
  for(int i = 0; i < output_count; i++){
    //    for(int j = 0; j < strlen(output[i]); j++){
             printf("%d, ", TTSoutarray[i]);
	     //	      }
	     //      printf("\n");
  }
  //  return output_count;
  }
#endif

/*
** Transforms text to integer code phonemes.
*/
int text2speech(int input_len, char *input, char *output){
  input_array = input;
  input_length = input_len;
  input_count = 0; output_count=0;
  input[input_len] = EOF;

  xlate_file();
  for (char i=0;i<output_count;i++){
    output[i]=output_array[i]; 
  }
  return output_count;
}

int text2speechfor256(int input_len, unsigned char *input, unsigned char *output){ // TODO: this is our model
  input_array = input;
  input_length = input_len;
  input_count = 0; output_count=0;
  input[input_len] = EOF;

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
  output[output_count-1]=255; 
  return output_count; //check TODO!
}

int text2speechforvotrax(int input_len, unsigned char *input, unsigned char *output){
  input_array = input;
  input_length = input_len;
  input_count = 0; output_count=0;
  input[input_len] = EOF;

  xlate_file();
  char countme=0;
  for (char i=0;i<output_count;i++){
    for (char ii=0;ii<ourvot[output_array[i]].length;ii++){
    output[countme]=ourvot[output_array[i]].mmm[ii];
    countme++;
	}
	//       output[i]=remap256[rand()%43];
  }
  return countme; 
}

int text2speechforSAM(int input_len, char *input, char *output){
  input_array = input;
  input_length = input_len;
  input_count = 0; output_count=0;
  input[input_len] = EOF;

  xlate_file();
  //      output_count=10;
  for (char i=0;i<output_count;i++){
        output[i]=remapsam[output_array[i]];
	}
  return output_count-1;
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

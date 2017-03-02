#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "stdlib.h"
#include "TTS.h"
#define MAX_LENGTH 1024

//static FILE *In_file;
//static FILE *Out_file;

static int Char, Char1, Char2, Char3;
static int input_count;
static int input_length;
static char *input_array;
unsigned char output_array[MAX_LENGTH];
int output_count = 0;

typedef struct{
  unsigned char length;
  unsigned char mmm[5];
} vottts;

static const vottts ourvot[] __attribute__ ((section (".flash")))=  {{1, {0x21}}, //IY 0 // start to convert to sc01
				{1, {0x27}},//IH1
				{3,{0x20,0x21,0x29}},//EY2= A, AY. Y
				{1, {0x00}},//eH3
				{1, {0x2f}},//AE4
				{1, {0x24}},  //aa5 ///
				{3, {0x35,0x26,0x26}},//ao6=OU, O, O
				{2, {0x26,0x28}}, //ow7=O, U
				{1, {0x28}}, //uh8
				{3, {0x28,0x28}}, //uw9=IU, U, U
				{1, {0x3a}}, //er10
				{2, {0x2E,0x2d}},//ax11=AE, W
				{1, {0x33}},//ah
				{4, {0x15,0x2f,0x02,0x29}},//ay=ah1, ae1, eh1, Y
				{4, {0x24,0x3d,0x26,0x28}},//aw=AH1, AW, O, U
				{5, {0x26,0x32,0x15,0x27,0x29}}, //oy=o, uh1, ah1, I, IE
				{1, {0x25}}, //p
				{1, {0x0e}}, //b
				{1, {0x2a}}, //t
				{1, {0x1e}}, //d
				{2, {0x19,0x1b}},//k=K, HF
				{1, {0x1c}}, // g
				{1, {0x1d}}, //f
				{1, {0x0f}}, //v
				{1, {0x38}}, //th
				{1, {0x39}}, //dh
				{1, {0x1F}}, //s
				{1, {0x12}}, //z
				{1, {0x11}}, //sh
				{1, {0x07}}, //zh
				{1, {0x1b}}, //h
				{1, {0x0c}},//m
				{1, {0x0d}}, //n
				{1, {0x14}}, //ng
				{1, {0x18}}, //l
				{1, {0x2d}}, //w
				{1, {0x29}}, //y
				{1, {0x2b}}, //r
				{2, {0x2A,0x11}},//ch=T, HFC?, SCH
				{2, {0x1e,0x1a}},//j=D, J
				{1, {0x2d}}, //wh
				{1, {0x3e}}, //pause
				{1, {0x3f}}//'' STOP???
  };


static const unsigned char poynt[5]  __attribute__ ((section (".flash"))) ={16, 15, 32, 18, 41};


//const char* NRL_list[54]={"IY", "IH", "EY", "EH", "AE", "AA", "AO", "OW", "UH", "UW", "ER", "AX", "AH", "AY", "AW", "OY", "p", "b", "t", "d", "k", "g", "f", "v", "TH", "DH", "s", "z", "SH", "ZH", "h", "m", "n", "NG", "l", "w", "y", "r", "CH", "j", "WH", "PAUSE", "END"};

//KLATT: END 0",  Q 1,  P 2,  PY 3,  PZ 4,  T 5,  TY 6,  TZ 7,  K 8,  KY 9,  KZ 10,  B 11,  BY 12,  BZ 13,  D 14,  DY 15,  DZ 16,  G 17,  GY 18,  GZ 19,  M 20,  N 21,  NG 22,  F 23,  TH 24,  S 25,  SH 26,  X 27,  H 28,  V 29,  QQ 30,  DH 31,  DI 32,  Z 33,  ZZ 34,  ZH 35,  CH 36,  CI 37,  J 38,  JY 39,  L 40,  LL 41,  RX 42,  R 43,  W 44,  Y 45,  I 46,  E 47,  AA 48,  U 49,  O 50,  OO 51,  A 52,  EE 53,  ER 54,  AR 55,  AW 56,  UU 57,  AI 58,  IE 59,  OI 60,  OU 61,  OV 62,  OA 63,  IA 64,  IB 65,  AIR 66,  OOR 67,  OR 68

static const unsigned char remap256[43]  __attribute__ ((section (".flash"))) ={19, 12, 20, 7, 26, 24, 23, 53, 30, 31, 51, 15, 15, 6, 32, 5, 9, 28, 17, 21, 42, 61, 40, 35, 29, 18, 55, 43, 37, 38, 27, 16, 11, 44, 45, 46, 49, 14, 50, 10, 48, 3, 3}; // what about silence and case 12=AX AX? DONE but silence?

static const unsigned char remapsam[43]  __attribute__ ((section (".flash"))) ={0, 1, 13, 2, 3, 4, 6, 17, 8, 18, 10, 11, 5, 14, 16, 15, 39, 27, 40, 28, 41, 29, 37, 33, 38, 34, 35, 31, 36, 32, 43, 24, 25, 26, 20, 21, 23, 19, 42, 30, 22, 54, 54};

// TMS

//NRL: IY, IH, EY, EH, AE, AA, AO, OW, UH, UW, ER, AX, AH, AY, AW, OY, p, b, t, d, k, g, f, v, TH, DH, s, z, SH, ZH, h, m, n, NG, l, w, y, r, CH, j, WH, PAUSE, ""

static const unsigned char remaptms[43]  __attribute__ ((section (".flash"))) ={52, 59, 57, 33, 25, 2, 30, 14, 70, 71, 22, 1, 68, 27, 29, 38, 107, 85, 110, 87, 102, 89, 114, 96, 95, 94, 119, 98, 122, 100, 116, 75, 77, 79, 72, 82, 84, 81, 113, 92, 83, 125, 126};
  
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


//static votmap remapvotrax[] = {{1, 0x01,0x02}}; //example


void xlate_word(unsigned char word[]);
void spell_word(unsigned char word[]);
void say_ascii(int character);
void xlate_file();
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

   FILE *fp = fopen(argv[1], "r");
   //   fseek(fp,0, SEEK_END);
   // read in and how long is it?
   //   int lengthy=ftell(fp);
   //   fseek(fp,0, SEEK_SET);
   unsigned char xxx;
   // malloc and reverse into buffer
   //   unsigned char *xxx=malloc(lengthy+1);
   //   fread(xxx,lengthy,1,fp);
   //   fclose(fp);

	  
  //allocate space for the output
	  //	  char output[MAX_LENGTH];
	  //	  char output[MAX_LENGTH*MAX_PHONEME_LENGTH];

	  //	  for(int i = 0; i < MAX_LENGTH; i++){
	  //    output_array[i] = output[i];
	  //    }
  
  //allocate space for the input
  int input_size = 0;
  int space_count = 0;

  //char input[input_size + space_count];
  
  //initialize input array to given arguments
  int index = 0;

  //  static char TTSinarray[64]={"testing "};
  static char TTSinarray[640], buffer[640];
    static unsigned char TTSoutarray[1280];
    char flag=1;
    while(flag==1){
    int count=0;
    xxx=0;
    while (xxx!=' ' && xxx!='\n'){
    int xx=fread(&xxx,1,1,fp);
    //    printf("%c",xxx);
      buffer[count++]=xxx;
      if (xx!=1) { flag=0; break;}
    }
    //    fread(&xxx,1,1,fp);
    
    strcpy(TTSinarray,buffer);

    //        TTSinarray[count] = 0; // place in text2speech

    //         printf("%s %d\n",TTSinarray, input_size);

  //  u8 TTSlength= text2speechfor256(9,TTSinarray,TTSoutarray); // 7 is length how? or is fixed?


  //  printf("INEDX %d in %c\n\n", index, input[4]);

  
  //transform text to integer code phonemes
      int output_count = text2speech(count,TTSinarray,TTSoutarray);
    //    int output_count = text2speechforTMS(input_size,TTSinarray,TTSoutarray);
      //      printf("%s outcount: %d\n",TTSinarray, output_count);
  for(int i = 0; i < output_count; i++){
    //    for(int j = 0; j < strlen(output[i]); j++){
             printf("%d, ", TTSoutarray[i]);
	     //	      }
	     //      printf("\n");
  }
  //  return output_count;
  }
	}
#endif

/*
** Transforms text to integer code phonemes.
*/
unsigned char text2speech(unsigned char input_len, unsigned char *input, unsigned char *output){
  input_array = input;
  input_length = input_len;
  input_count = 0; output_count=0;
  input[input_len] = EOF;
  xlate_file();
  //  printf("xxxxx %d\n", output_count);
  for (unsigned char i=0;i<output_count;i++){
    output[i]=output_array[i]; 
  }
  return output_count;
}

unsigned char text2speechfor256(unsigned char  input_len, signed char *input, signed char *output){ // TODO: this is our model
  input_array = input;
  input_length = input_len;
  input_count = 0; output_count=0;
  input[input_len] = EOF;
      xlate_file();
  //        output_count=16;
   if (output_count>=255) output_count=254;
  for (unsigned char i=0;i<output_count;i++){
              output[i]=remap256[output_array[i]];
	if (output_array[i]==12){
	  output_array[++i]=12;
	  output_count++;
	  if (output_count>=255) output_count=254;
 	  }
    //    output[i]=remap256[rand()%43];
    //    output[i]=remap256[input[i]-97];
  }
  //  output[output_count-1]=255; 
  return output_count-1; //check TODO!
}

unsigned char text2speechforvotrax(unsigned char  input_len, unsigned char *input, unsigned char *output){
  input_array = input;
  input_length = input_len;
  input_count = 0; output_count=0;
  input[input_len] = EOF;

  xlate_file();
  if (output_count>=255) output_count=254;

  unsigned char countme=0;
  //      printf("Output: %d\n", output_count);
  for (unsigned char i=0;i<output_count;i++){
    //  printf("LEN: %d\n", ourvot[output_array[i]].length);
    for (unsigned char ii=0;ii<ourvot[output_array[i]].length;ii++){
    output[countme]=ourvot[output_array[i]].mmm[ii];
    //    printf("%s ", NRL_list[output_array[i]]);
    countme++;
    if (countme>254) countme=254;
	}
  }
  return countme; 
}

unsigned char text2speechforSAM(unsigned char  input_len, unsigned char *input, unsigned char *output){
  input_array = input;
  input_length = input_len;
  input_count = 0; output_count=0;
  input[input_len] = EOF;

  xlate_file();
    if (output_count>255) output_count=254;

  //      output_count=10;
  for (unsigned char i=0;i<output_count;i++){
        output[i]=remapsam[output_array[i]];
	}
  return output_count-1;
}

unsigned char text2speechforTMS(unsigned char input_len, unsigned char *input, unsigned char  *output){
  input_array = input;
  input_length = input_len;
  input_count = 0; output_count=0;
  input[input_len] = EOF;

  xlate_file();
  if (output_count>255) output_count=254;

  //      output_count=10;
    //      printf("OC%d\n",output_count);

  for (unsigned char i=0;i<output_count;i++){
        output[i]=remaptms[output_array[i]];
	}
  //  output[output_count-1]=255; 
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

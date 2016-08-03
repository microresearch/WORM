/*
**	English to Phoneme rules.
**
**	Derived from: 
**
**	     AUTOMATIC TRANSLATION OF ENGLISH TEXT TO PHONETICS
**	            BY MEANS OF LETTER-TO-SOUND RULES
**
**			NRL Report 7948
**
**		      January 21st, 1976
**	    Naval Research Laboratory, Washington, D.C.
**
**
**	Published by the National Technical Information Service as
**	document "AD/A021 929".
**
**
**
**	The Phoneme codes:
**
**		IY	bEEt		IH	bIt
**		EY	gAte		EH	gEt
**		AE	fAt		AA	fAther
**		AO	lAWn		OW	lOne
**		UH	fUll		UW	fOOl
**		ER	mURdER		AX	About
**		AH	bUt		AY	hIde
**		AW	hOW		OY	tOY
**	
**		p	Pack		b	Back
**		t	Time		d	Dime
**		k	Coat		g	Goat
**		f	Fault		v	Vault
**		TH	eTHer		DH	eiTHer
**		s	Sue		z	Zoo
**		SH	leaSH		ZH	leiSure
**		HH	How		m	suM
**		n	suN		NG	suNG
**		l	Laugh		w	Wear
**		y	Young		r	Rate
**		CH	CHar		j	Jar
**		WH	WHere
**
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
**			(Found in right context only)
**		+	One of E, I or Y (a "front" vowel)
**
*/


/* Context definitions */
static char Anything[] = "";	/* No context requirement */
static char Nothing[] = " ";	/* Context is beginning or end of word */

/* Phoneme definitions */
static char Pause[] = " ";	/* Short silence */
static char Silent[] = "";	/* No phonemes */

#define LEFT_PART	0
#define MATCH_PART	1
#define RIGHT_PART	2
#define OUT_PART	3

//typedef char *Rule[4];	/* Rule is an array of 4 character pointers */

typedef struct{
  char *rule[3];
  signed char[8];
} Rule;

/*0 = Punctuation */
/*
**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
*/

/*
** NEW RULES (space delimited) BELOW
*/

static Rule S_rules[] = {{Anything, "SH",Anything, "28 "},
{"#", "SION",Anything, "29 11 32 "},
{Anything, "SOME",Anything, "26 12 31 "},
{"#", "SUR","#", "29 10 "},
{Anything, "SUR","#", "28 10 "},
{"#", "SU","#", "29 9 "},
{"#", "SSU","#", "28 9 "},
{"#", "SED",Nothing, "27 19 "},
{"#", "S","#", "27 "},
{Anything, "SAID",Anything, "26 3 19 "},
{"^", "SION",Anything, "28 11 32 "},
{Anything, "S","S", ""},
{".", "S",Nothing, "27 "},
{"#:.E", "S",Nothing, "27 "},
{"#:^##", "S",Nothing, "27 "},
{"#:^#", "S",Nothing, "26 "},
{"U", "S",Nothing, "26 "},
{" :#", "S",Nothing, "27 "},
{Nothing, "SCH",Anything, "26 20 "},
{Anything, "S","C+", ""},
{"#", "SM",Anything, "27 31 "},
{"#", "SN","'", "27 11 32 "},
{Anything, "S",Anything, "26 "},
{Anything, 0, Anything, Silent}};

static Rule L_rules[] = {{Anything, "LO","C#", "34 7 "},
{"L", "L",Anything, ""},
{"#:^", "L","%", "11 34 "},
{Anything, "LEAD",Anything, "34 0 19 "},
{Anything, "L",Anything, "34 "},
{Anything, 0, Anything, Silent}};

static Rule U_rules[] = {{Nothing, "UN","I", "36 9 32 "},
{Nothing, "UN",Anything, "12 32 "},
{Nothing, "UPON",Anything, "11 16 6 32 "},
{"T", "UR","#", "8 37 "},
{"S", "UR","#", "8 37 "},
{"R", "UR","#", "8 37 "},
{"D", "UR","#", "8 37 "},
{"L", "UR","#", "8 37 "},
{"Z", "UR","#", "8 37 "},
{"N", "UR","#", "8 37 "},
{"J", "UR","#", "8 37 "},
{"TH", "UR","#", "8 37 "},
{"CH", "UR","#", "8 37 "},
{"SH", "UR","#", "8 37 "},
{Anything, "UR","#", "36 8 37 "},
{Anything, "UR",Anything, "10 "},
{Anything, "U","^ ", "12 "},
{Anything, "U","^^", "12 "},
{Anything, "UY",Anything, "13 "},
{" G", "U","#", ""},
{"G", "U","%", ""},
{"G", "U","#", "35 "},
{"#N", "U",Anything, "36 9 "},
{"T", "U",Anything, "9 "},
{"S", "U",Anything, "9 "},
{"R", "U",Anything, "9 "},
{"D", "U",Anything, "9 "},
{"L", "U",Anything, "9 "},
{"Z", "U",Anything, "9 "},
{"N", "U",Anything, "9 "},
{"J", "U",Anything, "9 "},
{"TH", "U",Anything, "9 "},
{"CH", "U",Anything, "9 "},
{"SH", "U",Anything, "9 "},
{Anything, "U",Anything, "36 9 "},
{Anything, 0, Anything, Silent}};

static Rule B_rules[] = {{Nothing, "BE","^#", "17 1 "},
{Anything, "BEING",Anything, "17 0 1 33 "},
{Nothing, "BOTH",Nothing, "17 7 24 "},
{Nothing, "BUS","#", "17 1 27 "},
{Anything, "BUIL",Anything, "17 1 34 "},
{Anything, "B",Anything, "17 "},
{Anything, 0, Anything, Silent}};

static Rule punct_rules[] = {{Anything, " ",Anything, "41 "},
{Anything, "--",Anything, "41 "},
{Anything, "-",Anything, ""},
{".", "'S",Anything, "27 "},
{"#:.E", "'S",Anything, "27 "},
{"#", "'S",Anything, "27 "},
{Anything, "'",Anything, ""},
{Anything, ",",Anything, "41 "},
{Anything, ".",Anything, "41 "},
{Anything, "?",Anything, "41 "},
{Anything, "!",Anything, "41 "},
{Anything, "@",Anything, "4 18 "},
{Anything, 0, Anything, Silent}};

static Rule W_rules[] = {{Nothing, "WERE",Anything, "35 10 "},
{Anything, "WA","S", "35 5 "},
{Anything, "WA","T", "35 5 "},
{Anything, "WHERE",Anything, "40 3 37 "},
{Anything, "WHAT",Anything, "40 5 18 "},
{Anything, "WHOL",Anything, "30 7 34 "},
{Anything, "WHO",Anything, "30 9 "},
{Anything, "WH",Anything, "40 "},
{Anything, "WAR",Anything, "35 6 37 "},
{Anything, "WOR","^", "35 10 "},
{Anything, "WR",Anything, "37 "},
{Anything, "W",Anything, "35 "},
{Anything, 0, Anything, Silent}};

static Rule N_rules[] = {{"E", "NG","+", "32 39 "},
{Anything, "NG","R", "33 21 "},
{Anything, "NG","#", "33 21 "},
{Anything, "NGL","%", "33 21 11 34 "},
{Anything, "NG",Anything, "33 "},
{Anything, "NK",Anything, "33 20 "},
{Nothing, "NOW",Nothing, "32 14 "},
{Anything, "N",Anything, "32 "},
{Anything, 0, Anything, Silent}};

static Rule E_rules[] = {{"#:", "E",Nothing, ""},
{"':^", "E",Nothing, ""},
{" :", "E",Nothing, "0 "},
{"#", "ED",Nothing, "19 "},
{"#:", "E","D ", ""},
{Anything, "EV","ER", "3 23 "},
{Anything, "E","^%", "0 "},
{Anything, "ERI","#", "0 37 0 "},
{Anything, "ERI",Anything, "3 37 1 "},
{"#:", "ER","#", "10 "},
{Anything, "ER","#", "3 37 "},
{Anything, "ER",Anything, "10 "},
{Nothing, "EVEN",Anything, "0 23 3 32 "},
{"#:", "E","W", ""},
{"T", "EW",Anything, "9 "},
{"S", "EW",Anything, "9 "},
{"R", "EW",Anything, "9 "},
{"D", "EW",Anything, "9 "},
{"L", "EW",Anything, "9 "},
{"Z", "EW",Anything, "9 "},
{"N", "EW",Anything, "9 "},
{"J", "EW",Anything, "9 "},
{"TH", "EW",Anything, "9 "},
{"CH", "EW",Anything, "9 "},
{"SH", "EW",Anything, "9 "},
{Anything, "EW",Anything, "36 9 "},
{Anything, "E","O", "0 "},
{"#:S", "ES",Nothing, "1 27 "},
{"#:C", "ES",Nothing, "1 27 "},
{"#:G", "ES",Nothing, "1 27 "},
{"#:Z", "ES",Nothing, "1 27 "},
{"#:X", "ES",Nothing, "1 27 "},
{"#:J", "ES",Nothing, "1 27 "},
{"#:CH", "ES",Nothing, "1 27 "},
{"#:SH", "ES",Nothing, "1 27 "},
{"#:", "E","S ", ""},
{"#:", "ELY",Nothing, "34 0 "},
{"#:", "EMENT",Anything, "31 3 32 18 "},
{Anything, "EFUL",Anything, "22 8 34 "},
{Anything, "EE",Anything, "0 "},
{Anything, "EARN",Anything, "10 32 "},
{Nothing, "EAR","^", "10 "},
{Anything, "EAD",Anything, "3 19 "},
{"#:", "EA",Nothing, "0 11 "},
{Anything, "EA","SU", "3 "},
{Anything, "EA",Anything, "0 "},
{Anything, "EIGH",Anything, "2 "},
{Anything, "EI",Anything, "0 "},
{Nothing, "EYE",Anything, "13 "},
{Anything, "EY",Anything, "0 "},
{Anything, "EU",Anything, "36 9 "},
{Anything, "E",Anything, "3 "},
{Anything, 0, Anything, Silent}};

static Rule T_rules[] = {{Nothing, "THE",Nothing, "25 11 "},
{Anything, "TO",Nothing, "18 9 "},
{Anything, "THAT",Nothing, "25 4 18 "},
{Nothing, "THIS",Nothing, "25 1 26 "},
{Nothing, "THEY",Anything, "25 2 "},
{Nothing, "THERE",Anything, "25 3 37 "},
{Anything, "THER",Anything, "25 10 "},
{Anything, "THEIR",Anything, "25 3 37 "},
{Nothing, "THAN",Nothing, "25 4 32 "},
{Nothing, "THEM",Nothing, "25 3 31 "},
{Anything, "THESE",Nothing, "25 0 27 "},
{Nothing, "THEN",Anything, "25 3 32 "},
{Anything, "THROUGH",Anything, "24 37 9 "},
{Anything, "THOSE",Anything, "25 7 27 "},
{Anything, "THOUGH",Nothing, "25 7 "},
{Nothing, "THUS",Anything, "25 12 26 "},
{Anything, "TH",Anything, "24 "},
{"#:", "TED",Nothing, "18 1 19 "},
{"S", "TI","#N", "38 "},
{Anything, "TI","O", "28 "},
{Anything, "TI","A", "28 "},
{Anything, "TIEN",Anything, "28 11 32 "},
{Anything, "TUR","#", "38 10 "},
{Anything, "TU","A", "38 9 "},
{Nothing, "TWO",Anything, "18 9 "},
{Anything, "T",Anything, "18 "},
{Anything, 0, Anything, Silent}};

static Rule J_rules[] = {{Anything, "J",Anything, "39 "},
{Anything, 0, Anything, Silent}};

static Rule Y_rules[] = {{Anything, "YOUNG",Anything, "36 12 33 "},
{Nothing, "YOU",Anything, "36 9 "},
{Nothing, "YES",Anything, "36 3 26 "},
{Nothing, "Y",Anything, "36 "},
{"#:^", "Y",Nothing, "0 "},
{"#:^", "Y","I", "0 "},
{" :", "Y",Nothing, "13 "},
{" :", "Y","#", "13 "},
{" :", "Y","^+:#", "1 "},
{" :", "Y","^#", "13 "},
{Anything, "Y",Anything, "1 "},
{Anything, 0, Anything, Silent}};

static Rule I_rules[] = {{Nothing, "IN",Anything, "1 32 "},
{Nothing, "I",Nothing, "13 "},
{Anything, "IN","D", "13 32 "},
{Anything, "IER",Anything, "0 10 "},
{"#:R", "IED",Anything, "0 19 "},
{Anything, "IED",Nothing, "13 19 "},
{Anything, "IEN",Anything, "0 3 32 "},
{Anything, "IE","T", "13 3 "},
{" :", "I","%", "13 "},
{Anything, "I","%", "0 "},
{Anything, "IE",Anything, "0 "},
{Anything, "I","^+:#", "1 "},
{Anything, "IR","#", "13 37 "},
{Anything, "IZ","%", "13 27 "},
{Anything, "IS","%", "13 27 "},
{Anything, "I","D%", "13 "},
{"+^", "I","^+", "1 "},
{Anything, "I","T%", "13 "},
{"#:^", "I","^+", "1 "},
{Anything, "I","^+", "13 "},
{Anything, "IR",Anything, "10 "},
{Anything, "IGH",Anything, "13 "},
{Anything, "ILD",Anything, "13 34 19 "},
{Anything, "IGN",Nothing, "13 32 "},
{Anything, "IGN","^", "13 32 "},
{Anything, "IGN","%", "13 32 "},
{Anything, "IQUE",Anything, "0 20 "},
{Anything, "I",Anything, "1 "},
{Anything, 0, Anything, Silent}};

static Rule V_rules[] = {{Anything, "VIEW",Anything, "23 36 9 "},
{Anything, "V",Anything, "23 "},
{Anything, 0, Anything, Silent}};

static Rule K_rules[] = {{Nothing, "K","N", ""},
{Anything, "K",Anything, "20 "},
{Anything, 0, Anything, Silent}};

static Rule M_rules[] = {{Anything, "MOV",Anything, "31 9 23 "},
{Anything, "M",Anything, "31 "},
{Anything, 0, Anything, Silent}};

static Rule O_rules[] = {{Anything, "OF",Nothing, "11 23 "},
{Anything, "OROUGH",Anything, "10 7 "},
{"#:", "OR",Nothing, "10 "},
{"#:", "ORS",Nothing, "10 27 "},
{Anything, "OR",Anything, "6 37 "},
{Nothing, "ONE",Anything, "35 12 32 "},
{Nothing, "OW",Anything, "7 "},
{Anything, "OW",Anything, "7 "},
{Nothing, "OVER",Anything, "7 23 10 "},
{Anything, "OV",Anything, "12 23 "},
{Anything, "O","^%", "7 "},
{Anything, "O","^EN", "7 "},
{Anything, "O","^I#", "7 "},
{Anything, "OL","D", "7 34 "},
{Anything, "OUGHT",Anything, "6 18 "},
{Anything, "OUGH",Anything, "12 22 "},
{Nothing, "OU",Anything, "14 "},
{"H", "OU","S#", "14 "},
{Anything, "OUS",Anything, "11 26 "},
{Anything, "OUR",Anything, "6 37 "},
{Anything, "OULD",Anything, "8 19 "},
{"^", "OU","^L", "12 "},
{Anything, "OUP",Anything, "9 16 "},
{Anything, "OU",Anything, "14 "},
{Anything, "OY",Anything, "15 "},
{Anything, "OING",Anything, "7 1 33 "},
{Anything, "OI",Anything, "15 "},
{Anything, "OOR",Anything, "6 37 "},
{Anything, "OOK",Anything, "8 20 "},
{Anything, "OOD",Anything, "8 19 "},
{Anything, "OO",Anything, "9 "},
{Anything, "O","E", "7 "},
{Anything, "O",Nothing, "7 "},
{Anything, "OA",Anything, "7 "},
{Nothing, "ONLY",Anything, "7 32 34 0 "},
{Nothing, "ONCE",Anything, "35 12 32 26 "},
{Anything, "ON'T",Anything, "7 32 18 "},
{"C", "O","N", "5 "},
{Anything, "O","NG", "6 "},
{" :^", "O","N", "12 "},
{"I", "ON",Anything, "11 32 "},
{"#:", "ON",Nothing, "11 32 "},
{"#^", "ON",Anything, "11 32 "},
{Anything, "O","ST ", "7 "},
{Anything, "OF","^", "6 22 "},
{Anything, "OTHER",Anything, "12 25 10 "},
{Anything, "OSS",Nothing, "6 26 "},
{"#:^", "OM",Anything, "12 31 "},
{Anything, "O",Anything, "5 "},
{Anything, 0, Anything, Silent}};

static Rule X_rules[] = {{Anything, "X",Anything, "20 26 "},
{Anything, 0, Anything, Silent}};

static Rule D_rules[] = {{"#:", "DED",Nothing, "19 1 19 "},
{".E", "D",Nothing, "19 "},
{"#:^E", "D",Nothing, "18 "},
{Nothing, "DE","^#", "19 1 "},
{Nothing, "DO",Nothing, "19 9 "},
{Nothing, "DOES",Anything, "19 12 27 "},
{Nothing, "DOING",Anything, "19 9 1 33 "},
{Nothing, "DOW",Anything, "19 14 "},
{Anything, "DU","A", "39 9 "},
{Anything, "D",Anything, "19 "},
{Anything, 0, Anything, Silent}};

static Rule H_rules[] = {{Nothing, "HAV",Anything, "30 4 23 "},
{Nothing, "HERE",Anything, "30 0 37 "},
{Nothing, "HOUR",Anything, "14 10 "},
{Anything, "HOW",Anything, "30 14 "},
{Anything, "H","#", "30 "},
{Anything, "H",Anything, ""},
{Anything, 0, Anything, Silent}};

static Rule C_rules[] = {{Nothing, "CH","^", "20 "},
{"^E", "CH",Anything, "20 "},
{Anything, "CH",Anything, "38 "},
{" S", "CI","#", "26 13 "},
{Anything, "CI","A", "28 "},
{Anything, "CI","O", "28 "},
{Anything, "CI","EN", "28 "},
{Anything, "C","+", "26 "},
{Anything, "CK",Anything, "20 "},
{Anything, "COM","%", "20 12 31 "},
{Anything, "C",Anything, "20 "},
{Anything, 0, Anything, Silent}};

static Rule A_rules[] = {{Anything, "A",Nothing, "11 "},
{Nothing, "ARE",Nothing, "5 37 "},
{Nothing, "AR","O", "11 37 "},
{Anything, "AR","#", "3 37 "},
{"^", "AS","#", "2 26 "},
{Anything, "A","WA", "11 "},
{Anything, "AW",Anything, "6 "},
{" :", "ANY",Anything, "3 32 0 "},
{Anything, "A","^+#", "2 "},
{"#:", "ALLY",Anything, "11 34 0 "},
{Nothing, "AL","#", "11 34 "},
{Anything, "AGAIN",Anything, "11 21 3 32 "},
{"#:", "AG","E", "1 39 "},
{Anything, "A","^+:#", "4 "},
{" :", "A","^+ ", "2 "},
{Anything, "A","^%", "2 "},
{Nothing, "ARR",Anything, "11 37 "},
{Anything, "ARR",Anything, "4 37 "},
{" :", "AR",Nothing, "5 37 "},
{Anything, "AR",Nothing, "10 "},
{Anything, "AR",Anything, "5 37 "},
{Anything, "AIR",Anything, "3 37 "},
{Anything, "AI",Anything, "2 "},
{Anything, "AY",Anything, "2 "},
{Anything, "AU",Anything, "6 "},
{"#:", "AL",Nothing, "11 34 "},
{"#:", "ALS",Nothing, "11 34 27 "},
{Anything, "ALK",Anything, "6 20 "},
{Anything, "AL","^", "6 34 "},
{" :", "ABLE",Anything, "2 17 11 34 "},
{Anything, "ABLE",Anything, "11 17 11 34 "},
{Anything, "ANG","+", "2 32 39 "},
{Anything, "A",Anything, "4 "},
{Anything, 0, Anything, Silent}};

static Rule R_rules[] = {{Nothing, "RE","^#", "37 0 "},
{Anything, "R",Anything, "37 "},
{Anything, 0, Anything, Silent}};

static Rule Q_rules[] = {{Anything, "QUAR",Anything, "20 35 6 37 "},
{Anything, "QU",Anything, "20 35 "},
{Anything, "Q",Anything, "20 "},
{Anything, 0, Anything, Silent}};

static Rule Z_rules[] = {{Anything, "Z",Anything, "27 "},
{Anything, 0, Anything, Silent}};

static Rule G_rules[] = {{Anything, "GIV",Anything, "21 1 23 "},
{Nothing, "G","I^", "21 "},
{Anything, "GE","T", "21 3 "},
{"SU", "GGES",Anything, "21 39 3 26 "},
{Anything, "GG",Anything, "21 "},
{" B#", "G",Anything, "21 "},
{Anything, "G","+", "39 "},
{Anything, "GREAT",Anything, "21 37 2 18 "},
{"#", "GH",Anything, ""},
{Anything, "G",Anything, "21 "},
{Anything, 0, Anything, Silent}};

static Rule F_rules[] = {{Anything, "FUL",Anything, "22 8 34 "},
{Anything, "F",Anything, "22 "},
{Anything, 0, Anything, Silent}};

static Rule P_rules[] = {{Anything, "PH",Anything, "22 "},
{Anything, "PEOP",Anything, "16 0 16 "},
{Anything, "POW",Anything, "16 14 "},
{Anything, "PUT",Nothing, "16 8 18 "},
{Anything, "P",Anything, "16 "},
{Anything, 0, Anything, Silent}};

Rule *Rules[] =
	{
	punct_rules,
	A_rules, B_rules, C_rules, D_rules, E_rules, F_rules, G_rules, 
	H_rules, I_rules, J_rules, K_rules, L_rules, M_rules, N_rules, 
	O_rules, P_rules, Q_rules, R_rules, S_rules, T_rules, U_rules, 
	V_rules, W_rules, X_rules, Y_rules, Z_rules
	};


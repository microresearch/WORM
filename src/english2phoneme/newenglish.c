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

typedef struct{
  char *rulee[3];
  char oot[8];
} Rule;

/*0 = Punctuation */
/*
**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
*/

/*
** NEW RULES (space delimited) BELOW
*/

const Rule punct_rules[] __attribute__ ((section (".flash"))) = {{{Anything, " ",Anything},{41,-1}},
{{Anything, "--",Anything},{41,-1}},
{{Anything, "-",Anything},{-1}},
{{".", "'S",Anything},{27,-1}},
{{"#:.E", "'S",Anything},{27,-1}},
{{"#", "'S",Anything},{27,-1}},
{{Anything, "'",Anything},{-1}},
{{Anything, ",",Anything},{41,-1}},
{{Anything, ".",Anything},{41,-1}},
{{Anything, "?",Anything},{41,-1}},
{{Anything, "!",Anything},{41,-1}},
{{Anything, "@",Anything},{4,18,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule A_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "A",Nothing},{11,-1}},
{{Nothing, "ARE",Nothing},{5,37,-1}},
{{Nothing, "AR","O"},{11,37,-1}},
{{Anything, "AR","#"},{3,37,-1}},
{{"^", "AS","#"},{2,26,-1}},
{{Anything, "A","WA"},{11,-1}},
{{Anything, "AW",Anything},{6,-1}},
{{" :", "ANY",Anything},{3,32,0,-1}},
{{Anything, "A","^+#"},{2,-1}},
{{"#:", "ALLY",Anything},{11,34,0,-1}},
{{Nothing, "AL","#"},{11,34,-1}},
{{Anything, "AGAIN",Anything},{11,21,3,32,-1}},
{{"#:", "AG","E"},{1,39,-1}},
{{Anything, "A","^+:#"},{4,-1}},
{{" :", "A","^+ "},{2,-1}},
{{Anything, "A","^%"},{2,-1}},
{{Nothing, "ARR",Anything},{11,37,-1}},
{{Anything, "ARR",Anything},{4,37,-1}},
{{" :", "AR",Nothing},{5,37,-1}},
{{Anything, "AR",Nothing},{10,-1}},
{{Anything, "AR",Anything},{5,37,-1}},
{{Anything, "AIR",Anything},{3,37,-1}},
{{Anything, "AI",Anything},{2,-1}},
{{Anything, "AY",Anything},{2,-1}},
{{Anything, "AU",Anything},{6,-1}},
{{"#:", "AL",Nothing},{11,34,-1}},
{{"#:", "ALS",Nothing},{11,34,27,-1}},
{{Anything, "ALK",Anything},{6,20,-1}},
{{Anything, "AL","^"},{6,34,-1}},
{{" :", "ABLE",Anything},{2,17,11,34,-1}},
{{Anything, "ABLE",Anything},{11,17,11,34,-1}},
{{Anything, "ANG","+"},{2,32,39,-1}},
{{Anything, "A",Anything},{4,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule C_rules[] __attribute__ ((section (".flash"))) = {{{Nothing, "CH","^"},{20,-1}},
{{"^E", "CH",Anything},{20,-1}},
{{Anything, "CH",Anything},{38,-1}},
{{" S", "CI","#"},{26,13,-1}},
{{Anything, "CI","A"},{28,-1}},
{{Anything, "CI","O"},{28,-1}},
{{Anything, "CI","EN"},{28,-1}},
{{Anything, "C","+"},{26,-1}},
{{Anything, "CK",Anything},{20,-1}},
{{Anything, "COM","%"},{20,12,31,-1}},
{{Anything, "C",Anything},{20,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule B_rules[] __attribute__ ((section (".flash"))) = {{{Nothing, "BE","^#"},{17,1,-1}},
{{Anything, "BEING",Anything},{17,0,1,33,-1}},
{{Nothing, "BOTH",Nothing},{17,7,24,-1}},
{{Nothing, "BUS","#"},{17,1,27,-1}},
{{Anything, "BUIL",Anything},{17,1,34,-1}},
{{Anything, "B",Anything},{17,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule E_rules[] __attribute__ ((section (".flash"))) = {{{"#:", "E",Nothing},{-1}},
{{"':^", "E",Nothing},{-1}},
{{" :", "E",Nothing},{0,-1}},
{{"#", "ED",Nothing},{19,-1}},
{{"#:", "E","D "},{-1}},
{{Anything, "EV","ER"},{3,23,-1}},
{{Anything, "E","^%"},{0,-1}},
{{Anything, "ERI","#"},{0,37,0,-1}},
{{Anything, "ERI",Anything},{3,37,1,-1}},
{{"#:", "ER","#"},{10,-1}},
{{Anything, "ER","#"},{3,37,-1}},
{{Anything, "ER",Anything},{10,-1}},
{{Nothing, "EVEN",Anything},{0,23,3,32,-1}},
{{"#:", "E","W"},{-1}},
{{"T", "EW",Anything},{9,-1}},
{{"S", "EW",Anything},{9,-1}},
{{"R", "EW",Anything},{9,-1}},
{{"D", "EW",Anything},{9,-1}},
{{"L", "EW",Anything},{9,-1}},
{{"Z", "EW",Anything},{9,-1}},
{{"N", "EW",Anything},{9,-1}},
{{"J", "EW",Anything},{9,-1}},
{{"TH", "EW",Anything},{9,-1}},
{{"CH", "EW",Anything},{9,-1}},
{{"SH", "EW",Anything},{9,-1}},
{{Anything, "EW",Anything},{36,9,-1}},
{{Anything, "E","O"},{0,-1}},
{{"#:S", "ES",Nothing},{1,27,-1}},
{{"#:C", "ES",Nothing},{1,27,-1}},
{{"#:G", "ES",Nothing},{1,27,-1}},
{{"#:Z", "ES",Nothing},{1,27,-1}},
{{"#:X", "ES",Nothing},{1,27,-1}},
{{"#:J", "ES",Nothing},{1,27,-1}},
{{"#:CH", "ES",Nothing},{1,27,-1}},
{{"#:SH", "ES",Nothing},{1,27,-1}},
{{"#:", "E","S "},{-1}},
{{"#:", "ELY",Nothing},{34,0,-1}},
{{"#:", "EMENT",Anything},{31,3,32,18,-1}},
{{Anything, "EFUL",Anything},{22,8,34,-1}},
{{Anything, "EE",Anything},{0,-1}},
{{Anything, "EARN",Anything},{10,32,-1}},
{{Nothing, "EAR","^"},{10,-1}},
{{Anything, "EAD",Anything},{3,19,-1}},
{{"#:", "EA",Nothing},{0,11,-1}},
{{Anything, "EA","SU"},{3,-1}},
{{Anything, "EA",Anything},{0,-1}},
{{Anything, "EIGH",Anything},{2,-1}},
{{Anything, "EI",Anything},{0,-1}},
{{Nothing, "EYE",Anything},{13,-1}},
{{Anything, "EY",Anything},{0,-1}},
{{Anything, "EU",Anything},{36,9,-1}},
{{Anything, "E",Anything},{3,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule D_rules[] __attribute__ ((section (".flash"))) = {{{"#:", "DED",Nothing},{19,1,19,-1}},
{{".E", "D",Nothing},{19,-1}},
{{"#:^E", "D",Nothing},{18,-1}},
{{Nothing, "DE","^#"},{19,1,-1}},
{{Nothing, "DO",Nothing},{19,9,-1}},
{{Nothing, "DOES",Anything},{19,12,27,-1}},
{{Nothing, "DOING",Anything},{19,9,1,33,-1}},
{{Nothing, "DOW",Anything},{19,14,-1}},
{{Anything, "DU","A"},{39,9,-1}},
{{Anything, "D",Anything},{19,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule G_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "GIV",Anything},{21,1,23,-1}},
{{Nothing, "G","I^"},{21,-1}},
{{Anything, "GE","T"},{21,3,-1}},
{{"SU", "GGES",Anything},{21,39,3,26,-1}},
{{Anything, "GG",Anything},{21,-1}},
{{" B#", "G",Anything},{21,-1}},
{{Anything, "G","+"},{39,-1}},
{{Anything, "GREAT",Anything},{21,37,2,18,-1}},
{{"#", "GH",Anything},{-1}},
{{Anything, "G",Anything},{21,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule F_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "FUL",Anything},{22,8,34,-1}},
{{Anything, "F",Anything},{22,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule I_rules[] __attribute__ ((section (".flash"))) = {{{Nothing, "IN",Anything},{1,32,-1}},
{{Nothing, "I",Nothing},{13,-1}},
{{Anything, "IN","D"},{13,32,-1}},
{{Anything, "IER",Anything},{0,10,-1}},
{{"#:R", "IED",Anything},{0,19,-1}},
{{Anything, "IED",Nothing},{13,19,-1}},
{{Anything, "IEN",Anything},{0,3,32,-1}},
{{Anything, "IE","T"},{13,3,-1}},
{{" :", "I","%"},{13,-1}},
{{Anything, "I","%"},{0,-1}},
{{Anything, "IE",Anything},{0,-1}},
{{Anything, "I","^+:#"},{1,-1}},
{{Anything, "IR","#"},{13,37,-1}},
{{Anything, "IZ","%"},{13,27,-1}},
{{Anything, "IS","%"},{13,27,-1}},
{{Anything, "I","D%"},{13,-1}},
{{"+^", "I","^+"},{1,-1}},
{{Anything, "I","T%"},{13,-1}},
{{"#:^", "I","^+"},{1,-1}},
{{Anything, "I","^+"},{13,-1}},
{{Anything, "IR",Anything},{10,-1}},
{{Anything, "IGH",Anything},{13,-1}},
{{Anything, "ILD",Anything},{13,34,19,-1}},
{{Anything, "IGN",Nothing},{13,32,-1}},
{{Anything, "IGN","^"},{13,32,-1}},
{{Anything, "IGN","%"},{13,32,-1}},
{{Anything, "IQUE",Anything},{0,20,-1}},
{{Anything, "I",Anything},{1,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule H_rules[] __attribute__ ((section (".flash"))) = {{{Nothing, "HAV",Anything},{30,4,23,-1}},
{{Nothing, "HERE",Anything},{30,0,37,-1}},
{{Nothing, "HOUR",Anything},{14,10,-1}},
{{Anything, "HOW",Anything},{30,14,-1}},
{{Anything, "H","#"},{30,-1}},
{{Anything, "H",Anything},{-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule K_rules[] __attribute__ ((section (".flash"))) = {{{Nothing, "K","N"},{-1}},
{{Anything, "K",Anything},{20,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule J_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "J",Anything},{39,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule M_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "MOV",Anything},{31,9,23,-1}},
{{Anything, "M",Anything},{31,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule L_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "LO","C#"},{34,7,-1}},
{{"L", "L",Anything},{-1}},
{{"#:^", "L","%"},{11,34,-1}},
{{Anything, "LEAD",Anything},{34,0,19,-1}},
{{Anything, "L",Anything},{34,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule O_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "OF",Nothing},{11,23,-1}},
{{Anything, "OROUGH",Anything},{10,7,-1}},
{{"#:", "OR",Nothing},{10,-1}},
{{"#:", "ORS",Nothing},{10,27,-1}},
{{Anything, "OR",Anything},{6,37,-1}},
{{Nothing, "ONE",Anything},{35,12,32,-1}},
{{Nothing, "OW",Anything},{7,-1}},
{{Anything, "OW",Anything},{7,-1}},
{{Nothing, "OVER",Anything},{7,23,10,-1}},
{{Anything, "OV",Anything},{12,23,-1}},
{{Anything, "O","^%"},{7,-1}},
{{Anything, "O","^EN"},{7,-1}},
{{Anything, "O","^I#"},{7,-1}},
{{Anything, "OL","D"},{7,34,-1}},
{{Anything, "OUGHT",Anything},{6,18,-1}},
{{Anything, "OUGH",Anything},{12,22,-1}},
{{Nothing, "OU",Anything},{14,-1}},
{{"H", "OU","S#"},{14,-1}},
{{Anything, "OUS",Anything},{11,26,-1}},
{{Anything, "OUR",Anything},{6,37,-1}},
{{Anything, "OULD",Anything},{8,19,-1}},
{{"^", "OU","^L"},{12,-1}},
{{Anything, "OUP",Anything},{9,16,-1}},
{{Anything, "OU",Anything},{14,-1}},
{{Anything, "OY",Anything},{15,-1}},
{{Anything, "OING",Anything},{7,1,33,-1}},
{{Anything, "OI",Anything},{15,-1}},
{{Anything, "OOR",Anything},{6,37,-1}},
{{Anything, "OOK",Anything},{8,20,-1}},
{{Anything, "OOD",Anything},{8,19,-1}},
{{Anything, "OO",Anything},{9,-1}},
{{Anything, "O","E"},{7,-1}},
{{Anything, "O",Nothing},{7,-1}},
{{Anything, "OA",Anything},{7,-1}},
{{Nothing, "ONLY",Anything},{7,32,34,0,-1}},
{{Nothing, "ONCE",Anything},{35,12,32,26,-1}},
{{Anything, "ON'T",Anything},{7,32,18,-1}},
{{"C", "O","N"},{5,-1}},
{{Anything, "O","NG"},{6,-1}},
{{" :^", "O","N"},{12,-1}},
{{"I", "ON",Anything},{11,32,-1}},
{{"#:", "ON",Nothing},{11,32,-1}},
{{"#^", "ON",Anything},{11,32,-1}},
{{Anything, "O","ST "},{7,-1}},
{{Anything, "OF","^"},{6,22,-1}},
{{Anything, "OTHER",Anything},{12,25,10,-1}},
{{Anything, "OSS",Nothing},{6,26,-1}},
{{"#:^", "OM",Anything},{12,31,-1}},
{{Anything, "O",Anything},{5,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule N_rules[] __attribute__ ((section (".flash"))) = {{{"E", "NG","+"},{32,39,-1}},
{{Anything, "NG","R"},{33,21,-1}},
{{Anything, "NG","#"},{33,21,-1}},
{{Anything, "NGL","%"},{33,21,11,34,-1}},
{{Anything, "NG",Anything},{33,-1}},
{{Anything, "NK",Anything},{33,20,-1}},
{{Nothing, "NOW",Nothing},{32,14,-1}},
{{Anything, "N",Anything},{32,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule Q_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "QUAR",Anything},{20,35,6,37,-1}},
{{Anything, "QU",Anything},{20,35,-1}},
{{Anything, "Q",Anything},{20,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule P_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "PH",Anything},{22,-1}},
{{Anything, "PEOP",Anything},{16,0,16,-1}},
{{Anything, "POW",Anything},{16,14,-1}},
{{Anything, "PUT",Nothing},{16,8,18,-1}},
{{Anything, "P",Anything},{16,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule S_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "SH",Anything},{28,-1}},
{{"#", "SION",Anything},{29,11,32,-1}},
{{Anything, "SOME",Anything},{26,12,31,-1}},
{{"#", "SUR","#"},{29,10,-1}},
{{Anything, "SUR","#"},{28,10,-1}},
{{"#", "SU","#"},{29,9,-1}},
{{"#", "SSU","#"},{28,9,-1}},
{{"#", "SED",Nothing},{27,19,-1}},
{{"#", "S","#"},{27,-1}},
{{Anything, "SAID",Anything},{26,3,19,-1}},
{{"^", "SION",Anything},{28,11,32,-1}},
{{Anything, "S","S"},{-1}},
{{".", "S",Nothing},{27,-1}},
{{"#:.E", "S",Nothing},{27,-1}},
{{"#:^##", "S",Nothing},{27,-1}},
{{"#:^#", "S",Nothing},{26,-1}},
{{"U", "S",Nothing},{26,-1}},
{{" :#", "S",Nothing},{27,-1}},
{{Nothing, "SCH",Anything},{26,20,-1}},
{{Anything, "S","C+"},{-1}},
{{"#", "SM",Anything},{27,31,-1}},
{{"#", "SN","'"},{27,11,32,-1}},
{{Anything, "S",Anything},{26,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule R_rules[] __attribute__ ((section (".flash"))) = {{{Nothing, "RE","^#"},{37,0,-1}},
{{Anything, "R",Anything},{37,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule U_rules[] __attribute__ ((section (".flash"))) = {{{Nothing, "UN","I"},{36,9,32,-1}},
{{Nothing, "UN",Anything},{12,32,-1}},
{{Nothing, "UPON",Anything},{11,16,6,32,-1}},
{{"T", "UR","#"},{8,37,-1}},
{{"S", "UR","#"},{8,37,-1}},
{{"R", "UR","#"},{8,37,-1}},
{{"D", "UR","#"},{8,37,-1}},
{{"L", "UR","#"},{8,37,-1}},
{{"Z", "UR","#"},{8,37,-1}},
{{"N", "UR","#"},{8,37,-1}},
{{"J", "UR","#"},{8,37,-1}},
{{"TH", "UR","#"},{8,37,-1}},
{{"CH", "UR","#"},{8,37,-1}},
{{"SH", "UR","#"},{8,37,-1}},
{{Anything, "UR","#"},{36,8,37,-1}},
{{Anything, "UR",Anything},{10,-1}},
{{Anything, "U","^ "},{12,-1}},
{{Anything, "U","^^"},{12,-1}},
{{Anything, "UY",Anything},{13,-1}},
{{" G", "U","#"},{-1}},
{{"G", "U","%"},{-1}},
{{"G", "U","#"},{35,-1}},
{{"#N", "U",Anything},{36,9,-1}},
{{"T", "U",Anything},{9,-1}},
{{"S", "U",Anything},{9,-1}},
{{"R", "U",Anything},{9,-1}},
{{"D", "U",Anything},{9,-1}},
{{"L", "U",Anything},{9,-1}},
{{"Z", "U",Anything},{9,-1}},
{{"N", "U",Anything},{9,-1}},
{{"J", "U",Anything},{9,-1}},
{{"TH", "U",Anything},{9,-1}},
{{"CH", "U",Anything},{9,-1}},
{{"SH", "U",Anything},{9,-1}},
{{Anything, "U",Anything},{36,9,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule T_rules[] __attribute__ ((section (".flash"))) = {{{Nothing, "THE",Nothing},{25,11,-1}},
{{Anything, "TO",Nothing},{18,9,-1}},
{{Anything, "THAT",Nothing},{25,4,18,-1}},
{{Nothing, "THIS",Nothing},{25,1,26,-1}},
{{Nothing, "THEY",Anything},{25,2,-1}},
{{Nothing, "THERE",Anything},{25,3,37,-1}},
{{Anything, "THER",Anything},{25,10,-1}},
{{Anything, "THEIR",Anything},{25,3,37,-1}},
{{Nothing, "THAN",Nothing},{25,4,32,-1}},
{{Nothing, "THEM",Nothing},{25,3,31,-1}},
{{Anything, "THESE",Nothing},{25,0,27,-1}},
{{Nothing, "THEN",Anything},{25,3,32,-1}},
{{Anything, "THROUGH",Anything},{24,37,9,-1}},
{{Anything, "THOSE",Anything},{25,7,27,-1}},
{{Anything, "THOUGH",Nothing},{25,7,-1}},
{{Nothing, "THUS",Anything},{25,12,26,-1}},
{{Anything, "TH",Anything},{24,-1}},
{{"#:", "TED",Nothing},{18,1,19,-1}},
{{"S", "TI","#N"},{38,-1}},
{{Anything, "TI","O"},{28,-1}},
{{Anything, "TI","A"},{28,-1}},
{{Anything, "TIEN",Anything},{28,11,32,-1}},
{{Anything, "TUR","#"},{38,10,-1}},
{{Anything, "TU","A"},{38,9,-1}},
{{Nothing, "TWO",Anything},{18,9,-1}},
{{Anything, "T",Anything},{18,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule W_rules[] __attribute__ ((section (".flash"))) = {{{Nothing, "WERE",Anything},{35,10,-1}},
{{Anything, "WA","S"},{35,5,-1}},
{{Anything, "WA","T"},{35,5,-1}},
{{Anything, "WHERE",Anything},{40,3,37,-1}},
{{Anything, "WHAT",Anything},{40,5,18,-1}},
{{Anything, "WHOL",Anything},{30,7,34,-1}},
{{Anything, "WHO",Anything},{30,9,-1}},
{{Anything, "WH",Anything},{40,-1}},
{{Anything, "WAR",Anything},{35,6,37,-1}},
{{Anything, "WOR","^"},{35,10,-1}},
{{Anything, "WR",Anything},{37,-1}},
{{Anything, "W",Anything},{35,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule V_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "VIEW",Anything},{23,36,9,-1}},
{{Anything, "V",Anything},{23,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule Y_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "YOUNG",Anything},{36,12,33,-1}},
{{Nothing, "YOU",Anything},{36,9,-1}},
{{Nothing, "YES",Anything},{36,3,26,-1}},
{{Nothing, "Y",Anything},{36,-1}},
{{"#:^", "Y",Nothing},{0,-1}},
{{"#:^", "Y","I"},{0,-1}},
{{" :", "Y",Nothing},{13,-1}},
{{" :", "Y","#"},{13,-1}},
{{" :", "Y","^+:#"},{1,-1}},
{{" :", "Y","^#"},{13,-1}},
{{Anything, "Y",Anything},{1,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule X_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "X",Anything},{20,26,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule Z_rules[] __attribute__ ((section (".flash"))) = {{{Anything, "Z",Anything},{27,-1}},
{{Anything, 0, Anything}, {-2}}};

const Rule *Rules[]  =
	{
	punct_rules,
	A_rules, B_rules, C_rules, D_rules, E_rules, F_rules, G_rules, 
	H_rules, I_rules, J_rules, K_rules, L_rules, M_rules, N_rules, 
	O_rules, P_rules, Q_rules, R_rules, S_rules, T_rules, U_rules, 
	V_rules, W_rules, X_rules, Y_rules, Z_rules
	};

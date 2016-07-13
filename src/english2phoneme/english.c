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

typedef char *Rule[4];	/* Rule is an array of 4 character pointers */

/*0 = Punctuation */
/*
**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
*/
//static Rule punct_rules[] =
//	{
//	{Anything,	" ",		Anything,	Pause	},
//	{Anything,	"-",		Anything,	Silent	},
//	{".",		"'S",		Anything,	"z"	},
//	{"#:.E",	"'S",		Anything,	"z"	},
//	{"#",		"'S",		Anything,	"z"	},
//	{Anything,	"'",		Anything,	Silent	},
//	{Anything,	",",		Anything,	Pause	},
//	{Anything,	".",		Anything,	Pause	},
//	{Anything,	"?",		Anything,	Pause	},
//	{Anything,	"!",		Anything,	Pause	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule A_rules[] =
//	{
//	{Anything,	"A",		Nothing,	"AX"	},
//	{Nothing,	"ARE",		Nothing,	"AAr"	},
//	{Nothing,	"AR",		"O",		"AXr"	},
//	{Anything,	"AR",		"#",		"EHr"	},
//	{"^",		"AS",		"#",		"EYs"	},
//	{Anything,	"A",		"WA",		"AX"	},
//	{Anything,	"AW",		Anything,	"AO"	},
//	{" :",		"ANY",		Anything,	"EHnIY"	},
//	{Anything,	"A",		"^+#",		"EY"	},
//	{"#:",		"ALLY",		Anything,	"AXlIY"	},
//	{Nothing,	"AL",		"#",		"AXl"	},
//	{Anything,	"AGAIN",	Anything,	"AXgEHn"},
//	{"#:",		"AG",		"E",		"IHj"	},
//	{Anything,	"A",		"^+:#",		"AE"	},
//	{" :",		"A",		"^+ ",		"EY"	},
//	{Anything,	"A",		"^%",		"EY"	},
//	{Nothing,	"ARR",		Anything,	"AXr"	},
//	{Anything,	"ARR",		Anything,	"AEr"	},
//	{" :",		"AR",		Nothing,	"AAr"	},
//	{Anything,	"AR",		Nothing,	"ER"	},
//	{Anything,	"AR",		Anything,	"AAr"	},
//	{Anything,	"AIR",		Anything,	"EHr"	},
//	{Anything,	"AI",		Anything,	"EY"	},
//	{Anything,	"AY",		Anything,	"EY"	},
//	{Anything,	"AU",		Anything,	"AO"	},
//	{"#:",		"AL",		Nothing,	"AXl"	},
//	{"#:",		"ALS",		Nothing,	"AXlz"	},
//	{Anything,	"ALK",		Anything,	"AOk"	},
//	{Anything,	"AL",		"^",		"AOl"	},
//	{" :",		"ABLE",		Anything,	"EYbAXl"},
//	{Anything,	"ABLE",		Anything,	"AXbAXl"},
//	{Anything,	"ANG",		"+",		"EYnj"	},
//	{Anything,	"A",		Anything,	"AE"	},
// 	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule B_rules[] =
//	{
//	{Nothing,	"BE",		"^#",		"bIH"	},
//	{Anything,	"BEING",	Anything,	"bIYIHNG"},
//	{Nothing,	"BOTH",		Nothing,	"bOWTH"	},
//	{Nothing,	"BUS",		"#",		"bIHz"	},
//	{Anything,	"BUIL",		Anything,	"bIHl"	},
//	{Anything,	"B",		Anything,	"b"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule C_rules[] =
//	{
//	{Nothing,	"CH",		"^",		"k"	},
//	{"^E",		"CH",		Anything,	"k"	},
//	{Anything,	"CH",		Anything,	"CH"	},
//	{" S",		"CI",		"#",		"sAY"	},
//	{Anything,	"CI",		"A",		"SH"	},
//	{Anything,	"CI",		"O",		"SH"	},
//	{Anything,	"CI",		"EN",		"SH"	},
//	{Anything,	"C",		"+",		"s"	},
//	{Anything,	"CK",		Anything,	"k"	},
//	{Anything,	"COM",		"%",		"kAHm"	},
//	{Anything,	"C",		Anything,	"k"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule D_rules[] =
//	{
//	{"#:",		"DED",		Nothing,	"dIHd"	},
//	{".E",		"D",		Nothing,	"d"	},
//	{"#:^E",	"D",		Nothing,	"t"	},
//	{Nothing,	"DE",		"^#",		"dIH"	},
//	{Nothing,	"DO",		Nothing,	"dUW"	},
//	{Nothing,	"DOES",		Anything,	"dAHz"	},
//	{Nothing,	"DOING",	Anything,	"dUWIHNG"},
//	{Nothing,	"DOW",		Anything,	"dAW"	},
//	{Anything,	"DU",		"A",		"jUW"	},
//	{Anything,	"D",		Anything,	"d"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule E_rules[] =
//	{
//	{"#:",		"E",		Nothing,	Silent	},
//	{"':^",		"E",		Nothing,	Silent	},
//	{" :",		"E",		Nothing,	"IY"	},
//	{"#",		"ED",		Nothing,	"d"	},
//	{"#:",		"E",		"D ",		Silent	},
//	{Anything,	"EV",		"ER",		"EHv"	},
//	{Anything,	"E",		"^%",		"IY"	},
//	{Anything,	"ERI",		"#",		"IYrIY"	},
//	{Anything,	"ERI",		Anything,	"EHrIH"	},
//	{"#:",		"ER",		"#",		"ER"	},
//	{Anything,	"ER",		"#",		"EHr"	},
//	{Anything,	"ER",		Anything,	"ER"	},
//	{Nothing,	"EVEN",		Anything,	"IYvEHn"},
//	{"#:",		"E",		"W",		Silent	},
//	{"T",		"EW",		Anything,	"UW"	},
//	{"S",		"EW",		Anything,	"UW"	},
//	{"R",		"EW",		Anything,	"UW"	},
//	{"D",		"EW",		Anything,	"UW"	},
//	{"L",		"EW",		Anything,	"UW"	},
//	{"Z",		"EW",		Anything,	"UW"	},
//	{"N",		"EW",		Anything,	"UW"	},
//	{"J",		"EW",		Anything,	"UW"	},
//	{"TH",		"EW",		Anything,	"UW"	},
//	{"CH",		"EW",		Anything,	"UW"	},
//	{"SH",		"EW",		Anything,	"UW"	},
//	{Anything,	"EW",		Anything,	"yUW"	},
//	{Anything,	"E",		"O",		"IY"	},
//	{"#:S",		"ES",		Nothing,	"IHz"	},
//	{"#:C",		"ES",		Nothing,	"IHz"	},
//	{"#:G",		"ES",		Nothing,	"IHz"	},
//	{"#:Z",		"ES",		Nothing,	"IHz"	},
//	{"#:X",		"ES",		Nothing,	"IHz"	},
//	{"#:J",		"ES",		Nothing,	"IHz"	},
//	{"#:CH",	"ES",		Nothing,	"IHz"	},
//	{"#:SH",	"ES",		Nothing,	"IHz"	},
//	{"#:",		"E",		"S ",		Silent	},
//	{"#:",		"ELY",		Nothing,	"lIY"	},
//	{"#:",		"EMENT",	Anything,	"mEHnt"	},
//	{Anything,	"EFUL",		Anything,	"fUHl"	},
//	{Anything,	"EE",		Anything,	"IY"	},
//	{Anything,	"EARN",		Anything,	"ERn"	},
//	{Nothing,	"EAR",		"^",		"ER"	},
//	{Anything,	"EAD",		Anything,	"EHd"	},
//	{"#:",		"EA",		Nothing,	"IYAX"	},
//	{Anything,	"EA",		"SU",		"EH"	},
//	{Anything,	"EA",		Anything,	"IY"	},
//	{Anything,	"EIGH",		Anything,	"EY"	},
//	{Anything,	"EI",		Anything,	"IY"	},
//	{Nothing,	"EYE",		Anything,	"AY"	},
//	{Anything,	"EY",		Anything,	"IY"	},
//	{Anything,	"EU",		Anything,	"yUW"	},
//	{Anything,	"E",		Anything,	"EH"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule F_rules[] =
//	{
//	{Anything,	"FUL",		Anything,	"fUHl"	},
//	{Anything,	"F",		Anything,	"f"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule G_rules[] =
//	{
//	{Anything,	"GIV",		Anything,	"gIHv"	},
//	{Nothing,	"G",		"I^",		"g"	},
//	{Anything,	"GE",		"T",		"gEH"	},
//	{"SU",		"GGES",		Anything,	"gjEHs"	},
//	{Anything,	"GG",		Anything,	"g"	},
//	{" B#",		"G",		Anything,	"g"	},
//	{Anything,	"G",		"+",		"j"	},
//	{Anything,	"GREAT",	Anything,	"grEYt"	},
//	{"#",		"GH",		Anything,	Silent	},
//	{Anything,	"G",		Anything,	"g"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule H_rules[] =
//	{
//	{Nothing,	"HAV",		Anything,	"hAEv"	},
//	{Nothing,	"HERE",		Anything,	"hIYr"	},
//	{Nothing,	"HOUR",		Anything,	"AWER"	},
//	{Anything,	"HOW",		Anything,	"hAW"	},
//	{Anything,	"H",		"#",		"h"	},
//	{Anything,	"H",		Anything,	Silent	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule I_rules[] =
//	{
//	{Nothing,	"IN",		Anything,	"IHn"	},
//	{Nothing,	"I",		Nothing,	"AY"	},
//	{Anything,	"IN",		"D",		"AYn"	},
//	{Anything,	"IER",		Anything,	"IYER"	},
//	{"#:R",		"IED",		Anything,	"IYd"	},
//	{Anything,	"IED",		Nothing,	"AYd"	},
//	{Anything,	"IEN",		Anything,	"IYEHn"	},
//	{Anything,	"IE",		"T",		"AYEH"	},
//	{" :",		"I",		"%",		"AY"	},
//	{Anything,	"I",		"%",		"IY"	},
//	{Anything,	"IE",		Anything,	"IY"	},
//	{Anything,	"I",		"^+:#",		"IH"	},
//	{Anything,	"IR",		"#",		"AYr"	},
//	{Anything,	"IZ",		"%",		"AYz"	},
//	{Anything,	"IS",		"%",		"AYz"	},
//	{Anything,	"I",		"D%",		"AY"	},
//	{"+^",		"I",		"^+",		"IH"	},
//	{Anything,	"I",		"T%",		"AY"	},
//	{"#:^",		"I",		"^+",		"IH"	},
//	{Anything,	"I",		"^+",		"AY"	},
//	{Anything,	"IR",		Anything,	"ER"	},
//	{Anything,	"IGH",		Anything,	"AY"	},
//	{Anything,	"ILD",		Anything,	"AYld"	},
//	{Anything,	"IGN",		Nothing,	"AYn"	},
//	{Anything,	"IGN",		"^",		"AYn"	},
//	{Anything,	"IGN",		"%",		"AYn"	},
//	{Anything,	"IQUE",		Anything,	"IYk"	},
//	{Anything,	"I",		Anything,	"IH"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule J_rules[] =
//	{
//	{Anything,	"J",		Anything,	"j"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule K_rules[] =
//	{
//	{Nothing,	"K",		"N",		Silent	},
//	{Anything,	"K",		Anything,	"k"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule L_rules[] =
//	{
//	{Anything,	"LO",		"C#",		"lOW"	},
//	{"L",		"L",		Anything,	Silent	},
//	{"#:^",		"L",		"%",		"AXl"	},
//	{Anything,	"LEAD",		Anything,	"lIYd"	},
//	{Anything,	"L",		Anything,	"l"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule M_rules[] =
//	{
//	{Anything,	"MOV",		Anything,	"mUWv"	},
//	{Anything,	"M",		Anything,	"m"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule N_rules[] =
//	{
//	{"E",		"NG",		"+",		"nj"	},
//	{Anything,	"NG",		"R",		"NGg"	},
//	{Anything,	"NG",		"#",		"NGg"	},
//	{Anything,	"NGL",		"%",		"NGgAXl"},
//	{Anything,	"NG",		Anything,	"NG"	},
//	{Anything,	"NK",		Anything,	"NGk"	},
//	{Nothing,	"NOW",		Nothing,	"nAW"	},
//	{Anything,	"N",		Anything,	"n"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule O_rules[] =
//	{
//	{Anything,	"OF",		Nothing,	"AXv"	},
//	{Anything,	"OROUGH",	Anything,	"EROW"	},
//	{"#:",		"OR",		Nothing,	"ER"	},
//	{"#:",		"ORS",		Nothing,	"ERz"	},
//	{Anything,	"OR",		Anything,	"AOr"	},
//	{Nothing,	"ONE",		Anything,	"wAHn"	},
//	{Anything,	"OW",		Anything,	"OW"	},
//	{Nothing,	"OVER",		Anything,	"OWvER"	},
//	{Anything,	"OV",		Anything,	"AHv"	},
//	{Anything,	"O",		"^%",		"OW"	},
//	{Anything,	"O",		"^EN",		"OW"	},
//	{Anything,	"O",		"^I#",		"OW"	},
//	{Anything,	"OL",		"D",		"OWl"	},
//	{Anything,	"OUGHT",	Anything,	"AOt"	},
//	{Anything,	"OUGH",		Anything,	"AHf"	},
//	{Nothing,	"OU",		Anything,	"AW"	},
//	{"H",		"OU",		"S#",		"AW"	},
//	{Anything,	"OUS",		Anything,	"AXs"	},
//	{Anything,	"OUR",		Anything,	"AOr"	},
//	{Anything,	"OULD",		Anything,	"UHd"	},
//	{"^",		"OU",		"^L",		"AH"	},
//	{Anything,	"OUP",		Anything,	"UWp"	},
//	{Anything,	"OU",		Anything,	"AW"	},
//	{Anything,	"OY",		Anything,	"OY"	},
//	{Anything,	"OING",		Anything,	"OWIHNG"},
//	{Anything,	"OI",		Anything,	"OY"	},
//	{Anything,	"OOR",		Anything,	"AOr"	},
//	{Anything,	"OOK",		Anything,	"UHk"	},
//	{Anything,	"OOD",		Anything,	"UHd"	},
//	{Anything,	"OO",		Anything,	"UW"	},
//	{Anything,	"O",		"E",		"OW"	},
//	{Anything,	"O",		Nothing,	"OW"	},
//	{Anything,	"OA",		Anything,	"OW"	},
//	{Nothing,	"ONLY",		Anything,	"OWnlIY"},
//	{Nothing,	"ONCE",		Anything,	"wAHns"	},
//	{Anything,	"ON'T",		Anything,	"OWnt"	},
//	{"C",		"O",		"N",		"AA"	},
//	{Anything,	"O",		"NG",		"AO"	},
//	{" :^",		"O",		"N",		"AH"	},
//	{"I",		"ON",		Anything,	"AXn"	},
//	{"#:",		"ON",		Nothing,	"AXn"	},
//	{"#^",		"ON",		Anything,	"AXn"	},
//	{Anything,	"O",		"ST ",		"OW"	},
//	{Anything,	"OF",		"^",		"AOf"	},
//	{Anything,	"OTHER",	Anything,	"AHDHER"},
//	{Anything,	"OSS",		Nothing,	"AOs"	},
//	{"#:^",		"OM",		Anything,	"AHm"	},
//	{Anything,	"O",		Anything,	"AA"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule P_rules[] =
//	{
//	{Anything,	"PH",		Anything,	"f"	},
//	{Anything,	"PEOP",		Anything,	"pIYp"	},
//	{Anything,	"POW",		Anything,	"pAW"	},
//	{Anything,	"PUT",		Nothing,	"pUHt"	},
//	{Anything,	"P",		Anything,	"p"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule Q_rules[] =
//	{
//	{Anything,	"QUAR",		Anything,	"kwAOr"	},
//	{Anything,	"QU",		Anything,	"kw"	},
//	{Anything,	"Q",		Anything,	"k"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule R_rules[] =
//	{
//	{Nothing,	"RE",		"^#",		"rIY"	},
//	{Anything,	"R",		Anything,	"r"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule S_rules[] =
//	{
//	{Anything,	"SH",		Anything,	"SH"	},
//	{"#",		"SION",		Anything,	"ZHAXn"	},
//	{Anything,	"SOME",		Anything,	"sAHm"	},
//	{"#",		"SUR",		"#",		"ZHER"	},
//	{Anything,	"SUR",		"#",		"SHER"	},
//	{"#",		"SU",		"#",		"ZHUW"	},
//	{"#",		"SSU",		"#",		"SHUW"	},
//	{"#",		"SED",		Nothing,	"zd"	},
//	{"#",		"S",		"#",		"z"	},
//	{Anything,	"SAID",		Anything,	"sEHd"	},
//	{"^",		"SION",		Anything,	"SHAXn"	},
//	{Anything,	"S",		"S",		Silent	},
//	{".",		"S",		Nothing,	"z"	},
//	{"#:.E",	"S",		Nothing,	"z"	},
//	{"#:^##",	"S",		Nothing,	"z"	},
//	{"#:^#",	"S",		Nothing,	"s"	},
//	{"U",		"S",		Nothing,	"s"	},
//	{" :#",		"S",		Nothing,	"z"	},
//	{Nothing,	"SCH",		Anything,	"sk"	},
//	{Anything,	"S",		"C+",		Silent	},
//	{"#",		"SM",		Anything,	"zm"	},
//	{"#",		"SN",		"'",		"zAXn"	},
//	{Anything,	"S",		Anything,	"s"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule T_rules[] =
//	{
//	{Nothing,	"THE",		Nothing,	"DHAX"	},
//	{Anything,	"TO",		Nothing,	"tUW"	},
//	{Anything,	"THAT",		Nothing,	"DHAEt"	},
//	{Nothing,	"THIS",		Nothing,	"DHIHs"	},
//	{Nothing,	"THEY",		Anything,	"DHEY"	},
//	{Nothing,	"THERE",	Anything,	"DHEHr"	},
//	{Anything,	"THER",		Anything,	"DHER"	},
//	{Anything,	"THEIR",	Anything,	"DHEHr"	},
//	{Nothing,	"THAN",		Nothing,	"DHAEn"	},
//	{Nothing,	"THEM",		Nothing,	"DHEHm"	},
//	{Anything,	"THESE",	Nothing,	"DHIYz"	},
//	{Nothing,	"THEN",		Anything,	"DHEHn"	},
//	{Anything,	"THROUGH",	Anything,	"THrUW"	},
//	{Anything,	"THOSE",	Anything,	"DHOWz"	},
//	{Anything,	"THOUGH",	Nothing,	"DHOW"	},
//	{Nothing,	"THUS",		Anything,	"DHAHs"	},
//	{Anything,	"TH",		Anything,	"TH"	},
//	{"#:",		"TED",		Nothing,	"tIHd"	},
//	{"S",		"TI",		"#N",		"CH"	},
//	{Anything,	"TI",		"O",		"SH"	},
//	{Anything,	"TI",		"A",		"SH"	},
//	{Anything,	"TIEN",		Anything,	"SHAXn"	},
//	{Anything,	"TUR",		"#",		"CHER"	},
//	{Anything,	"TU",		"A",		"CHUW"	},
//	{Nothing,	"TWO",		Anything,	"tUW"	},
//	{Anything,	"T",		Anything,	"t"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule U_rules[] =
//	{
//	{Nothing,	"UN",		"I",		"yUWn"	},
//	{Nothing,	"UN",		Anything,	"AHn"	},
//	{Nothing,	"UPON",		Anything,	"AXpAOn"},
//	{"T",		"UR",		"#",		"UHr"	},
//	{"S",		"UR",		"#",		"UHr"	},
//	{"R",		"UR",		"#",		"UHr"	},
//	{"D",		"UR",		"#",		"UHr"	},
//	{"L",		"UR",		"#",		"UHr"	},
//	{"Z",		"UR",		"#",		"UHr"	},
//	{"N",		"UR",		"#",		"UHr"	},
//	{"J",		"UR",		"#",		"UHr"	},
//	{"TH",		"UR",		"#",		"UHr"	},
//	{"CH",		"UR",		"#",		"UHr"	},
//	{"SH",		"UR",		"#",		"UHr"	},
//	{Anything,	"UR",		"#",		"yUHr"	},
//	{Anything,	"UR",		Anything,	"ER"	},
//	{Anything,	"U",		"^ ",		"AH"	},
//	{Anything,	"U",		"^^",		"AH"	},
//	{Anything,	"UY",		Anything,	"AY"	},
//	{" G",		"U",		"#",		Silent	},
//	{"G",		"U",		"%",		Silent	},
//	{"G",		"U",		"#",		"w"	},
//	{"#N",		"U",		Anything,	"yUW"	},
//	{"T",		"U",		Anything,	"UW"	},
//	{"S",		"U",		Anything,	"UW"	},
//	{"R",		"U",		Anything,	"UW"	},
//	{"D",		"U",		Anything,	"UW"	},
//	{"L",		"U",		Anything,	"UW"	},
//	{"Z",		"U",		Anything,	"UW"	},
//	{"N",		"U",		Anything,	"UW"	},
//	{"J",		"U",		Anything,	"UW"	},
//	{"TH",		"U",		Anything,	"UW"	},
//	{"CH",		"U",		Anything,	"UW"	},
//	{"SH",		"U",		Anything,	"UW"	},
//	{Anything,	"U",		Anything,	"yUW"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule V_rules[] =
//	{
//	{Anything,	"VIEW",		Anything,	"vyUW"	},
//	{Anything,	"V",		Anything,	"v"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule W_rules[] =
//	{
//	{Nothing,	"WERE",		Anything,	"wER"	},
//	{Anything,	"WA",		"S",		"wAA"	},
//	{Anything,	"WA",		"T",		"wAA"	},
//	{Anything,	"WHERE",	Anything,	"WHEHr"	},
//	{Anything,	"WHAT",		Anything,	"WHAAt"	},
//	{Anything,	"WHOL",		Anything,	"hOWl"	},
//	{Anything,	"WHO",		Anything,	"hUW"	},
//	{Anything,	"WH",		Anything,	"WH"	},
//	{Anything,	"WAR",		Anything,	"wAOr"	},
//	{Anything,	"WOR",		"^",		"wER"	},
//	{Anything,	"WR",		Anything,	"r"	},
//	{Anything,	"W",		Anything,	"w"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule X_rules[] =
//	{
//	{Anything,	"X",		Anything,	"ks"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule Y_rules[] =
//	{
//	{Anything,	"YOUNG",	Anything,	"yAHNG"	},
//	{Nothing,	"YOU",		Anything,	"yUW"	},
//	{Nothing,	"YES",		Anything,	"yEHs"	},
//	{Nothing,	"Y",		Anything,	"y"	},
//	{"#:^",		"Y",		Nothing,	"IY"	},
//	{"#:^",		"Y",		"I",		"IY"	},
//	{" :",		"Y",		Nothing,	"AY"	},
//	{" :",		"Y",		"#",		"AY"	},
//	{" :",		"Y",		"^+:#",		"IH"	},
//	{" :",		"Y",		"^#",		"AY"	},
//	{Anything,	"Y",		Anything,	"IH"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
///*
//**	LEFT_PART	MATCH_PART	RIGHT_PART	OUT_PART
//*/
//static Rule Z_rules[] =
//	{
//	{Anything,	"Z",		Anything,	"z"	},
//	{Anything,	0,		Anything,	Silent	},
//	};
//
//Rule *Rules[] =
//	{
//	punct_rules,
//	A_rules, B_rules, C_rules, D_rules, E_rules, F_rules, G_rules, 
//	H_rules, I_rules, J_rules, K_rules, L_rules, M_rules, N_rules, 
//	O_rules, P_rules, Q_rules, R_rules, S_rules, T_rules, U_rules, 
//	V_rules, W_rules, X_rules, Y_rules, Z_rules
//	};

/*
** NEW RULES BELOW
*/

static Rule K_rules[] = {{Nothing, "K","N", ""},
{Anything, "K",Anything, "20"},
{Anything, 0, Anything, Silent}};

static Rule D_rules[] = {{"#:", "DED",Nothing, "19119"},
{".E", "D",Nothing, "19"},
{"#:^E", "D",Nothing, "18"},
{Nothing, "DE","^#", "191"},
{Nothing, "DO",Nothing, "199"},
{Nothing, "DOES",Anything, "191227"},
{Nothing, "DOING",Anything, "199133"},
{Nothing, "DOW",Anything, "1914"},
{Anything, "DU","A", "399"},
{Anything, "D",Anything, "19"},
{Anything, 0, Anything, Silent}};

static Rule N_rules[] = {{"E", "NG","+", "3239"},
{Anything, "NG","R", "3321"},
{Anything, "NG","#", "3321"},
{Anything, "NGL","%", "33211134"},
{Anything, "NG",Anything, "33"},
{Anything, "NK",Anything, "3320"},
{Nothing, "NOW",Nothing, "3214"},
{Anything, "N",Anything, "32"},
{Anything, 0, Anything, Silent}};

static Rule P_rules[] = {{Anything, "PH",Anything, "22"},
{Anything, "PEOP",Anything, "16016"},
{Anything, "POW",Anything, "1614"},
{Anything, "PUT",Nothing, "16818"},
{Anything, "P",Anything, "16"},
{Anything, 0, Anything, Silent}};

static Rule S_rules[] = {{Anything, "SH",Anything, "28"},
{"#", "SION",Anything, "291132"},
{Anything, "SOME",Anything, "261231"},
{"#", "SUR","#", "2910"},
{Anything, "SUR","#", "2810"},
{"#", "SU","#", "299"},
{"#", "SSU","#", "289"},
{"#", "SED",Nothing, "2719"},
{"#", "S","#", "27"},
{Anything, "SAID",Anything, "26319"},
{"^", "SION",Anything, "281132"},
{Anything, "S","S", ""},
{".", "S",Nothing, "27"},
{"#:.E", "S",Nothing, "27"},
{"#:^##", "S",Nothing, "27"},
{"#:^#", "S",Nothing, "26"},
{"U", "S",Nothing, "26"},
{" :#", "S",Nothing, "27"},
{Nothing, "SCH",Anything, "2620"},
{Anything, "S","C+", ""},
{"#", "SM",Anything, "2731"},
{"#", "SN","'", "271132"},
{Anything, "S",Anything, "26"},
{Anything, 0, Anything, Silent}};

static Rule V_rules[] = {{Anything, "VIEW",Anything, "23369"},
{Anything, "V",Anything, "23"},
{Anything, 0, Anything, Silent}};

static Rule Z_rules[] = {{Anything, "Z",Anything, "27"},
{Anything, 0, Anything, Silent}};

static Rule punct_rules[] = {{Anything, " ",Anything, "41"},
{Anything, "--",Anything, "41"},
{Anything, "-",Anything, ""},
{".", "'S",Anything, "27"},
{"#:.E", "'S",Anything, "27"},
{"#", "'S",Anything, "27"},
{Anything, "'",Anything, ""},
{Anything, ",",Anything, "41"},
{Anything, ".",Anything, "41"},
{Anything, "?",Anything, "41"},
{Anything, "!",Anything, "41"},
{Anything, "@",Anything, "418"},
{Anything, 0, Anything, Silent}};

static Rule G_rules[] = {{Anything, "GIV",Anything, "21123"},
{Nothing, "G","I^", "21"},
{Anything, "GE","T", "213"},
{"SU", "GGES",Anything, "2139326"},
{Anything, "GG",Anything, "21"},
{" B#", "G",Anything, "21"},
{Anything, "G","+", "39"},
{Anything, "GREAT",Anything, "2137218"},
{"#", "GH",Anything, ""},
{Anything, "G",Anything, "21"},
{Anything, 0, Anything, Silent}};

static Rule U_rules[] = {{Nothing, "UN","I", "36932"},
{Nothing, "UN",Anything, "1232"},
{Nothing, "UPON",Anything, "1116632"},
{"T", "UR","#", "837"},
{"S", "UR","#", "837"},
{"R", "UR","#", "837"},
{"D", "UR","#", "837"},
{"L", "UR","#", "837"},
{"Z", "UR","#", "837"},
{"N", "UR","#", "837"},
{"J", "UR","#", "837"},
{"TH", "UR","#", "837"},
{"CH", "UR","#", "837"},
{"SH", "UR","#", "837"},
{Anything, "UR","#", "36837"},
{Anything, "UR",Anything, "10"},
{Anything, "U","^ ", "12"},
{Anything, "U","^^", "12"},
{Anything, "UY",Anything, "13"},
{" G", "U","#", ""},
{"G", "U","%", ""},
{"G", "U","#", "35"},
{"#N", "U",Anything, "369"},
{"T", "U",Anything, "9"},
{"S", "U",Anything, "9"},
{"R", "U",Anything, "9"},
{"D", "U",Anything, "9"},
{"L", "U",Anything, "9"},
{"Z", "U",Anything, "9"},
{"N", "U",Anything, "9"},
{"J", "U",Anything, "9"},
{"TH", "U",Anything, "9"},
{"CH", "U",Anything, "9"},
{"SH", "U",Anything, "9"},
{Anything, "U",Anything, "369"},
{Anything, 0, Anything, Silent}};

static Rule T_rules[] = {{Nothing, "THE",Nothing, "2511"},
{Anything, "TO",Nothing, "189"},
{Anything, "THAT",Nothing, "25418"},
{Nothing, "THIS",Nothing, "25126"},
{Nothing, "THEY",Anything, "252"},
{Nothing, "THERE",Anything, "25337"},
{Anything, "THER",Anything, "2510"},
{Anything, "THEIR",Anything, "25337"},
{Nothing, "THAN",Nothing, "25432"},
{Nothing, "THEM",Nothing, "25331"},
{Anything, "THESE",Nothing, "25027"},
{Nothing, "THEN",Anything, "25332"},
{Anything, "THROUGH",Anything, "24379"},
{Anything, "THOSE",Anything, "25727"},
{Anything, "THOUGH",Nothing, "257"},
{Nothing, "THUS",Anything, "251226"},
{Anything, "TH",Anything, "24"},
{"#:", "TED",Nothing, "18119"},
{"S", "TI","#N", "38"},
{Anything, "TI","O", "28"},
{Anything, "TI","A", "28"},
{Anything, "TIEN",Anything, "281132"},
{Anything, "TUR","#", "3810"},
{Anything, "TU","A", "389"},
{Nothing, "TWO",Anything, "189"},
{Anything, "T",Anything, "18"},
{Anything, 0, Anything, Silent}};

static Rule F_rules[] = {{Anything, "FUL",Anything, "22834"},
{Anything, "F",Anything, "22"},
{Anything, 0, Anything, Silent}};

static Rule I_rules[] = {{Nothing, "IN",Anything, "132"},
{Nothing, "I",Nothing, "13"},
{Anything, "IN","D", "1332"},
{Anything, "IER",Anything, "010"},
{"#:R", "IED",Anything, "019"},
{Anything, "IED",Nothing, "1319"},
{Anything, "IEN",Anything, "0332"},
{Anything, "IE","T", "133"},
{" :", "I","%", "13"},
{Anything, "I","%", "0"},
{Anything, "IE",Anything, "0"},
{Anything, "I","^+:#", "1"},
{Anything, "IR","#", "1337"},
{Anything, "IZ","%", "1327"},
{Anything, "IS","%", "1327"},
{Anything, "I","D%", "13"},
{"+^", "I","^+", "1"},
{Anything, "I","T%", "13"},
{"#:^", "I","^+", "1"},
{Anything, "I","^+", "13"},
{Anything, "IR",Anything, "10"},
{Anything, "IGH",Anything, "13"},
{Anything, "ILD",Anything, "133419"},
{Anything, "IGN",Nothing, "1332"},
{Anything, "IGN","^", "1332"},
{Anything, "IGN","%", "1332"},
{Anything, "IQUE",Anything, "020"},
{Anything, "I",Anything, "1"},
{Anything, 0, Anything, Silent}};

static Rule J_rules[] = {{Anything, "J",Anything, "39"},
{Anything, 0, Anything, Silent}};

static Rule B_rules[] = {{Nothing, "BE","^#", "171"},
{Anything, "BEING",Anything, "170133"},
{Nothing, "BOTH",Nothing, "17724"},
{Nothing, "BUS","#", "17127"},
{Anything, "BUIL",Anything, "17134"},
{Anything, "B",Anything, "17"},
{Anything, 0, Anything, Silent}};

static Rule Y_rules[] = {{Anything, "YOUNG",Anything, "361233"},
{Nothing, "YOU",Anything, "369"},
{Nothing, "YES",Anything, "36326"},
{Nothing, "Y",Anything, "36"},
{"#:^", "Y",Nothing, "0"},
{"#:^", "Y","I", "0"},
{" :", "Y",Nothing, "13"},
{" :", "Y","#", "13"},
{" :", "Y","^+:#", "1"},
{" :", "Y","^#", "13"},
{Anything, "Y",Anything, "1"},
{Anything, 0, Anything, Silent}};

static Rule O_rules[] = {{Anything, "OF",Nothing, "1123"},
{Anything, "OROUGH",Anything, "107"},
{"#:", "OR",Nothing, "10"},
{"#:", "ORS",Nothing, "1027"},
{Anything, "OR",Anything, "637"},
{Nothing, "ONE",Anything, "351232"},
{Nothing, "OW",Anything, "7"},
{Anything, "OW",Anything, "7"},
{Nothing, "OVER",Anything, "72310"},
{Anything, "OV",Anything, "1223"},
{Anything, "O","^%", "7"},
{Anything, "O","^EN", "7"},
{Anything, "O","^I#", "7"},
{Anything, "OL","D", "734"},
{Anything, "OUGHT",Anything, "618"},
{Anything, "OUGH",Anything, "1222"},
{Nothing, "OU",Anything, "14"},
{"H", "OU","S#", "14"},
{Anything, "OUS",Anything, "1126"},
{Anything, "OUR",Anything, "637"},
{Anything, "OULD",Anything, "819"},
{"^", "OU","^L", "12"},
{Anything, "OUP",Anything, "916"},
{Anything, "OU",Anything, "14"},
{Anything, "OY",Anything, "15"},
{Anything, "OING",Anything, "7133"},
{Anything, "OI",Anything, "15"},
{Anything, "OOR",Anything, "637"},
{Anything, "OOK",Anything, "820"},
{Anything, "OOD",Anything, "819"},
{Anything, "OO",Anything, "9"},
{Anything, "O","E", "7"},
{Anything, "O",Nothing, "7"},
{Anything, "OA",Anything, "7"},
{Nothing, "ONLY",Anything, "732340"},
{Nothing, "ONCE",Anything, "35123226"},
{Anything, "ON'T",Anything, "73218"},
{"C", "O","N", "5"},
{Anything, "O","NG", "6"},
{" :^", "O","N", "12"},
{"I", "ON",Anything, "1132"},
{"#:", "ON",Nothing, "1132"},
{"#^", "ON",Anything, "1132"},
{Anything, "O","ST ", "7"},
{Anything, "OF","^", "622"},
{Anything, "OTHER",Anything, "122510"},
{Anything, "OSS",Nothing, "626"},
{"#:^", "OM",Anything, "1231"},
{Anything, "O",Anything, "5"},
{Anything, 0, Anything, Silent}};

static Rule E_rules[] = {{"#:", "E",Nothing, ""},
{"':^", "E",Nothing, ""},
{" :", "E",Nothing, "0"},
{"#", "ED",Nothing, "19"},
{"#:", "E","D ", ""},
{Anything, "EV","ER", "323"},
{Anything, "E","^%", "0"},
{Anything, "ERI","#", "0370"},
{Anything, "ERI",Anything, "3371"},
{"#:", "ER","#", "10"},
{Anything, "ER","#", "337"},
{Anything, "ER",Anything, "10"},
{Nothing, "EVEN",Anything, "023332"},
{"#:", "E","W", ""},
{"T", "EW",Anything, "9"},
{"S", "EW",Anything, "9"},
{"R", "EW",Anything, "9"},
{"D", "EW",Anything, "9"},
{"L", "EW",Anything, "9"},
{"Z", "EW",Anything, "9"},
{"N", "EW",Anything, "9"},
{"J", "EW",Anything, "9"},
{"TH", "EW",Anything, "9"},
{"CH", "EW",Anything, "9"},
{"SH", "EW",Anything, "9"},
{Anything, "EW",Anything, "369"},
{Anything, "E","O", "0"},
{"#:S", "ES",Nothing, "127"},
{"#:C", "ES",Nothing, "127"},
{"#:G", "ES",Nothing, "127"},
{"#:Z", "ES",Nothing, "127"},
{"#:X", "ES",Nothing, "127"},
{"#:J", "ES",Nothing, "127"},
{"#:CH", "ES",Nothing, "127"},
{"#:SH", "ES",Nothing, "127"},
{"#:", "E","S ", ""},
{"#:", "ELY",Nothing, "340"},
{"#:", "EMENT",Anything, "3133218"},
{Anything, "EFUL",Anything, "22834"},
{Anything, "EE",Anything, "0"},
{Anything, "EARN",Anything, "1032"},
{Nothing, "EAR","^", "10"},
{Anything, "EAD",Anything, "319"},
{"#:", "EA",Nothing, "011"},
{Anything, "EA","SU", "3"},
{Anything, "EA",Anything, "0"},
{Anything, "EIGH",Anything, "2"},
{Anything, "EI",Anything, "0"},
{Nothing, "EYE",Anything, "13"},
{Anything, "EY",Anything, "0"},
{Anything, "EU",Anything, "369"},
{Anything, "E",Anything, "3"},
{Anything, 0, Anything, Silent}};

static Rule L_rules[] = {{Anything, "LO","C#", "347"},
{"L", "L",Anything, ""},
{"#:^", "L","%", "1134"},
{Anything, "LEAD",Anything, "34019"},
{Anything, "L",Anything, "34"},
{Anything, 0, Anything, Silent}};

static Rule H_rules[] = {{Nothing, "HAV",Anything, "30423"},
{Nothing, "HERE",Anything, "30037"},
{Nothing, "HOUR",Anything, "1410"},
{Anything, "HOW",Anything, "3014"},
{Anything, "H","#", "30"},
{Anything, "H",Anything, ""},
{Anything, 0, Anything, Silent}};

static Rule Q_rules[] = {{Anything, "QUAR",Anything, "2035637"},
{Anything, "QU",Anything, "2035"},
{Anything, "Q",Anything, "20"},
{Anything, 0, Anything, Silent}};

static Rule W_rules[] = {{Nothing, "WERE",Anything, "3510"},
{Anything, "WA","S", "355"},
{Anything, "WA","T", "355"},
{Anything, "WHERE",Anything, "40337"},
{Anything, "WHAT",Anything, "40518"},
{Anything, "WHOL",Anything, "30734"},
{Anything, "WHO",Anything, "309"},
{Anything, "WH",Anything, "40"},
{Anything, "WAR",Anything, "35637"},
{Anything, "WOR","^", "3510"},
{Anything, "WR",Anything, "37"},
{Anything, "W",Anything, "35"},
{Anything, 0, Anything, Silent}};

static Rule R_rules[] = {{Nothing, "RE","^#", "370"},
{Anything, "R",Anything, "37"},
{Anything, 0, Anything, Silent}};

static Rule C_rules[] = {{Nothing, "CH","^", "20"},
{"^E", "CH",Anything, "20"},
{Anything, "CH",Anything, "38"},
{" S", "CI","#", "2613"},
{Anything, "CI","A", "28"},
{Anything, "CI","O", "28"},
{Anything, "CI","EN", "28"},
{Anything, "C","+", "26"},
{Anything, "CK",Anything, "20"},
{Anything, "COM","%", "201231"},
{Anything, "C",Anything, "20"},
{Anything, 0, Anything, Silent}};

static Rule X_rules[] = {{Anything, "X",Anything, "2026"},
{Anything, 0, Anything, Silent}};

static Rule A_rules[] = {{Anything, "A",Nothing, "11"},
{Nothing, "ARE",Nothing, "537"},
{Nothing, "AR","O", "1137"},
{Anything, "AR","#", "337"},
{"^", "AS","#", "226"},
{Anything, "A","WA", "11"},
{Anything, "AW",Anything, "6"},
{" :", "ANY",Anything, "3320"},
{Anything, "A","^+#", "2"},
{"#:", "ALLY",Anything, "11340"},
{Nothing, "AL","#", "1134"},
{Anything, "AGAIN",Anything, "1121332"},
{"#:", "AG","E", "139"},
{Anything, "A","^+:#", "4"},
{" :", "A","^+ ", "2"},
{Anything, "A","^%", "2"},
{Nothing, "ARR",Anything, "1137"},
{Anything, "ARR",Anything, "437"},
{" :", "AR",Nothing, "537"},
{Anything, "AR",Nothing, "10"},
{Anything, "AR",Anything, "537"},
{Anything, "AIR",Anything, "337"},
{Anything, "AI",Anything, "2"},
{Anything, "AY",Anything, "2"},
{Anything, "AU",Anything, "6"},
{"#:", "AL",Nothing, "1134"},
{"#:", "ALS",Nothing, "113427"},
{Anything, "ALK",Anything, "620"},
{Anything, "AL","^", "634"},
{" :", "ABLE",Anything, "2171134"},
{Anything, "ABLE",Anything, "11171134"},
{Anything, "ANG","+", "23239"},
{Anything, "A",Anything, "4"},
{Anything, 0, Anything, Silent}};

static Rule M_rules[] = {{Anything, "MOV",Anything, "31923"},
{Anything, "M",Anything, "31"},
{Anything, 0, Anything, Silent}};

Rule *Rules[] =
	{
	punct_rules,
	A_rules, B_rules, C_rules, D_rules, E_rules, F_rules, G_rules, 
	H_rules, I_rules, J_rules, K_rules, L_rules, M_rules, N_rules, 
	O_rules, P_rules, Q_rules, R_rules, S_rules, T_rules, U_rules, 
	V_rules, W_rules, X_rules, Y_rules, Z_rules
	};


#include <stdio.h>
#include <string.h>

char* s1 = "Kill Worluk for double score.";

//	"PAH1I1R", "TRAH2I1Y", "SU1PRE1N", "AWL", "HA2AYL", 
//	"EH1MPAH1I1R",


enum Phonemes {	EH3, EH2, EH1, PA0, DT, A1,  A2,  ZH, 	AH2,    I3,  I2,  I1,  M,  N,   B,   V,	CH,     SH,  Z,   AW1, NG, AH1, OO1, OO,	L,      K,   J,   H,   G,  F,   D,   S,	A,      AY,  Y1,  UH3, AH, P,   O,   I,	U,      Y,   T,   R,   E,  W,   AE,  AE1,	AW2,    UH2, UH1, UH,  O2, O1,  IU,  U1,	THV,    TH,  ER,  EH,  E1, AW,  PA1, STOP};

unsigned char END=0xFF;

enum Intonations {I_1 =0x40,   I_2 =0x80,   I_3 =0xC0};

int pp=2;

static const char *const GorfWordTable[] =
{
	"A2AYY1", "A2E1", "UH1GEH1I3N", "AE1EH2M", "AEM", 
	"AE1EH3ND", "UH1NAH2I1YLA2SHUH2N", "AH2NUHTHER", "AH1NUHTHVRR", 
	"AH1R", "UHR", "UH1VEH1EH3NNDJER", "BAEEH3D", "BAEEH1D", "BE", 
	"BEH3EH1N", "BUH1DTTEH2NN", "KUHDEH2T", 
	"KAE1NUH1T", "KAE1EH3PTI3N", 
	"KRAH2UH3NI3KUH3O2LZ", "KO1UH3I3E1N", "KO1UH3I3E1NS", 
	"KERNAH2L", "KAH1NCHEHSNEHS", "DE1FEH1NDER", 
	"DE1STRO1I1Y", "DE1STRO1I1Y1D", 
	"DU1UM", "DRAW1S", "EHMPAH2I3YR", "EHND", 
	"EH1NEH1MY", "EH1SKA1E1P", "FLEHGSHIP", 
	"FOR", "GUH1LAEKTI1K", 
	"DJEH2NERUH3L", "GDTO1O1RRFF", "GDTO1RFYA2N", "GDTO1RFE1EH2N", "GDTO1RFYA2NS", 
	"HAH1HAH1HAH1HAH1", "HUHRDER", 
	"HAE1EH3V", "HI1TI1NG", "AH1I1Y",  "AH1I1Y1", "I1MPAH1SI1BL", 
	"IN*", "INSERT", "I1S", "LI1V", "LAWNG", "MEE1T", "MUU1V", 
	"MAH2I1Y", "MAH2I3Y", "NIR", "NEHKST", "NUH3AH2YS", "NO", 
	"NAH1O1U1W", "PA1", "PLA1AYER", "PRE1PAE1ER", "PRI1SI3NEH3RS", 
	"PRUH2MOTEH3D", "POO1IUSH", "RO1U1BAH1T", "RO1U1BAH1TS", 
	"RO1U1BAH1UH3TS", "SEK",  "SHIP", "SHAH1UH3T", "SUHM", "SPA2I3YS", "PA0", 
	"SERVAH2I1Y1VUH3L", "TAK", "THVUH", "THVUH1", 
	"THUH", "TAH1EH3YM", "TU", "TIUU1", 
	"UH2NBE1AYTUH3BUH3L", 
	"WORAYY1EH3R", "WORAYY1EH3RS", "WI1L", 
	"Y1I3U1", "YIUU1U1", "YI1U1U1", "Y1IUU1U1", "Y1I1U1U1", "YOR", "YU1O1RSEH1LF", 
	"FO1R", "FO2R", "WIL", "GDTO1RVYA2N", 
	"KO1UH3I3AYNN", 
	"UH1TAEEH3K", "BAH2I3Y1T", "KAH1NKER", "DYVAH1U1ER", "DUHST", "GAE1LUH1KSY", "GAH1EH3T", 
	"PAH1I1R", "TRAH2I1Y", "SU1PRE1N", "AWL", "HA2AYL", 
	"EH1MPAH1I1R",
0
};

static const char *const PhonemeTable[65] =
{
	"EH3","EH2","EH1","PA0","DT" ,"A1" ,"A2" ,"ZH",
	"AH2","I3" ,"I2" ,"I1" ,"M"  ,"N"  ,"B"  ,"V",
	"CH" ,"SH" ,"Z"  ,"AW1","NG" ,"AH1","OO1","OO",
	"L"  ,"K"  ,"J"  ,"H"  ,"G"  ,"F"  ,"D"  ,"S",
	"A"  ,"AY" ,"Y1" ,"UH3","AH" ,"P"  ,"O"  ,"I",
	"U"  ,"Y"  ,"T"  ,"R"  ,"E"  ,"W"  ,"AE" ,"AE1",
	"AW2","UH2","UH1","UH" ,"O2" ,"O1" ,"IU" ,"U1",
	"THV","TH" ,"ER" ,"EH" ,"E1" ,"AW" ,"PA1","STOP",
	0
};


void main(){
unsigned char p1[] = {I_2|PA0, K, I, I3, L, PA0, W, O, O1, R, L, UH, K, PA0, F, O1, R, D, UH, B, UH3, L, S, K, O, O1, R, PA1, I_2|PA0, END};
 int x;

 char* s26 = "While you developed science, we developed magic.";
unsigned char p26[45] = {W, AH1, EH3, I3, L, Y1, I_1|IU, I_1|U, D, E1, V, EH1, L, UH3, P, T, S, AH1, I3, AY, EH, N, S, PA1, W, E1, Y, D, E1, V, EH1, L, UH3, P, T, M, AE1, EH3, D, J, I1, K, PA1, END};

 // for (x=0;x<45;x++){
   //   printf("0x%X ,",p26[x]);
//	  }

//RUN through GorfWordTable look for and print matches:

 for (pp=0;pp<115;pp++){
 int z=0;
 printf("const unsigned char gorf%d[]         __attribute__ ((section (\".flash\")))  = {",pp);
 while(z<strlen(GorfWordTable[pp])){
   int longest=0, longestx=0;
 for (x=0;x<64;x++){
    //    if (strcmp
    if ((strstr(GorfWordTable[pp]+z,PhonemeTable[x])!=NULL) && strstr(GorfWordTable[pp]+z,PhonemeTable[x])==GorfWordTable[pp]+z) {
      // is it the longest
      if (strlen(PhonemeTable[x])>longest) { longestx=x; longest=strlen(PhonemeTable[x]);}
    }
 }
      printf("%d, ",longestx);
      z+=strlen(PhonemeTable[longestx]);
      //      if (z>=strlen(GorfWordTable[pp])) break;
 }
 printf("};\n");
}
}

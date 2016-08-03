#!/usr/bin/env python
#
# retroSpeak project
#
# English to Phoneme text to speech
# Python version of Naval Research Laboratory algorithm rules list
# described in NRL Report 7948 January 21st, 1976 Elovitz et al
# http://www.dtic.mil/cgi-bin/GetTRDoc?AD=ADA021929
# Adapted from the public domain C version by Wasser, 1985
# Retrieved from: ftp://svr-ftp.eng.cam.ac.uk/pub/comp.speech/synthesis/english2phoneme.tar.gz
#
# As this is based heavily on Public Domain work, this code is relased as public domain.
#
# 
# Outputs a form of International Pronounciation Alphabet (IPA) which needs mapping
# to the allophone set used by the SP0256-AL2 in the retroSpeak project
#
# This is a rule set for US English pronounciation
#
#   The Phoneme codes:
# 
#           IY      bEEt            IH      bIt
#           EY      gAte            EH      gEt
#           AE      fAt             AA      fAther
#           AO      lAWn            OW      lOne
#           UH      fUll            UW      fOOl
#           ER      mURdER          AX      About
#           AH      bUt             AY      hIde
#           AW      hOW             OY      tOY
#           p       Pack            b       Back
#           t       Time            d       Dime
#           k       Coat            g       Goat
#           f       Fault           v       Vault
#           TH      eTHer           DH      eiTHer
#           s       Sue             z       Zoo
#           SH      leaSH           ZH      leiSure
#           h       How             m       suM
#           n       suN             NG      suNG
#           l       Laugh           w       Wear
#           y       Young           r       Rate
#           CH      CHar            j       Jar
#           WH      WHere
#
#
#   Rules are made up of four parts:
#   
#           The left context.
#           The text to match.
#           The right context.
#           The phonemes to substitute for the matched text.
#
#   Procedure:
# 
#           Seperate each block of letters (apostrophes included) 
#           and add a space on each side.  For each unmatched 
#           letter in the word, look through the rules where the 
#           text to match starts with the letter in the word.  If 
#           the text to match is found and the right and left 
#           context patterns also match, output the phonemes for 
#           that rule and skip to the next unmatched letter.
# 
# 
#   Special Context Symbols:
# 
#           #       One or more vowels
#           :       Zero or more consonants
#           ^       One consonant.
#           .       One of B, D, V, G, J, L, M, N, R, W or Z (voiced 
#                   consonants)
#           %       One of ER, E, ES, ED, ING, ELY (a suffix)
#                   (Found in right context only)
#           +       One of E, I or Y (a "front" vowel)
# 


# Context definitions
Anything = "" # No context requirement
Nothing = " " # Context is beginning or end of word 

# Phoneme definitions 
Pause  = "PAUSE" # Short silence
Silent = "" # No phonemes

#       Left part        Match Part      Right Part      Out Part

Rules = {
    'punctuation':[
        ( Anything,      " ",            Anything,       Pause   ),
        ( Anything,      "--",           Anything,       Pause   ),
        ( Anything,      "-",            Anything,       Silent  ),
        ( ".",           "'S",           Anything,       "z"     ),
        ( "#:.E",        "'S",           Anything,       "z"     ),
        ( "#",           "'S",           Anything,       "z"     ),
        ( Anything,      "'",            Anything,       Silent  ),
        ( Anything,      ",",            Anything,       Pause   ),
        ( Anything,      ".",            Anything,       Pause   ),
        ( Anything,      "?",            Anything,       Pause   ),
        ( Anything,      "!",            Anything,       Pause   ),
        ( Anything,      "@",            Anything,       "AE t"  )
        ],

#       Left part        Match Part      Right Part      Out Part
    'A':[
        ( Anything,      "A",            Nothing,        "AX"    ),
        ( Nothing,       "ARE",          Nothing,        "AA r"   ),
        ( Nothing,       "AR",           "O",            "AX r"   ),
        ( Anything,      "AR",           "#",            "EH r"   ),
        ( "^",           "AS",           "#",            "EY s"   ),
        ( Anything,      "A",            "WA",           "AX"    ),
        ( Anything,      "AW",           Anything,       "AO"    ),
        ( " :",          "ANY",          Anything,       "EH n IY" ),
        ( Anything,      "A",            "^+#",          "EY"    ),
        ( "#:",          "ALLY",         Anything,       "AX l IY" ),
        ( Nothing,       "AL",           "#",            "AX l"   ),
        ( Anything,      "AGAIN",        Anything,       "AX g EH n"),
        ( "#:",          "AG",           "E",            "IH j"   ),
        ( Anything,      "A",            "^+:#",         "AE"    ),
        ( " :",          "A",            "^+ ",          "EY"    ),
        ( Anything,      "A",            "^%",           "EY"    ),
        ( Nothing,       "ARR",          Anything,       "AX r"   ),
        ( Anything,      "ARR",          Anything,       "AE r"   ),
        ( " :",          "AR",           Nothing,        "AA r"   ),
        ( Anything,      "AR",           Nothing,        "ER"    ),
        ( Anything,      "AR",           Anything,       "AA r"   ),
        ( Anything,      "AIR",          Anything,       "EH r"   ),
        ( Anything,      "AI",           Anything,       "EY"    ),
        ( Anything,      "AY",           Anything,       "EY"    ),
        ( Anything,      "AU",           Anything,       "AO"    ),
        ( "#:",          "AL",           Nothing,        "AX l"   ),
        ( "#:",          "ALS",          Nothing,        "AX l z"  ),
        ( Anything,      "ALK",          Anything,       "AO k"   ),
        ( Anything,      "AL",           "^",            "AO l"   ),
        ( " :",          "ABLE",         Anything,       "EY b AX l"),
        ( Anything,      "ABLE",         Anything,       "AX b AX l"),
        ( Anything,      "ANG",          "+",            "EY n j"  ),
        ( Anything,      "A",            Anything,       "AE"    )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'B':[
        ( Nothing,       "BE",           "^#",           "b IH"   ),
        ( Anything,      "BEING",        Anything,       "b IY IH NG"),
        ( Nothing,       "BOTH",         Nothing,        "b OW TH" ),
        ( Nothing,       "BUS",          "#",            "b IH z"  ),
        ( Anything,      "BUIL",         Anything,       "b IH l"  ),
        ( Anything,      "B",            Anything,       "b"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'C':[
        ( Nothing,       "CH",           "^",            "k"     ),
        ( "^E",          "CH",           Anything,       "k"     ),
        ( Anything,      "CH",           Anything,       "CH"    ),
        ( " S",          "CI",           "#",            "s AY"   ),
        ( Anything,      "CI",           "A",            "SH"    ),
        ( Anything,      "CI",           "O",            "SH"    ),
        ( Anything,      "CI",           "EN",           "SH"    ),
        ( Anything,      "C",            "+",            "s"     ),
        ( Anything,      "CK",           Anything,       "k"     ),
        ( Anything,      "COM",          "%",            "k AH m"  ),
        ( Anything,      "C",            Anything,       "k"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'D':[
        ( "#:",          "DED",          Nothing,        "d IH d"  ),
        ( ".E",          "D",            Nothing,        "d"     ),
        ( "#:^E",        "D",            Nothing,        "t"     ),
        ( Nothing,       "DE",           "^#",           "d IH"   ),
        ( Nothing,       "DO",           Nothing,        "d UW"   ),
        ( Nothing,       "DOES",         Anything,       "d AH z"  ),
        ( Nothing,       "DOING",        Anything,       "d UW IH NG"),
        ( Nothing,       "DOW",          Anything,       "d AW"   ),
        ( Anything,      "DU",           "A",            "j UW"   ),
        ( Anything,      "D",            Anything,       "d"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'E':[
        ( "#:",          "E",            Nothing,        Silent  ),
        ( "':^",         "E",            Nothing,        Silent  ),
        ( " :",          "E",            Nothing,        "IY"    ),
        ( "#",           "ED",           Nothing,        "d"     ),
        ( "#:",          "E",            "D ",           Silent  ),
        ( Anything,      "EV",           "ER",           "EH v"   ),
        ( Anything,      "E",            "^%",           "IY"    ),
        ( Anything,      "ERI",          "#",            "IY r IY" ),
        ( Anything,      "ERI",          Anything,       "EH r IH" ),
        ( "#:",          "ER",           "#",            "ER"    ),
        ( Anything,      "ER",           "#",            "EH r"   ),
        ( Anything,      "ER",           Anything,       "ER"    ),
        ( Nothing,       "EVEN",         Anything,       "IY v EH n"),
        ( "#:",          "E",            "W",            Silent  ),
        ( "T",           "EW",           Anything,       "UW"    ),
        ( "S",           "EW",           Anything,       "UW"    ),
        ( "R",           "EW",           Anything,       "UW"    ),
        ( "D",           "EW",           Anything,       "UW"    ),
        ( "L",           "EW",           Anything,       "UW"    ),
        ( "Z",           "EW",           Anything,       "UW"    ),
        ( "N",           "EW",           Anything,       "UW"    ),
        ( "J",           "EW",           Anything,       "UW"    ),
        ( "TH",          "EW",           Anything,       "UW"    ),
        ( "CH",          "EW",           Anything,       "UW"    ),
        ( "SH",          "EW",           Anything,       "UW"    ),
        ( Anything,      "EW",           Anything,       "y UW"   ),
        ( Anything,      "E",            "O",            "IY"    ),
        ( "#:S",         "ES",           Nothing,        "IH z"   ),
        ( "#:C",         "ES",           Nothing,        "IH z"   ),
        ( "#:G",         "ES",           Nothing,        "IH z"   ),
        ( "#:Z",         "ES",           Nothing,        "IH z"   ),
        ( "#:X",         "ES",           Nothing,        "IH z"   ),
        ( "#:J",         "ES",           Nothing,        "IH z"   ),
        ( "#:CH",        "ES",           Nothing,        "IH z"   ),
        ( "#:SH",        "ES",           Nothing,        "IH z"   ),
        ( "#:",          "E",            "S ",           Silent  ),
        ( "#:",          "ELY",          Nothing,        "l IY"   ),
        ( "#:",          "EMENT",        Anything,       "m EH n t" ),
        ( Anything,      "EFUL",         Anything,       "f UH l"  ),
        ( Anything,      "EE",           Anything,       "IY"    ),
        ( Anything,      "EARN",         Anything,       "ER n"   ),
        ( Nothing,       "EAR",          "^",            "ER"    ),
        ( Anything,      "EAD",          Anything,       "EH d"   ),
        ( "#:",          "EA",           Nothing,        "IY AX"  ),
        ( Anything,      "EA",           "SU",           "EH"    ),
        ( Anything,      "EA",           Anything,       "IY"    ),
        ( Anything,      "EIGH",         Anything,       "EY"    ),
        ( Anything,      "EI",           Anything,       "IY"    ),
        ( Nothing,       "EYE",          Anything,       "AY"    ),
        ( Anything,      "EY",           Anything,       "IY"    ),
        ( Anything,      "EU",           Anything,       "y UW"   ),
        ( Anything,      "E",            Anything,       "EH"    )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'F':[
        ( Anything,      "FUL",          Anything,       "f UH l"  ),
        ( Anything,      "F",            Anything,       "f"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'G':[
        ( Anything,      "GIV",          Anything,       "g IH v"  ),
        ( Nothing,       "G",            "I^",           "g"     ),
        ( Anything,      "GE",           "T",            "g EH"   ),
        ( "SU",          "GGES",         Anything,       "g j EH s" ),
        ( Anything,      "GG",           Anything,       "g"     ),
        ( " B#",         "G",            Anything,       "g"     ),
        ( Anything,      "G",            "+",            "j"     ),
        ( Anything,      "GREAT",        Anything,       "g r EY t" ),
        ( "#",           "GH",           Anything,       Silent  ),
        ( Anything,      "G",            Anything,       "g"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'H':[
        ( Nothing,       "HAV",          Anything,       "h AE v"  ),
        ( Nothing,       "HERE",         Anything,       "h IY r"  ),
        ( Nothing,       "HOUR",         Anything,       "AW ER"  ),
        ( Anything,      "HOW",          Anything,       "h AW"   ),
        ( Anything,      "H",            "#",            "h"     ),
        ( Anything,      "H",            Anything,       Silent  )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'I':[
        ( Nothing,       "IN",           Anything,       "IH n"   ),
        ( Nothing,       "I",            Nothing,        "AY"    ),
        ( Anything,      "IN",           "D",            "AY n"   ),
        ( Anything,      "IER",          Anything,       "IY ER"  ),
        ( "#:R",         "IED",          Anything,       "IY d"   ),
        ( Anything,      "IED",          Nothing,        "AY d"   ),
        ( Anything,      "IEN",          Anything,       "IY EH n" ),
        ( Anything,      "IE",           "T",            "AY EH"  ),
        ( " :",          "I",            "%",            "AY"    ),
        ( Anything,      "I",            "%",            "IY"    ),
        ( Anything,      "IE",           Anything,       "IY"    ),
        ( Anything,      "I",            "^+:#",         "IH"    ),
        ( Anything,      "IR",           "#",            "AY r"   ),
        ( Anything,      "IZ",           "%",            "AY z"   ),
        ( Anything,      "IS",           "%",            "AY z"   ),
        ( Anything,      "I",            "D%",           "AY"    ),
        ( "+^",          "I",            "^+",           "IH"    ),
        ( Anything,      "I",            "T%",           "AY"    ),
        ( "#:^",         "I",            "^+",           "IH"    ),
        ( Anything,      "I",            "^+",           "AY"    ),
        ( Anything,      "IR",           Anything,       "ER"    ),
        ( Anything,      "IGH",          Anything,       "AY"    ),
        ( Anything,      "ILD",          Anything,       "AY l d"  ),
        ( Anything,      "IGN",          Nothing,        "AY n"   ),
        ( Anything,      "IGN",          "^",            "AY n"   ),
        ( Anything,      "IGN",          "%",            "AY n"   ),
        ( Anything,      "IQUE",         Anything,       "IY k"   ),
        ( Anything,      "I",            Anything,       "IH"    )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'J':[
        ( Anything,      "J",            Anything,       "j"     )       
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'K':[
        ( Nothing,       "K",            "N",            Silent  ),
        ( Anything,      "K",            Anything,       "k"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'L':[
        ( Anything,      "LO",           "C#",           "l OW"   ),
        ( "L",           "L",            Anything,       Silent  ),
        ( "#:^",         "L",            "%",            "AX l"   ),
        ( Anything,      "LEAD",         Anything,       "l IY d"  ),
        ( Anything,      "L",            Anything,       "l"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'M':[
        ( Anything,      "MOV",          Anything,       "m UW v"  ),
        ( Anything,      "M",            Anything,       "m"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'N':[
        ( "E",           "NG",           "+",            "n j"    ),
        ( Anything,      "NG",           "R",            "NG g"   ),
        ( Anything,      "NG",           "#",            "NG g"   ),
        ( Anything,      "NGL",          "%",            "NG g AX l"),
        ( Anything,      "NG",           Anything,       "NG"    ),
        ( Anything,      "NK",           Anything,       "NG k"   ),
        ( Nothing,       "NOW",          Nothing,        "n AW"   ),
        ( Anything,      "N",            Anything,       "n"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'O':[
        ( Anything,      "OF",           Nothing,        "AX v"   ),
        ( Anything,      "OROUGH",       Anything,       "ER OW"  ),
        ( "#:",          "OR",           Nothing,        "ER"    ),
        ( "#:",          "ORS",          Nothing,        "ER z"   ),
        ( Anything,      "OR",           Anything,       "AO r"   ),
        ( Nothing,       "ONE",          Anything,       "w AH n"  ),
        ( Nothing,       "OW",           Anything,       "OW"    ),
        ( Anything,      "OW",           Anything,       "OW"    ),
        ( Nothing,       "OVER",         Anything,       "OW v ER" ),
        ( Anything,      "OV",           Anything,       "AH v"   ),
        ( Anything,      "O",            "^%",           "OW"    ),
        ( Anything,      "O",            "^EN",          "OW"    ),
        ( Anything,      "O",            "^I#",          "OW"    ),
        ( Anything,      "OL",           "D",            "OW l"   ),
        ( Anything,      "OUGHT",        Anything,       "AO t"   ),
        ( Anything,      "OUGH",         Anything,       "AH f"   ),
        ( Nothing,       "OU",           Anything,       "AW"    ),
        ( "H",           "OU",           "S#",           "AW"    ),
        ( Anything,      "OUS",          Anything,       "AX s"   ),
        ( Anything,      "OUR",          Anything,       "AO r"   ),
        ( Anything,      "OULD",         Anything,       "UH d"   ),
        ( "^",           "OU",           "^L",           "AH"    ),
        ( Anything,      "OUP",          Anything,       "UW p"   ),
        ( Anything,      "OU",           Anything,       "AW"    ),
        ( Anything,      "OY",           Anything,       "OY"    ),
        ( Anything,      "OING",         Anything,       "OW IH NG"),
        ( Anything,      "OI",           Anything,       "OY"    ),
        ( Anything,      "OOR",          Anything,       "AO r"   ),
        ( Anything,      "OOK",          Anything,       "UH k"   ),
        ( Anything,      "OOD",          Anything,       "UH d"   ),
        ( Anything,      "OO",           Anything,       "UW"    ),
        ( Anything,      "O",            "E",            "OW"    ),
        ( Anything,      "O",            Nothing,        "OW"    ),
        ( Anything,      "OA",           Anything,       "OW"    ),
        ( Nothing,       "ONLY",         Anything,       "OW n l IY"),
        ( Nothing,       "ONCE",         Anything,       "w AH n s" ),
        ( Anything,      "ON'T",         Anything,       "OW n t"  ),
        ( "C",           "O",            "N",            "AA"    ),
        ( Anything,      "O",            "NG",           "AO"    ),
        ( " :^",         "O",            "N",            "AH"    ),
        ( "I",           "ON",           Anything,       "AX n"   ),
        ( "#:",          "ON",           Nothing,        "AX n"   ),
        ( "#^",          "ON",           Anything,       "AX n"   ),
        ( Anything,      "O",            "ST ",          "OW"    ),
        ( Anything,      "OF",           "^",            "AO f"   ),
        ( Anything,      "OTHER",        Anything,       "AH DH ER"),
        ( Anything,      "OSS",          Nothing,        "AO s"   ),
        ( "#:^",         "OM",           Anything,       "AH m"   ),
        ( Anything,      "O",            Anything,       "AA"    )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART

    'P':[
        ( Anything,      "PH",           Anything,       "f"     ),
        ( Anything,      "PEOP",         Anything,       "p IY p"  ),
        ( Anything,      "POW",          Anything,       "p AW"   ),
        ( Anything,      "PUT",          Nothing,        "p UH t"  ),
        ( Anything,      "P",            Anything,       "p"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'Q':[
        ( Anything,      "QUAR",         Anything,       "k w AO r" ),
        ( Anything,      "QU",           Anything,       "k w"    ),
        ( Anything,      "Q",            Anything,       "k"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'R':[
        ( Nothing,       "RE",           "^#",           "r IY"   ),
        ( Anything,      "R",            Anything,       "r"     ),
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'S':[
        ( Anything,      "SH",           Anything,       "SH"    ),
        ( "#",           "SION",         Anything,       "ZH AX n" ),
        ( Anything,      "SOME",         Anything,       "s AH m"  ),
        ( "#",           "SUR",          "#",            "ZH ER"  ),
        ( Anything,      "SUR",          "#",            "SH ER"  ),
        ( "#",           "SU",           "#",            "ZH UW"  ),
        ( "#",           "SSU",          "#",            "SH UW"  ),
        ( "#",           "SED",          Nothing,        "z d"    ),
        ( "#",           "S",            "#",            "z"     ),
        ( Anything,      "SAID",         Anything,       "s EH d"  ),
        ( "^",           "SION",         Anything,       "SH AX n" ),
        ( Anything,      "S",            "S",            Silent  ),
        ( ".",           "S",            Nothing,        "z"     ),
        ( "#:.E",        "S",            Nothing,        "z"     ),
        ( "#:^##",       "S",            Nothing,        "z"     ),
        ( "#:^#",        "S",            Nothing,        "s"     ),
        ( "U",           "S",            Nothing,        "s"     ),
        ( " :#",         "S",            Nothing,        "z"     ),
        ( Nothing,       "SCH",          Anything,       "s k"    ),
        ( Anything,      "S",            "C+",           Silent  ),
        ( "#",           "SM",           Anything,       "z m"    ),
        ( "#",           "SN",           "'",            "z AX n"  ),
        ( Anything,      "S",            Anything,       "s"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'T':[
        ( Nothing,       "THE",          Nothing,        "DH AX"  ),
        ( Anything,      "TO",           Nothing,        "t UW"   ),
        ( Anything,      "THAT",         Nothing,        "DH AE t" ),
        ( Nothing,       "THIS",         Nothing,        "DH IH s" ),
        ( Nothing,       "THEY",         Anything,       "DH EY"  ),
        ( Nothing,       "THERE",        Anything,       "DH EH r" ),
        ( Anything,      "THER",         Anything,       "DH ER"  ),
        ( Anything,      "THEIR",        Anything,       "DH EH r" ),
        ( Nothing,       "THAN",         Nothing,        "DH AE n" ),
        ( Nothing,       "THEM",         Nothing,        "DH EH m" ),
        ( Anything,      "THESE",        Nothing,        "DH IY z" ),
        ( Nothing,       "THEN",         Anything,       "DH EH n" ),
        ( Anything,      "THROUGH",      Anything,       "TH r UW" ),
        ( Anything,      "THOSE",        Anything,       "DH OW z" ),
        ( Anything,      "THOUGH",       Nothing,        "DH OW"  ),
        ( Nothing,       "THUS",         Anything,       "DH AH s" ),
        ( Anything,      "TH",           Anything,       "TH"    ),
        ( "#:",          "TED",          Nothing,        "t IH d"  ),
        ( "S",           "TI",           "#N",           "CH"    ),
        ( Anything,      "TI",           "O",            "SH"    ),
        ( Anything,      "TI",           "A",            "SH"    ),
        ( Anything,      "TIEN",         Anything,       "SH AX n" ),
        ( Anything,      "TUR",          "#",            "CH ER"  ),
        ( Anything,      "TU",           "A",            "CH UW"  ),
        ( Nothing,       "TWO",          Anything,       "t UW"   ),
        ( Anything,      "T",            Anything,       "t"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'U':[
        ( Nothing,       "UN",           "I",            "y UW n"  ),
        ( Nothing,       "UN",           Anything,       "AH n"   ),
        ( Nothing,       "UPON",         Anything,       "AX p AO n"),
        ( "T",           "UR",           "#",            "UH r"   ),
        ( "S",           "UR",           "#",            "UH r"   ),
        ( "R",           "UR",           "#",            "UH r"   ),
        ( "D",           "UR",           "#",            "UH r"   ),
        ( "L",           "UR",           "#",            "UH r"   ),
        ( "Z",           "UR",           "#",            "UH r"   ),
        ( "N",           "UR",           "#",            "UH r"   ),
        ( "J",           "UR",           "#",            "UH r"   ),
        ( "TH",          "UR",           "#",            "UH r"   ),
        ( "CH",          "UR",           "#",            "UH r"   ),
        ( "SH",          "UR",           "#",            "UH r"   ),
        ( Anything,      "UR",           "#",            "y UH r"  ),
        ( Anything,      "UR",           Anything,       "ER"    ),
        ( Anything,      "U",            "^ ",           "AH"    ),
        ( Anything,      "U",            "^^",           "AH"    ),
        ( Anything,      "UY",           Anything,       "AY"    ),
        ( " G",          "U",            "#",            Silent  ),
        ( "G",           "U",            "%",            Silent  ),
        ( "G",           "U",            "#",            "w"     ),
        ( "#N",          "U",            Anything,       "y UW"   ),
        ( "T",           "U",            Anything,       "UW"    ),
        ( "S",           "U",            Anything,       "UW"    ),
        ( "R",           "U",            Anything,       "UW"    ),
        ( "D",           "U",            Anything,       "UW"    ),
        ( "L",           "U",            Anything,       "UW"    ),
        ( "Z",           "U",            Anything,       "UW"    ),
        ( "N",           "U",            Anything,       "UW"    ),
        ( "J",           "U",            Anything,       "UW"    ),
        ( "TH",          "U",            Anything,       "UW"    ),
        ( "CH",          "U",            Anything,       "UW"    ),
        ( "SH",          "U",            Anything,       "UW"    ),
        ( Anything,      "U",            Anything,       "y UW"  )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'V':[
        ( Anything,      "VIEW",         Anything,       "v y UW"  ),
        ( Anything,      "V",            Anything,       "v"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'W':[
        ( Nothing,       "WERE",         Anything,       "w ER"   ),
        ( Anything,      "WA",           "S",            "w AA"   ),
        ( Anything,      "WA",           "T",            "w AA"   ),
        ( Anything,      "WHERE",        Anything,       "WH EH r" ),
        ( Anything,      "WHAT",         Anything,       "WH AA t" ),
        ( Anything,      "WHOL",         Anything,       "h OW l"  ),
        ( Anything,      "WHO",          Anything,       "h UW"   ),
        ( Anything,      "WH",           Anything,       "WH"    ),
        ( Anything,      "WAR",          Anything,       "w AO r"  ),
        ( Anything,      "WOR",          "^",            "w ER"   ),
        ( Anything,      "WR",           Anything,       "r"     ),
        ( Anything,      "W",            Anything,       "w"     )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'X':[
        ( Anything,      "X",            Anything,       "k s"    )      
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'Y':[
        ( Anything,      "YOUNG",        Anything,       "y AH NG" ),
        ( Nothing,       "YOU",          Anything,       "y UW"   ),
        ( Nothing,       "YES",          Anything,       "y EH s"  ),
        ( Nothing,       "Y",            Anything,       "y"     ),
        ( "#:^",         "Y",            Nothing,        "IY"    ),
        ( "#:^",         "Y",            "I",            "IY"    ),
        ( " :",          "Y",            Nothing,        "AY"    ),
        ( " :",          "Y",            "#",            "AY"    ),
        ( " :",          "Y",            "^+:#",         "IH"    ),
        ( " :",          "Y",            "^#",           "AY"    ),
        ( Anything,      "Y",            Anything,       "IH"    )
        ],

#       LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
    'Z':[
        ( Anything,      "Z",            Anything,       "z"     )
        ] }

Ascii_codes = [
"nUWl ","stAArt AXv hEHdER ","stAArt AXv tEHkst ","EHnd AXv tEHkst ",
"EHnd AXv trAEnsmIHSHAXn",
"EHnkwAYr ","AEk ","bEHl ","bAEkspEYs ","tAEb ","lIHnIYfIYd ",
"vERtIHkAXl tAEb ","fAOrmfIYd ","kAErAYj rIYtERn ","SHIHft AWt ",
"SHIHft IHn ","dIHlIYt ","dIHvIHs kAAntrAAl wAHn ","dIHvIHs kAAntrAAl tUW ",
"dIHvIHs kAAntrAAl THrIY ","dIHvIHs kAAntrAAl fOWr ","nAEk ","sIHnk ",
"EHnd tEHkst blAAk ","kAEnsEHl ","EHnd AXv mEHsIHj ","sUWbstIHtUWt ",
"EHskEYp ","fAYEHld sIYpERAEtER ","grUWp sIYpERAEtER ","rIYkAOrd sIYpERAEtER ",
"yUWnIHt sIYpERAEtER ","spEYs ","EHksklAEmEYSHAXn mAArk ","dAHbl kwOWt ",
"nUWmbER sAYn ","dAAlER sAYn ","pERsEHnt ","AEmpERsAEnd ","kwOWt ",
"OWpEHn pEHrEHn ","klOWz pEHrEHn ","AEstEHrIHsk ","plAHs ","kAAmmAX ",
"mIHnAHs ","pIYrIYAAd ","slAESH ",

"zIHrOW ","wAHn ","tUW ","THrIY ","fOWr ",
"fAYv ","sIHks ","sEHvAXn ","EYt ","nAYn ",

"kAAlAXn ","sEHmIHkAAlAXn ","lEHs DHAEn ","EHkwAXl sAYn ","grEYtER DHAEn ",
"kwEHsCHAXn mAArk ","AEt sAYn ",

"EY ","bIY ","sIY ","dIY ","IY ","EHf ","jIY  ",
"EYtCH ","AY ","jEY ","kEY ","EHl ","EHm ","EHn ","AA ","pIY ",
"kw ","AAr ","EHz ","tIY ","AHw ","vIY ",
"dAHblyUWw ","EHks ","wAYIY ","zIY ",

"lEHft brAEkEHt ","bAEkslAESH ","rAYt brAEkEHt ","kAErEHt ",
"AHndERskAOr ","AEpAAstrAAfIH ",

"EY ","bIY ","sIY ","dIY ","IY ","EHf ","jIY  ",
"EYtCH ","AY ","jEY ","kEY ","EHl ","EHm ","EHn ","AA ","pIY ",
"kw ","AAr ","EHz ","tIY ","AHw ","vIY ",
"dAHblyUWw ","EHks ","wAYIY ","zIY ",

"lEHft brEYs ","vERtIHkAXl bAAr ","rAYt brEYs ","tAYld ","dEHl "
    ]

Cardinals = 	[
	"zIHrOW ",	"wAHn ",	"tUW ",		"THrIY ",
	"fOWr ",	"fAYv ",	"sIHks ",	"sEHvAXn ",
	"EYt ",		"nAYn ",		
	"tEHn ",	"IYlEHvAXn ",	"twEHlv ",	"THERtIYn ",
	"fOWrtIYn ",	"fIHftIYn ", 	"sIHkstIYn ",	"sEHvEHntIYn ",
	"EYtIYn ",	"nAYntIYn "
	]

Twenties =	[
	"twEHntIY ",	"THERtIY ",	"fAOrtIY ",	"fIHftIY ",
	"sIHkstIY ",	"sEHvEHntIY ",	"EYtIY ",	"nAYntIY "
	]

Ordinals =	[
	"zIHrOWEHTH ",	"fERst ",	"sEHkAHnd ",	"THERd ",
	"fOWrTH ",	"fIHfTH ",	"sIHksTH ",	"sEHvEHnTH ",
	"EYtTH ",	"nAYnTH ",		
	"tEHnTH ",	"IYlEHvEHnTH ",	"twEHlvTH ",	"THERtIYnTH ",
	"fAOrtIYnTH ",	"fIHftIYnTH ", 	"sIHkstIYnTH ",	"sEHvEHntIYnTH ",
	"EYtIYnTH ",	"nAYntIYnTH "
	]

Ord_twenties = 	[
	"twEHntIYEHTH ","THERtIYEHTH ",	"fOWrtIYEHTH ",	"fIHftIYEHTH ",
	"sIHkstIYEHTH ","sEHvEHntIYEHTH ","EYtIYEHTH ",	"nAYntIYEHTH "
	]

miscy = [
"mAYnAHs ", "IHnfIHnIHtIY ", "bIHlIYAXn ", "AEnd ", "mIHlIYAXn", "AEnd ", "THAWzAEnd ", "hAHndrEHd "
]

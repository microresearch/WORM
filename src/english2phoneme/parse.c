#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_LENGTH 128
#define MAX_PHONEME_LENGTH 3

static FILE *In_file;
static FILE *Out_file;

static int Char, Char1, Char2, Char3;
static int input_count;
static int input_length;
static char *input_array;
static char *output_array[MAX_LENGTH];

int output_count = 0;

/*
** main(argc, argv)
**	int argc;
**	char *argv[];
**
**	This is the main program. It translates each argument into an integer code,
**  where each integer maps to a phoneme. Populates an output array, where each
**  elem is an integer code for a corresponding phoneme in the input Returns the
**  number of phonemes in the input.
*/
main(argc, argv)
	int argc;
	char *argv[];
	{
 
  //allocate space for the output
  char output[MAX_LENGTH][MAX_PHONEME_LENGTH];
  for(int i = 0; i < MAX_LENGTH; i++){
    output_array[i] = output[i];
  }
  
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
  for(int i = 0; i < num_words; i++){
      char *word = argv[1 + i];
      for(int j = 0; j < strlen(word); j++){
        input[index] = word[j];
        index += 1;
      }
      input[index] = ' ';
      index += 1;
  }
  input[index] = EOF;
  
  //transform text to integer code phonemes
  int output_count = text2speech(input_size + space_count,input,output);
  for(int i = 0; i < output_count; i++){
      for(int j = 0; j < strlen(output_array[i]); j++){
         printf("%c", output_array[i][j]);
      }
      printf("\n");
  }
  return output_count;
	}

/*
** Transforms text to integer code phonemes.
*/
text2speech(int input_len, char *input){
  input_array = input;
  input_length = input_len;
  input_count = 0;
  xlate_file();
  return output_count;
}

/*
** Add one character (digit of integer) to output array
*/
outchar(int chr, int index){
  output_array[output_count][index] = chr;
}

/*
** Add space-delimited string of integers to output array,
** such that each integer is an element in the output array.
*/
outstring(string)
	char *string;
	{
	while (*string != '\0'){
    int index = 0;
    while(*string != ' '){
      outchar(*string++, index);
      index++;
    }
    output_array[output_count][index] = '\0';
    output_count++;
    *string++;
  }
	}


int makeupper(character)
	int character;
	{
	if (islower(character))
		return toupper(character);
	else
		return character;
	}

new_char()
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
xlate_file()
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
		else
		if (Char == '$' && isdigit(Char1))
			have_dollars();
		else
			have_special();
		}
	}

have_dollars()
	{
  
  fprintf(stderr, "Cannot read monetary values. Please enter alpha text.\n");
  exit(1);
  
	long int value;

	value = 0L;
	for (new_char() ; isdigit(Char) || Char == ',' ; new_char())
		{
		if (Char != ',')
			value = 10 * value + (Char-'0');
		}

	say_cardinal(value);	/* Say number of whole dollars */

	/* Found a character that is a non-digit and non-comma */

	/* Check for no decimal or no cents digits */
	if (Char != '.' || !isdigit(Char1))
		{
		if (value == 1L)
			outstring("dAAlER ");
		else
			outstring("dAAlAArz ");
		return;
		}

	/* We have '.' followed by a digit */

	new_char();	/* Skip the period */

	/* If it is ".dd " say as " DOLLARS AND n CENTS " */
	if (isdigit(Char1) && !isdigit(Char2))
		{
		if (value == 1L)
			outstring("dAAlER ");
		else
			outstring("dAAlAArz ");
		if (Char == '0' && Char1 == '0')
			{
			new_char();	/* Skip tens digit */
			new_char();	/* Skip units digit */
			return;
			}

		outstring("AAnd ");
		value = (Char-'0')*10 + Char1-'0';
		say_cardinal(value);

		if (value == 1L)
			outstring("sEHnt ");
		else
			outstring("sEHnts ");
		new_char();	/* Used Char (tens digit) */
		new_char();	/* Used Char1 (units digit) */
		return;
		}

	/* Otherwise say as "n POINT ddd DOLLARS " */

	outstring("pOYnt ");
	for ( ; isdigit(Char) ; new_char())
		{
		say_ascii(Char);
		}

	outstring("dAAlAArz ");

	return;
	}

have_special()
	{
	/*if (Char == '\n')
		outchar('\n');
	else*/
	if (!isspace(Char))
		say_ascii(Char);

	new_char();
	return;
	}


have_number()
	{
  fprintf(stderr, "Cannot read numerical values. Please enter alpha text.\n");
  exit(1);
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
		outstring("pOYnt ");
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


have_letter()
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
abbrev(buff)
	char buff[];
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

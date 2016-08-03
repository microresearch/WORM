from en_US_rules import miscy
import phoneme_map

#Using a map that maps phonemes to integer codes, converts the phoneme representation
#of ascii characters, as defined in en_US_rules.py, into integer code representations.

new_ascii = []

#Fills new_ascii with integer code representation of Ascii_codes
def replace_phonemes(phoneme_map):
	for elem in miscy:
		new_ascii.append(transform_string(elem))

#Given a string of phonemes, returns the integer code representaion of that string
#(return string is space delimited between phonemes)
def transform_string(phoneme_string):
	new_string = ""
	i = 0
	while i < len(phoneme_string):
		if phoneme_string[i].isupper():
			new_string += phoneme_map[phoneme_string[i:i+2]]
			i += 2
			new_string += ","
		elif phoneme_string[i] == " ":
			new_string += phoneme_map["PAUSE"]
			new_string += ",-1,"
			i += 1
		else:
			new_string += phoneme_map[phoneme_string[i]]
			new_string += ","
			i += 1
	return new_string

#Output transformed codes in C format. Output to be written to newasciifile.txt,
#and then copied over to spellwor.c in english2phoneme
def output_C_format():
	file=open("newasciifile.txt","w")
	output = "static char *Ascii[] = \n{\n"
        count=0
	for code in new_ascii:
                count=count+1
		output += "{"
		output += code
                output += "},"
	file.write(output)
        print output
        print count
	return output

phoneme_map = phoneme_map.make_map()
replace_phonemes(phoneme_map)
output_C_format()

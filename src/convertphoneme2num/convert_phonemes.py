
from en_US_rules import Rules
import phoneme_map

#Using a map that maps phonemes to integer codes, converts the phonemes in phoneme rules
#as defined in en_US_rules.py into integer codes. Outputs the rules in C code format to
#be used as the rules in english.c in english2phoneme.

new_rules = {}

#Convert phonemes in rules to integer codes corresponding
#to the phonemes
def replace_phonemes(phoneme_map):
	for rule in Rules:
		for i,elem in enumerate(Rules[rule]):
			to_transform = elem[3].split()
			transformed = ""
			for phoneme in to_transform:
				transformed += phoneme_map[phoneme]
				transformed += " "
			temp = list(elem)
			temp[3] = transformed
			elem = tuple(temp)
			Rules[rule][i]=elem

#Write elem to output in correct format
def output_helper(output, elem):
	if elem == "":
		output += "Anything"
	elif elem == " ":
		output += "Nothing"
	else:
		output += "\"" + elem + "\""
	return output

#Output transformed rules in C format. Output to be written to newfile.txt,
#and then copied over to english.c in english2phoneme
def output_C_format():
	file=open("newfile.txt","w")
	output = ""
	for rule in Rules:
		output += "static Rule "
		if rule == "punctuation":
			output += "punct"
		else:
			output += rule
		output += "_rules[] = {"
		for elem in Rules[rule]:
			output += "{"
			output = output_helper(output, elem[0])
			output += ", "
			output += "\"" + elem[1] + "\"" + ","
			output = output_helper(output, elem[2])
			output += ", "
			if elem == "":
				output += "Silent"
			elif elem == " ":
				output += "Pause"
			else:
				output += "\"" + elem[3] + "\""
			output += "},\n"
		output += "{Anything, 0, Anything, Silent}};\n\n"
	file.write(output)

phoneme_map = phoneme_map.make_map()
replace_phonemes(phoneme_map)
output_C_format()

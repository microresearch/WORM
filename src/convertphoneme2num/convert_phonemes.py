
from en_US_rules import Rules

phonemes = [
		"IY", "IH", "EY", "EH", "AE", "AA", "AO", "OW", "UH", "UW", "ER", "AX", "AH", "AY", "AW", "OY", "p", "b", "t", "d", "k", "g", "f", "v",
		"TH", "DH", "s", "z", "SH", "ZH", "h", "m", "n", "NG", "l", "w", "y", "r", "CH", "j", "WH", "PAUSE", ""
]

new_rules = {}

def make_map(phonemes):
	phoneme_map = {}
	for i,phoneme in enumerate(phonemes):
		phoneme_map[phoneme] = str(i)
	print(phoneme_map)
	return phoneme_map

def replace_phonemes(phoneme_map):
#	print(Rules)
	for rule in Rules:
#		print(rule)
#		print(Rules[rule])
		for i,elem in enumerate(Rules[rule]):
			to_transform = elem[3].split()
			transformed = ""
			for phoneme in to_transform:
				transformed += phoneme_map[phoneme]
			temp = list(elem)
			temp[3] = transformed
			elem = tuple(temp)
			Rules[rule][i]=elem
#			print(elem)
#	print(Rules)

def output_helper(output, elem):
	if elem == "":
		output += "Anything"
	elif elem == " ":
		output += "Nothing"
	else:
		output += "\"" + elem + "\""
	return output

def output_C_format():
	file=open("newfile.txt","w")
	output = ""
#	print(Rules)
	for rule in Rules:
#		print(rule)
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

phoneme_map = make_map(phonemes)
replace_phonemes(phoneme_map)
output_C_format()

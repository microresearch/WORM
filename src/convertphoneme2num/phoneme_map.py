
phonemes = [
		"IY", "IH", "EY", "EH", "AE", "AA", "AO", "OW", "UH", "UW", "ER", "AX", "AH", "AY", "AW", "OY", "p", "b", "t", "d", "k", "g", "f", "v",
		"TH", "DH", "s", "z", "SH", "ZH", "h", "m", "n", "NG", "l", "w", "y", "r", "CH", "j", "WH", "PAUSE", ""
]

def make_map():
	phoneme_map = {}
	for i,phoneme in enumerate(phonemes):
		phoneme_map[phoneme] = str(i)
	print(phoneme_map)
	return phoneme_map
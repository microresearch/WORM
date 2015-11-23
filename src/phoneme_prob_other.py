import nltk,random,sys
from nltk.tokenize import RegexpTokenizer
f = open('/root/beddoes_phonemes_other_fixed')
raw = f.read()

#tokens = nltk.word_tokenize(raw)

#tokenizer = RegexpTokenizer('\//', gaps=True)
#tokens=tokenizer.tokenize(raw)

#Create your bigrams
bgs = nltk.bigrams(raw)

#for bg in bgs:
#    print bg

print bgs

#compute frequency distribution for all the bigrams in the text
#fdist = nltk.FreqDist(bgs)
#fdist = nltk.FreqDist(tokens)
#for k,v in fdist.items():
#    print k,v

# how many phonemes in KLATT? 69

#templist=fdist
#templist=sorted(templist, key=int) 
#for ff in templist:
#    print ff

#for word, frequency in fdist.most_common(64):
#    print('%s;%d' % (word, frequency)).encode('utf-8')
#    print ('%s' % word +","),

def generate_model(cfdist, word, num=15):
    for i in range(num):
        words = list(cfdist[word])
        word = random.choice(words)
        print word, # no space?
        sys.stdout.write("")
 #       word = cfdist[word].max()


cfd = nltk.ConditionalFreqDist(bgs) # [_bigram-condition]
print cfd
#print cfd

generate_model(cfd,'t',1024)

import nltk,random
from nltk.tokenize import RegexpTokenizer
f = open('/root/beddoes_phonemes')
raw = f.read()

#tokens = nltk.word_tokenize(raw)

tokenizer = RegexpTokenizer('\//', gaps=True)
tokens=tokenizer.tokenize(raw)

#Create your bigrams
bgs = nltk.bigrams(tokens)

#for bg in bgs:
#    print bg

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
        print word,
        word = cfdist[word].max()


cfd = nltk.ConditionalFreqDist(bgs) # [_bigram-condition]
#print cfd

generate_model(cfd,'1',1024)

# example of word generation using trigrams

import abbrase

import random
import re

graph = abbrase.WordGraph('wordlist_bigrams.txt')
words = [' %s.' % word.lower()
         for word in graph.wordlist if re.match('^[A-za-z]*$', word)]

# find common trigrams
trigrams = set()

for word in words[:10000]:
    for pos in xrange(0, len(word) - 2):
        trigrams.add(word[pos:pos + 3])

trigrams = sorted(trigrams)

print len(trigrams)

def gen_word(length):
    gen = ' '
    while True:
        possible = [ngram for ngram in trigrams if ngram.startswith(gen[-2:])]
        if len(gen) == length:
            if any('.' in word for word in possible):
                return gen
            else:
                # restart
                possible = []
        if not possible:
            gen = ' '
            continue
        choice = random.choice(possible)
        if len(gen) == 1:
            gen = choice[1:]
        else:
            gen += choice[-1]


for _ in range(20):
    print gen_word(10),

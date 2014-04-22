import struct

import digest


def gen_cryptorand_int():
    return struct.unpack('>I', open('/dev/urandom', 'rb').read(4))[0]


class WordGraph(object):

    def __init__(self, fname):
        compressed_graph = open(fname)
        n_words = int(compressed_graph.readline())
        self.wordlist = ['']  # ['', 'and', 'the', ...]
        self.prefixes = {}  # {'and': [1, ...], ...}
        for n in xrange(1, n_words, 1):
            word = compressed_graph.readline().strip()
            self.wordlist.append(word)
            self.prefixes.setdefault(word[:3], []).append(n)

        self.followers = []

        for a in xrange(n_words):
            self.followers.append(compressed_graph.readline().strip())

    def get_followers(self, node_number):
        return set(digest.decode(self.followers[node_number]))

    def gen_passphrase(self, length):
        # pick the series of prefixes (3-letter abbreviations)
        # that will make up the password
        prefix_list = list(self.prefixes)
        assert len(prefix_list) == 1024
        out = []
        for _ in xrange(length):
            out.append(prefix_list[gen_cryptorand_int() & 1023])

        # find possible words for each of the chosen prefixes
        word_sets = [set(self.prefixes[p]) for p in out]

        # working backwards, reduce possible words for each prefix
        # to only those words that have an outgoing edge to a word
        # in the next set of possible words
        next_words = set()

        # sometimes a transition between two prefixes is impossible
        # (~13% of prefix pairs don't have associated bigrams)
        # it doesn't seem to matter very much, but let's keep track of it
        mismatch = -1
        for words in word_sets[::-1]:
            new_words = set(word for word in words
                            if self.get_followers(word) & next_words)
            if not new_words:
                mismatch += 1
                new_words = words
            words.intersection_update(new_words)
            next_words = words

        # working forwards, pick a word for each prefix
        last_word = 0
        out_words = []
        for words in word_sets:
            words = (words & self.get_followers(last_word)) or words
            # heuristic: try to chain with the most common word
            # (smallest node number)
            word = min(words)
            out_words.append(self.wordlist[word])
            last_word = word

        # print mismatch
        return ''.join(out), ' '.join(out_words)

graph = WordGraph('wordlist_bigrams.txt')

count = 32
length = 5
print 'Generating %d passwords with %d bits of entropy' % (count, length * 10)
pass_len = length * 3
print 'Password'.ljust(pass_len), '  ', 'Phrase'
print '-' * pass_len, '  ', '-' * (4 * length)
for _ in xrange(count):
    print '%s    %s' % graph.gen_passphrase(length)

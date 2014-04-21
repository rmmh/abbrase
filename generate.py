import random
import struct

def gen_cryptorand_int():
    return struct.unpack('>I', open('/dev/urandom', 'rb').read(4))[0]


class WordGraph(object):

    def __init__(self, fname):
        digest = open(fname)
        n_words = int(digest.readline())
        common = ['']
        prefixes = {}
        for n in xrange(1, n_words, 1):
            word = digest.readline().strip()
            common.append(word)
            prefixes.setdefault(word[:3], []).append(n)

        edges = []

        for a in xrange(n_words):
            line = digest.readline()
            edges.append(line)

        self.common = common
        self.prefixes = prefixes
        self.edges = edges

    def get_outgoing(self, node_number):
        'lazily decode edges'
        outgoing = self.edges[node_number]

        if isinstance(outgoing, str):
            num = 0
            following = set()
            for b in outgoing.split():
                num += int(b)
                following.add(num)
            outgoing = following
            self.edges[node_number] = outgoing
        return outgoing

    def gen_passphrase(self, length):
        prefix_list = list(self.prefixes)
        assert len(prefix_list) == 1024
        out = []
        for _ in xrange(length):
            out.append(prefix_list[gen_cryptorand_int() & 1023])

        word_sets = [set(self.prefixes[p]) for p in out]

        next_words = set()
        mismatch = -1
        for words in word_sets[::-1]:
            new_words = set(
                word for word in words if self.get_outgoing(word) & next_words)
            if not new_words:
                mismatch += 1
                new_words = words
            words.intersection_update(new_words)
            next_words = words

        last_word = 0
        out_words = []
        for words in word_sets:
            words = (words & self.get_outgoing(last_word)) or words
            word = min(words)
            out_words.append(self.common[word])
            last_word = word

        #print mismatch
        return ''.join(out), ' '.join(out_words)


graph = WordGraph('data/2gram_digest.txt')

count = 32
length = 5
print 'Generating %d passwords with %d bits of entropy' % (count, length * 10)
pass_len = length * 3
print 'Password'.ljust(pass_len), '  ', 'Phrase'
print '-' * pass_len, '  ', '-' * (4 * length)
for _ in xrange(count):
    print '%s    %s' % graph.gen_passphrase(length)

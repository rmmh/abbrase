#!/usr/bin/env python

import math
import random
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
            self.followers.append(compressed_graph.readline().rstrip('\n'))

    def get_followers(self, node_number):
        return set(digest.decode(self.followers[node_number]))

    def gen_password(self, length):
        # pick the series of prefixes (3-letter abbreviations)
        # that will make up the password
        prefix_list = list(self.prefixes)
        assert len(prefix_list) == 1024
        out = []
        for _ in xrange(length):
            out.append(prefix_list[gen_cryptorand_int() & 1023])
        return ''.join(out)

    def gen_passphrase(self, length, password=None):
        if password is None:
            password = self.gen_password(length)
        assert len(password) == length * 3
        password_chunks = [password[x:x + 3]
                           for x in xrange(0, len(password), 3)]

        # find possible words for each of the chosen prefixes
        word_sets = [set(self.prefixes[p]) for p in password_chunks]

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
        return password, ' '.join(out_words)


def wordgraph_dump(a, b):
    for n in xrange(a, b):
        print '#%d: %s: %.30s %s' % (n, graph.wordlist[n], graph.followers[n],
                                     digest.decode(graph.followers[n]))



class PhraseGenerator(object):

    def __init__(self, graph, n_words):
        self.graph = graph
        self.n_words = n_words
        self.adjacency_lists = []
        for n in xrange(n_words):
            self.adjacency_lists.append([x for x in digest.decode(self.graph.followers[n])
                                         if x < n_words])

    def generate(self, length, chosen_path=None):
        ''' generate a random phrase. this method is not cryptographically secure '''
        # pick a phrase at random
        # or, pick a path through a DAG uniformly from all paths possible

        # 1) calculate how many paths can reach each word
        path_counts = [[0] * self.n_words for _ in xrange(length)]

        for n in xrange(self.n_words):
            path_counts[0][n] = 1

        for level in xrange(length - 1):
            for n in xrange(self.n_words):
                count = path_counts[level][n]
                for out in self.adjacency_lists[n]:
                    path_counts[level + 1][out] += count

        # 2) pick a path to backtrack along
        total_paths = sum(path_counts[-1])
        if chosen_path is None:
            # *WARNING* not cryptographically secure
            chosen_path = random.randint(0, total_paths)
        print '%.2f bits of entropy' % math.log(total_paths, 2),  "chose %d/%d" % (chosen_path, total_paths)

        # 3) working backwards, pick the word that contributed our chosen_path
        words = []
        for level in xrange(length - 1, -1, -1):
            for n in xrange(self.n_words):
                if (not words or words[-1] in self.adjacency_lists[n]) and path_counts[level][n] > chosen_path:
                    words.append(n)
                    break
                else:
                    chosen_path -= path_counts[level][n]
            else:
                print "couldn't find a predecessor :(", words, level
        phrase = ' '.join(self.graph.wordlist[word] for word in words[::-1])
        print phrase

#pg = PhraseGenerator(graph, 16)
#pg.generate(5)


if __name__ == '__main__':
    graph = WordGraph('wordlist_bigrams.txt')
    # wordgraph_dump(1, 3000)
    count = 32
    length = 5
    print 'Generating %d passwords with %d bits of entropy' % (count, length * 10)
    pass_len = length * 3
    print 'Password'.ljust(pass_len), '  ', 'Mnemonic'
    print '-' * pass_len, '  ', '-' * (4 * length)
    for _ in xrange(count):
        print '%s    %s' % graph.gen_passphrase(length)

    assert graph.gen_passphrase(5, password="untneragedronic")[1] \
                    == "until nerve agent dropped nice"

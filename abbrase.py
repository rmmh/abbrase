#!/usr/bin/env python3

import argparse
import math
import secrets
import sys

import digest


class WordGraph(object):

    def __init__(self, fname):
        with open(fname) as compressed_graph:
            n_words = int(compressed_graph.readline())
            self.wordlist = ['']  # ['', 'and', 'the', ...]
            self.prefixes = {}  # {'and': [1, ...], ...}
            for n in range(1, n_words):
                word = compressed_graph.readline().strip()
                self.wordlist.append(word)
                self.prefixes.setdefault(word[:3].lower(), []).append(n)

            self.followers = []

            for a in range(n_words):
                self.followers.append(compressed_graph.readline().rstrip('\n'))

    def get_followers(self, node_number):
        return set(digest.decode(self.followers[node_number]))

    def gen_password(self, length, seed=0):
        # pick the series of prefixes (3-letter abbreviations)
        # that will make up the password
        prefix_list = list(self.prefixes)
        assert len(prefix_list) == 1024
        out = []
        if seed:
            while seed:
                out.append(prefix_list[seed & 1023])
                seed >>= 10
        else:
            for _ in range(length):
                out.append(prefix_list[secrets.randbelow(1024)])
        return ''.join(out)

    def split_password(self, password):
        assert len(password) % 3 == 0
        return [password[x:x + 3].lower() for x in range(0, len(password), 3)]

    def numbered_to_phrase(self, word_numbers):
        return ' '.join(self.wordlist[n] for n in word_numbers)

    def gen_passphrase_numbered(self, prefixes, skip_sets=None):
        # find possible words for each of the chosen prefixes
        word_sets = [set(self.prefixes[p]) for p in prefixes]

        if skip_sets is not None:
            assert len(skip_sets) == len(word_sets)
            for words, skips in zip(word_sets, skip_sets):
                if len(skips) < len(words):
                    words.difference_update(skips)
                assert words, "no words left!"

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
        out_word_numbers = []
        for words in word_sets:
            words = (words & self.get_followers(last_word)) or words
            # heuristic: try to chain with the most common word
            # (smallest node number)
            word = min(words)
            out_word_numbers.append(word)
            last_word = word

        return out_word_numbers

    def gen_passphrase(self, password):
        prefixes = self.split_password(password)

        word_numbers = self.gen_passphrase_numbered(prefixes)

        return self.numbered_to_phrase(word_numbers)

    def gen_passphrases(self, password, count=16):
        prefixes = self.split_password(password)
        skip_sets = [set() for _ in prefixes]
        phrases = []

        for _ in range(count):
            phrase_numbers = self.gen_passphrase_numbered(prefixes, skip_sets)
            for word, skips in zip(phrase_numbers, skip_sets):
                skips.add(word)
            phrases.append(self.numbered_to_phrase(phrase_numbers))

        return phrases


def wordgraph_dump(a, b):
    for n in range(a, b):
        print('#%d: %s: %.30s %s' % (n, graph.wordlist[n], graph.followers[n],
                                     digest.decode(graph.followers[n])))

def table(strings):
    split_strings = [s.split() for s in strings]
    position_lengths = [[len(w) for w in s] for s in split_strings]
    widths = [max(lens) for lens in zip(*position_lengths)]
    return [' '.join(word.ljust(width) for word, width in zip(words, widths))
            for words in split_strings]

class PhraseGenerator(object):
    def __init__(self, graph, n_words=None):
        self.graph = graph
        self.n_words = n_words = n_words or len(graph.wordlist) - 1
        assert self.graph.wordlist[0] == ''
        self.idx_to_graph = sorted(range(1, n_words + 1), key=self.graph.wordlist.__getitem__)

        graph_to_idx = [self.idx_to_graph.index(n) for n in range(1, n_words+1)]
        graph_to_idx = [None] + sorted(range(n_words), key=self.idx_to_graph.__getitem__)
        assert 0 not in self.idx_to_graph
        assert 0 in graph_to_idx
        for n in range(n_words):
            assert n == graph_to_idx[self.idx_to_graph[n]], n
        # print(self.idx_to_graph, [self.graph.wordlist[x] for x in self.idx_to_graph])
        self.adjacency_lists = [0] * n_words
        for n in range(1, n_words + 1):
            self.adjacency_lists[graph_to_idx[n]] = sorted(
                [graph_to_idx[x] for x in digest.decode(self.graph.followers[n])
                 if x <= n_words])
        self.path_counts = []
        self.total_paths = 0

    def _prepare_path_counts(self, length):
        if len(self.path_counts) != length:
            path_counts = [[0] * self.n_words for _ in range(length)]

            for n in range(self.n_words):
                path_counts[length - 1][n] = 1

            for level in range(length - 2, -1, -1):
                for n in range(self.n_words):
                    count = 0
                    for out in self.adjacency_lists[n]:
                        count += path_counts[level + 1][out]
                    path_counts[level][n] = count
            self.path_counts = path_counts
            self.total_paths = sum(path_counts[0])

        return self.path_counts

    def n_bits(self, length):
        self._prepare_path_counts(length)
        return math.log2(self.total_paths)

    def generate(self, length, chosen_path=None):
        ''' generate a random phrase '''
        # pick a phrase at random
        # or, pick a path through a DAG uniformly from all paths possible

        path_counts = self._prepare_path_counts(length)

        # 2) pick a path to follow
        if chosen_path is None:
            chosen_path = secrets.randbelow(self.total_paths)
            #print('%.2f bits of entropy' % math.log(self.total_paths, 2), end=' ')
            #print("chose %d/%d" % (chosen_path, self.total_paths))
        if not 0 <= chosen_path < self.total_paths:
            raise ValueError('chosen path %d not in [0,%d)' % (chosen_path, self.total_paths))

        # 3) working forwards, pick the word that contributed our chosen_path
        path = chosen_path
        words = []
        for level in range(length):
            for n in range(self.n_words) if level == 0 else self.adjacency_lists[words[-1]]:
                #print(words, n, path_counts[level][n], path, self.adjacency_lists[n])
                if path_counts[level][n] <= path:
                    path -= path_counts[level][n]
                else:
                    words.append(n)
                    break
            else:
                print("couldn't find a successor :(", words, level)
        assert len(words) == length, chosen_path
        return ' '.join(self.graph.wordlist[self.idx_to_graph[word]] for word in words)

def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--phrase', action='store_true',
                        help='Generate passphrases instead of abbrases.')
    parser.add_argument('-m', '--multiple', action='store_true',
                        help='generate multiple mnemonics for each password')
    parser.add_argument('length', default=5, type=int, nargs='?')
    parser.add_argument('count', default=32, type=int, nargs='?')
    parser.add_argument('-s', '--seed', type=int, help='convert number into passphrase')
    options = parser.parse_args(args)

    if options.seed:
        options.count = 1

    graph = WordGraph('wordlist_bigrams.txt')

    # wordgraph_dump(1, 3000)
    count = options.count
    length = options.length
    n_bits = length * 10
    if options.phrase:
        pg = PhraseGenerator(graph)
        n_bits = '%.2f' % pg.n_bits(length)
    if not options.seed:
        print('Generating %d passwords with %s bits of entropy' % (
            count, n_bits))
    if options.phrase:
        print('Passphrase')
        print('-' * length * (1 + sum(len(w) for w in graph.wordlist) // len(graph.wordlist)))
    else:
        pass_len = length * 3
        print('Password'.ljust(pass_len), '  ', 'Mnemonic')
        print('-' * pass_len, '  ', '-' * (4 * length))
    for _ in range(count):
        if options.phrase:
            print(pg.generate(length, chosen_path=options.seed))
        else:
            if options.seed:
                password = graph.gen_password(0, seed=options.seed)
            else:
                password = graph.gen_password(length)
            if options.multiple:
                phrases = graph.gen_passphrases(password)
                print('%s   ' % (password))
                print('\t' + '\n\t'.join(table(phrases)))
            else:
                phrase = graph.gen_passphrase(password)
                print('%s   %s' % (password, phrase))

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

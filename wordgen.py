#!/usr/bin/env python3
import argparse
import collections
import itertools
import json
import math
import re
import secrets
import string
import shutil

N = 3
V = None

ROMAN_RE = re.compile('^(?=[MDCLXVI])M*(C[MD]|D?C*)(X[CL]|L?X*)(I[XV]|V?I*)$', re.I)


def build_ngrams():
    counts = collections.defaultdict(int)
    for line in open('/usr/share/dict/words'):
        word = line.strip()
        if not re.match(r'^[a-z]{3,}$', word):
            continue
        word = line.strip()
        counts[' ' + word[:N-1]] += 1
        for n in range(len(word) - N + 2):
            counts[word[n:n+N].ljust(N)] += 1

    allowed = set()
    for k, v in counts.items():
        if v > 7 and k.strip().isalpha():
            allowed.add(k)

    print(' '.join(x.replace(' ', '`') for x in sorted(allowed)))
    print(len(allowed), 26 ** N + (N-1) * 26 ** 2)
    assert 'fbt' not in counts, counts['fbt']

    if 0:
        length = 10
        trials = 1 << 18
        valid = 0
        for _ in range(trials):
            word = ''.join(secrets.choice(string.ascii_lowercase) for _ in range(length))
            if ' ' + word[:N-1] in allowed and all(word[n:n+N].ljust(N) in allowed for n in range(len(word) - N + 2)):
                valid += 1
                print(word)
        print(valid, trials)
        est = int(valid/trials * 26 ** length)
        print(est, math.log2(est))
    if 0:
        global V
        V = []
        valid = 0
        att = 0
        for word in itertools.product(string.ascii_lowercase, repeat=6):
            word = ''.join(word)
            att += 1
            if ' ' + word[:N-1] in allowed and all(word[n:n+N].ljust(N) in allowed for n in range(len(word) - N + 2)):
                valid += 1
                V.append(word)
                if valid % 65536 == 0:
                    print(word)
        print('V:', valid, att)

    out = [0] * 27 ** 2
    assert 96 == ord('`')
    for w in allowed:
        w = w.replace(' ', '`').encode('ascii')
        out[(w[0] - 96) * 27 + w[1] - 96] |= 1 << (w[2] - 96)

    return out


class WordGenerator(object):
    def __init__(self, allowed, n_words=None):
        self.allowed = allowed
        self.path_counts = []
        self.total_paths = 0

    def _prepare_path_counts(self, length):
        if len(self.path_counts) != length:
            path_counts = [[0] * 27 ** 2 for _ in range(length)]

            suff = []
            for n in range(27 ** 2):
                if self.allowed[n] & 1:
                    path_counts[length - 1][n] = 1
                    suff.append('%s%s' % (chr(96 + (n // 27)), chr(96 + (n % 27))))

            for level in range(length - 2, 0, -1):
                for n in range(27, 27 ** 2):
                    count = 0
                    for x in range(1, 27):
                        if self.allowed[n] & (1 << x):
                            count += path_counts[level + 1][(n % 27) * 27 + x]
                    path_counts[level][n] = count
            for n in range(27):
                count = 0
                for x in range(1, 27):
                    if self.allowed[n] & (1 << x):
                        count += path_counts[1][(n % 27) * 27 + x]
                path_counts[0][n] = count
            self.path_counts = path_counts
            self.total_paths = sum(path_counts[0])

        return self.path_counts

    def n_bits(self, length):
        self._prepare_path_counts(length)
        return math.log2(self.total_paths)

    def generate(self, length, chosen_path=None):
        ''' generate a random Englishy word '''
        # pick a word at random
        # or, pick a path through a DAG uniformly from all paths possible

        path_counts = self._prepare_path_counts(length)

        # 2) pick a path to follow
        if chosen_path is None:
            chosen_path = secrets.randbelow(self.total_paths)
            #print('%.2f bits of entropy' % math.log(self.total_paths, 2), end=' ')
            #print("chose %d/%d" % (chosen_path, self.total_paths))
        if not 0 <= chosen_path < self.total_paths:
            raise ValueError('chosen path %d not in [0,%d)' % (chosen_path, self.total_paths))

        # 3) working forwards, pick the character that contributed our chosen_path
        path = chosen_path
        grams = []
        for level in range(length):
            if level == 0:
                adj = range(len(path_counts[0]))  # first level: (space, char, char)
            else:
                base = grams[-1] % 27 * 27
                adj = (base + x for x in range(27) if self.allowed[grams[-1]] & (1 << x))
            for n in adj:
                #print(grams, n, path_counts[level][n], path, self.adjacency_lists[n])
                if path_counts[level][n] <= path:
                    path -= path_counts[level][n]
                else:
                    grams.append(n)
                    break
            else:
                print("couldn't find a successor :(", grams, level)
        assert len(grams) == length, chosen_path
        return ''.join(chr(c % 27 + 96) for c in grams)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--digest', action='store_true', help='build trigrams from data file')
    parser.add_argument('-a', '--all', action='store_true', help='emit all possibilities')
    parser.add_argument('-l', '--line', action='store_true', help='one output per line')
    parser.add_argument('length', default=9, type=int, nargs='?')
    parser.add_argument('count', default=64, type=int, nargs='?')
    options = parser.parse_args()

    if options.digest:
        allowed = build_ngrams()
        with open('trigrams.json', 'w') as f:
            json.dump(allowed, f)

    gen = WordGenerator(json.load(open('trigrams.json')))

    if options.digest and V:
        for n, v in enumerate(V):
            assert gen.generate(len(v), n) == v, (n, v)

    cols = shutil.get_terminal_size((80, 0)).columns
    if options.line:
        cols = 0

    if options.all:
        print('Generating all {2:,} {0}-letter words with {1:.1f} bits of entropy'.format(
            options.length, gen.n_bits(options.length), gen.total_paths))
    else:
        print('Generating {} {}-letter words with {:.1f} bits of entropy ({:,} possible)'.format(
            options.count, options.length, gen.n_bits(options.length), gen.total_paths))

    words = (gen.generate(options.length) for _ in range(options.count))
    if options.all:
        words = (gen.generate(options.length, n) for n in range(gen.total_paths))

    if cols > (2 + options.length * 2):
        words = list(words)
        cols = 1 + (cols - options.length) // (2 + options.length)
        for x in range(0, len(words), cols):
            print('  '.join(words[x:x+cols]))
        if len(words) % cols:
            print('  '.join(words[x+cols:]))
    else:
        for w in words:
            print(w)

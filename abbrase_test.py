#!/usr/bin/env python3

import unittest
import secrets

import abbrase


class AbbraseTest(unittest.TestCase):
    def setUp(self):
        self.graph = abbrase.WordGraph('wordlist_bigrams.txt')

    def test_gen_passphrase(self):
        for phrase in (
                'until nerve agent dropped nice',
                'obedience observed Tom instead engaged',
                'determined off agricultural winter squadron'):
            words = phrase.split()
            password = ''.join(x[:3] for x in words)
            assert self.graph.gen_passphrase(password) == phrase

    def test_gen_password(self):
        passwords = set()
        while len(passwords) < 1024:
            passwords.add(self.graph.gen_password(1))

class WordGraphTest(unittest.TestCase):
    def setUp(self):
        self.graph = abbrase.WordGraph('wordlist_bigrams.txt')

    def test_generate(self):
        gen = abbrase.PhraseGenerator(self.graph, 256)
        for c in [1, 2]:
            gen.generate(c, 1)
            ps = [gen.generate(c, n) for n in range(gen.total_paths)]
            self.assertEqual(ps, sorted(ps))

    def test_generate_large(self):
        # do probabilistic testing for large phrase generation:
        # pick a random phrase, then test narrowing bounds around the phrase to ensure ordering
        gen = abbrase.PhraseGenerator(self.graph)
        gen.generate(5, 1)
        print("testing", gen.total_paths)
        for trial in range(100):
            target = secrets.randbelow(gen.total_paths)
            last_lo, last_hi = '', '~'
            lo_idx = 0
            hi_idx = gen.total_paths - 1
            n = 0
            while lo_idx < target < hi_idx:
                n += 1
                lo_gen = gen.generate(5, lo_idx)
                hi_gen = gen.generate(5, hi_idx)
                # print(lo_gen, hi_gen)
                self.assertLess(lo_gen, hi_gen, (lo_idx, hi_idx, lo_gen, hi_gen))
                self.assertLess(last_lo, lo_gen)
                self.assertLess(hi_gen, last_hi)
                lo_idx += (target - lo_idx + 1) // 2
                hi_idx -= (hi_idx - target + 1) // 2
                last_lo = lo_gen
                last_hi = hi_gen
            target_gen = gen.generate(5, target)
            self.assertLess(lo_gen, target_gen)
            self.assertLess(target_gen, hi_gen)
            print(trial, n, target, target - lo_idx, target - hi_idx, target_gen)



if __name__ == '__main__':
    unittest.main()

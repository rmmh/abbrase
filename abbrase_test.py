#!/usr/bin/env python

import unittest

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

if __name__ == '__main__':
    unittest.main()

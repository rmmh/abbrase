import gzip


def build_common(digest):
    common = {}
    n = 1
    out = ''
    for line in open('data/1gram_common.csv'):
        word = line.lower().split()[0]
        if word in common:
            continue
        if word[:3] in prefixes:
            common[word] = n
            out += word + '\n'
            n += 1
    digest.write('%s\n' % n)
    digest.write(out)
    print 'words:', n

    return common


def build_edges(common):
    edges = [[] for _ in xrange(len(common) + 1)]
    last_a = ''
    prefix_transitions = set()
    for line in gzip.GzipFile('data/2gram.csv.gz'):
        parts = line.lower().split()
        if len(parts) == 3:
            a, b, count = parts
            if a not in common or b not in common:
                continue
            prefix_a = a[:3]
            prefix_b = b[:3]
            prefix_transitions.add(prefix_a + prefix_b)
            if a != last_a:
                if last_a:
                    if a[0] != last_a[0]:
                        print '!', last_a
                last_a = a
            edges[common[a]].append(common[b])

    print 'Attested prefix-prefix combinations:',
    print '%.3f%%' % (100.0 * len(prefix_transitions) /
                     (1.0 * len(prefixes) * len(prefixes)))

    return edges


def encode(l):
    ''' pack list of monotonically increasing positive integers into a string

    replace elements with the difference from the previous element minus one,
    then encode as printable base-32 varints

    >>> encode([1, 2, 3, 5, 8, 13, 21])
    '@@@ABDG'
    '''
    enc = ''
    last_num = 0
    for num in l:
        delta = num - last_num - 1
        assert delta >= 0, "input must be strictly increasing"
        last_num = num
        while True:
            enc += chr((0x40 if delta < 0x20 else 0x20) | (delta & 0x1f))
            delta >>= 5
            if not delta:
                break
    return enc


def decode(enc):
    ''' inverse of encode

    >>> deencode('@@@ABDG')
    [1, 2, 3, 5, 8, 13, 21]
    '''
    enc_ind = 0
    dec = []
    last_num = 0
    while enc_ind < len(enc):
        delta = 0
        delta_ind = 0
        while True:
            val = ord(enc[enc_ind + delta_ind])
            delta |= (val & 0x1f) << (5 * delta_ind)
            delta_ind += 1
            if val & 0x40:
                break
        enc_ind += delta_ind
        num = last_num + int(delta) + 1
        last_num = num
        dec.append(num)
    return dec

for l in ([1, 2, 3, 5], [1], [1, 2, 3, 5, 6, 8, 9, 10, 11, 12, 3000000]):
    assert decode(encode(l)) == l


if __name__ == '__main__':
    prefixes = set()
    for line in open("data/prefixes.txt"):
        prefixes.add(line.split()[0])

    digest = open('data/2gram_digest.txt', 'wb')

    common = build_common(digest)
    edges = build_edges(common)

    for n, out in enumerate(edges):
        digest.write(encode(sorted(set(out))))
        digest.write('\n')

    digest.close()
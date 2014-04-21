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

    print len(prefix_transitions), len(prefix_transitions) / (1.0 * len(prefixes) * len(prefixes))

    return edges


def encode(l):
    ''' efficiently pack list of numbers into a string

    replace list elements with the difference from the previous element,
    then join with spaces

    >>> encode([1, 2, 3, 5])
    '1 1 1 2'
    '''
    enc = ''
    last_num = 0
    for num in l:
        enc += str(num - last_num) + ' '
        last_num = num
    return enc


def decode(enc):
    ''' inverse of encode

    >>> deencode('1 1 2 3 1')
    [1, 2, 4, 7, 8]
    '''
    dec = []
    last_num = 0
    for num in enc.split():
        num = int(num)
        dec.append(last_num + num)
        last_num = num
    return dec


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

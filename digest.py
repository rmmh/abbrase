import sys

prefixes = set()
prefix_transitions = set()

for line in open("data/prefixes.txt"):
    prefixes.add(line.split()[0])

digest = open('data/2gram_digest.txt', 'wb')

common = {}
n = 1
out = ''
for line in open('data/1gram_common.csv'):
    word = line.split()[0]
    if word[:3] in prefixes:
        common[word] = n
        out += word + '\n'
        n += 1
digest.write('%s\n' % (n - 1))
digest.write(out)

print n

last_a = ''

edges = [[] for _ in xrange(n)]

for line in sys.stdin:
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

for n, out in enumerate(edges):
    # simple differential encoding
    last_num = 0
    for num in sorted(set(out)):
        digest.write('%s ' % (num - last_num))
        last_num = num
    digest.write('\n')

digest.close()

print len(prefix_transitions), len(prefix_transitions)/(1.0 * len(prefixes) * len(prefixes))

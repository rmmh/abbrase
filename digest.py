import sys

prefixes = set()
prefix_transitions = set()

for line in open("data/prefixes.txt"):
    prefixes.add(line.split()[0])

digest = open('data/2gram_digest.txt', 'wb')

common = {}
n = 1
for line in open('data/1gram_common.csv'):
    word = line.split()[0]
    if word[:3] in prefixes:
        common[word] = n
        digest.write(word + '\n')
        n += 1
digest.write('GRAPH:\n')

print n

last_a = ''

transition_buf = []

for line in sys.stdin:
    parts = line.split()
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
                digest.write('%s ' % common[last_a])
                # simple differential encoding
                last_num = 0
                for num in sorted(common[s] for s in transition_buf):
                    digest.write('%s ' % (num - last_num))
                    last_num = num
                digest.write('\n')
            transition_buf = []
            last_a = a
        transition_buf.append(b)

digest.close()

print len(prefix_transitions), len(prefix_transitions)/(1.0 * len(prefixes) * len(prefixes))

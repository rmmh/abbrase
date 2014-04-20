digest = open('data/2gram_digest.txt')

common = ['']
for n, line in enumerate(digest, 1):
    line = line.strip()
    if line == 'GRAPH:':
        break
    common.append(line)

for line in digest:
    a, bs = line.strip().split(' ', 1)
    print common[int(a)]
    num = 0
    following = []
    for b in bs.split():
        num += int(b)
        following.append(common[num])
    print ' '.join(sorted(following))

print common[:500]

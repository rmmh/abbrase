digest = open('data/2gram_digest.txt')

n_words = int(digest.readline())

common = ['']
for _ in xrange(n_words):
    common.append(digest.readline().strip())

print common[:500]

for a in xrange(n_words):
    line = digest.readline()
    num = 0
    if a > 10000:
        following = []
        for b in line.split():
            num += int(b)
            following.append(common[num])
        print common[int(a)]
        print ' '.join(sorted(following))

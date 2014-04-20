all: data/2gram_digest.txt

CORPUS_EXEMPLAR=googlebooks-eng-1M-2gram-20090715-99.csv.zip

data/${CORPUS_EXEMPLAR}:
	mkdir -p data
	cd data; curl -O -C - \
				'http://storage.googleapis.com/books/ngrams/books/googlebooks-eng-1M-1gram-20090715-[0-9].csv.zip' \
				'http://storage.googleapis.com/books/ngrams/books/googlebooks-eng-1M-2gram-20090715-[0-99].csv.zip'

# the ngrams data is 'mostly sorted' -- lines tend to be in order, but it occasionally restarts
# do a groupby (join records from different years into one) to reduce the data volume, then final sort+groupby
data/1gram.csv.gz: | data/${CORPUS_EXEMPLAR} groupby
	zcat data/googlebooks-eng-1M-1gram-*.csv.zip | pv | ./groupby 3 | LC_ALL=c sort | ./groupby 2 | gzip -9 > $@

data/2gram.csv.gz: | data/${CORPUS_EXEMPLAR} groupby
	zcat data/googlebooks-eng-1M-2gram-*.csv.zip | pv | ./groupby 3 | LC_ALL=c sort | ./groupby 2 | gzip -9 > $@

# extract the 100,000 most common words
data/1gram_common.csv: data/1gram.csv.gz
	zcat $< | sort -rgk2 | head -n 100000 > $@

data/prefixes.txt: data/1gram_common.csv
	cat $< | sed 's/^\(...\).*\t/\1\t/' | grep '^[a-z]\{3\}' | ./groupby 2 | sort -rgk2 | head -n 2048 > $@

data/2gram_digest.txt: data/prefixes.txt data/2gram.csv.gz
	zcat data/2gram.csv.gz | pypy digest.py

#Abbrase#

Abbrase is an abbreviated passphrase generator.

Creating secure passwords is easy. Remembering them is hard. [Pwgen](http://sourceforge.net/projects/pwgen/) makes them memorable though prounouncability. XKCD [suggests](http://xkcd.com/936/) using a series of random common words, but the "bag of random words" technique has difficulty scaling to higher entropy levels.

Abbrase is an experiment with generating probable phrases using markov chains, and abbreviating each word to the first few letters. This strikes a balance between excessive password length and excessive mnemonic length.

The markov models are generated with data from Google NGrams [dataset](http://storage.googleapis.com/books/ngrams/books/datasetsv2.html), used under the [Creative Commons Attribution 3.0 Unported License](http://creativecommons.org/licenses/by/3.0/). 

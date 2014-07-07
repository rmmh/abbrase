function WordGraph(url) {
	this.state = "init";
	this.n_words = 0;
	this.words = [];
	this.prefixes = {};
	this.prefix_list = [];
	this.followers_compressed = [];
	this.url = url;
	this.onprogress = null;
	this.onready = null;
}

WordGraph.prototype.load = function() {
	if (this.state != "init")
		return;
	this.state = "downloading";
	var xhr = new XMLHttpRequest();
	xhr.open('GET', this.url, true);
	xhr.totalsize = 20670394;
	var self = this;
	xhr.onreadystatechange = function(evt) {
		if (xhr.status >= 400) {
			if (self.onprogress)
				self.onprogress(-xhr.status);
		}
		if (xhr.readyState === 3) {
			var perc = (100 * xhr.response.length / xhr.totalsize)|0;
			if (xhr.perc != perc) {
				xhr.perc = perc;
				if (self.onprogress) {
					self.onprogress(perc);
				}
			}
		} else if (xhr.readyState === 4) {
			console.log("Done");
			self.parse(xhr.responseText);
		}
	}
	xhr.send();
}

WordGraph.prototype.parse = function(text) {
	var lines = text.split("\n");
	lines.reverse();

	this.n_words = parseInt(lines.pop());
	this.words.push('');
	for (var n = 1; n < this.n_words; n++) {
		var word = lines.pop();
		var prefix = word.substr(0, 3).toLowerCase();
		this.words.push(word);
		this.prefixes[prefix] = this.prefixes[prefix] || [];
		this.prefixes[prefix].push(n);
	}

	this.prefix_list = Object.keys(this.prefixes);
	this.prefix_list.sort();

	for (var n = 1; n <= this.n_words; n++) {
		this.followers_compressed.push(lines.pop());
	}

	this.state = "ready";

	if (this.onready !== null) {
		this.onready(this);
	}
}

WordGraph.prototype.get_followers = function(word_number) {
	var enc = this.followers_compressed[word_number];
	/*
	general encoding steps:
	input: [1, 2, 3, 5, 80]
	subtract previous value: [1, 1, 1, 2, 75]
	subtract 1: [0, 0, 0, 1, 74]
	contract runs of zeros: [0x3, 1, 74]
	printably encode numbers as base-32 varints,
	and runs of zeros as the 31 leftover characters:
	output: "bA*B"

	this function reverses the steps.

	Cf. decode in digest.py
	*/
	var enc_ind = 0;
	var dec = [];
	var last_num = 0;
	var zero_run = 0;
	while (enc_ind < enc.length || zero_run) {
		var delta = 0;
		var delta_ind = 0;
		if (zero_run) {
			zero_run--;
		} else {
			var val = enc.charCodeAt(enc_ind);
			if (val >= 0x60) {
				zero_run = val & 0x1f
				delta_ind += 1
			} else {
				do {
					val = enc.charCodeAt(enc_ind + delta_ind);
					delta |= (val & 0x1f) << (5 * delta_ind);
					delta_ind++;
				} while (!(val & 0x40));
			}
		}
		enc_ind += delta_ind;
		num = last_num + delta + 1;
		last_num = num;
		dec.push(num);
	}
	return dec;
};

function intersect(a, b) {
	var ret = [];
	var ai = 0, bi = 0;
	while (ai < a.length && bi < b.length) {
		var diff = a[ai] - b[bi];
		if (diff == 0) {
			ret.push(a[ai]);
			ai++, bi++;
		} else if (diff < 0) {
			ai++;
		} else if (diff > 0) {
			bi++;
		}
	}
	return ret;
}

function PassphraseGenerator(graph) {
	this.graph = graph;
}

PassphraseGenerator.prototype.gen_password = function(length) {
	if (this.graph.state !== "ready")
		throw new Error("wordgraph isn't loaded yet");

	if (this.graph.prefix_list.length != 1024)
		throw new RangeError("unexpected number of prefixes");

	/* pick series of prefixes that will make up the passwords */
	var prefix_numbers = new Uint16Array(length);
	crypto.getRandomValues(prefix_numbers);
	var prefixes_chosen = [];

	var word_sets = [];
	var out_password = "";

	/* find possible words for each of the chosen prefixes */
	for (var i = 0; i < length; i++) {
		prefix_numbers[i] = prefix_numbers[i] % 1024;
		prefixes_chosen[i] = this.graph.prefix_list[prefix_numbers[i]];
		out_password += prefixes_chosen[i];
		word_sets.push(this.graph.prefixes[prefixes_chosen[i]]);
	}

	/* working backwards, reduce possible words for each prefix to only
	   those words that have a link to a word in the next set of possible words
	*/

	var mismatch = 0; /* track how many links were impossible */
	var next_words = [];
	for (var i = length - 1; i >= 0; i--) {
		var words = word_sets[i];
		var new_words = [];
		if (next_words.length > 0) {
			for (var j = 0; j < words.length; j++) {
				var word = words[j];
				var followers = this.graph.get_followers(word);
				if (intersect(next_words, followers).length)
					new_words.push(word);
			}
		}
		if (new_words.length > 0) {
			word_sets[i] = new_words;
		} else {
			mismatch++;
		}

		next_words = word_sets[i];
	}

	/* working forwards, pick a word for each prefix */
	var out_words = [];
	var phrase_rank = 0;
	var last_word = 0;
	for (var i = 0; i < length; i++) {
		var followers = this.graph.get_followers(last_word);
		var forward = intersect(word_sets[i], followers);
		/* Picking the first word available biases the phrase towards more
		* common words, and produces generally satisfactory results.
		* N.B.: to save space, adjacency lists don't encode probabilities */
		last_word = (forward.length > 0 ? forward : word_sets[i])[0];
		phrase_rank += last_word;
		out_words.push(this.graph.words[last_word]);
	}

	return {"password": out_password,
		"mnemonic": out_words.join(" "),
		"phrase_rank": phrase_rank,
		"mismatch": mismatch,
		"numbers": prefix_numbers};
};

function ljust(inp, length, pad) {
	if (pad === undefined)
		pad = ' ';
	while (inp.length < length) {
		inp += pad;
	}
	return inp;
}

PassphraseGenerator.prototype.make_table = function(length, count) {
	var passwords = [];

	for (var i = 0; i < count; i++) {
		passwords.push(generator.gen_password(length));
	}

	// Sort by the sum of the word positions -- meaning phrases with more common
	// words come first.
	//
	// This tends to rank desirable passwords higher, but could aid an attacker
	// leveraging selection biases (maximum entropy loss is log2(count) bits).

	passwords.sort(function (a, b) {
		return a.phrase_rank - b.phrase_rank;
	});

	output = 'Generating ' + count + ' passwords with ';
	output += (length * 10) + ' bits of entropy\n';

	output += ljust("Password", 3 * length) + '    ' + 'Mnemonic\n';
	output += ljust('', 3 * length, '-') + '    ' + ljust('', 4 * length, '-') + '\n';

	for (var i = 0; i < count; i++) {
		var gen = passwords[i];
		output += gen.password + '    ' + gen.mnemonic;
		// output += ' ' + gen.phrase_rank + ' ' + gen.mismatch;
		output += '\n';
	}

	return output;
};

function pretty_arraybuffer(ab) {
	var ret = [];
	for (var i = 0; i < ab.length; i++)
		ret.push(ab[i]);
	return '[' + ret.join(', ') + ']';
}

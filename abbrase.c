#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PREFIXES 1024
#define PREFIX_LEN 3

struct IntVec {
  int len;
  int cap;
  int *data;
};

struct IntVec *intvec_alloc() {
  struct IntVec *vec = malloc(sizeof *vec);
  vec->len = 0;
  vec->cap = 1;
  vec->data = malloc(sizeof(int) * vec->cap);
  return vec;
}

void intvec_free(struct IntVec *vec) {
  free(vec->data);
  free(vec);
}

void intvec_append(struct IntVec *vec, int val) {
  if (vec->len == vec->cap) {
    vec->cap *= 2;
    vec->data = realloc(vec->data, sizeof(int) * vec->cap);
  }
  vec->data[vec->len++] = val;
}

int intvec_get(struct IntVec *vec, int pos) {
  if (pos < 0 || pos >= vec->len)
    err(10, "invalid vector index %d not in [0, %d)", pos, vec->len);
  return vec->data[pos];
}

struct IntVec *intvec_copy(struct IntVec *vec) {
  /* could be faster, but no need to optimize */
  struct IntVec *ret = intvec_alloc();
  int i;
  for (i = 0; i < vec->len; i++)
    intvec_append(ret, vec->data[i]);
  return ret;
}

void intvec_print(struct IntVec *vec) {
  int i;
  printf("[");
  for (i = 0; i < vec->len; i++) {
    if (i != 0)
      printf(", ");
    printf("%d", vec->data[i]);
  }
  printf("]");
}

/* return a new IntVec with the elements in common between a and b.
   Requires a and b to be sorted. */
struct IntVec *intvec_intersect(struct IntVec *a, struct IntVec *b) {
  struct IntVec *ret = intvec_alloc();
  int ai = 0, bi = 0;
  while (ai < a->len && bi < b->len) {
    int diff = a->data[ai] - b->data[bi];
    if (diff == 0) {
      intvec_append(ret, a->data[ai]);
      ai++, bi++;
    } else if (diff < 0) {
      ai++;
    } else if (diff > 0) {
      bi++;
    }
  }
  return ret;
}

struct Prefix {
  char prefix[PREFIX_LEN];
  struct IntVec *words;
};

int prefix_cmp(const void *a, const void *b) {
  return memcmp(((const struct Prefix*)a)->prefix,
                ((const struct Prefix*)b)->prefix, PREFIX_LEN);
}

struct WordGraph {
  int n_words;
  int n_prefixes;
  char **words;
  char **followers_compressed;
  struct Prefix prefixes[MAX_PREFIXES];
};

void getline_trimmed(char **target, FILE *stream) {
  size_t n, len;
  n = 0;
  *target = NULL;
  if (getline(target, &n, stream) == -1)
    err(1, "corrupted wordgraph file");
  len = strlen(*target);
  if ((*target)[len - 1] == '\n')
    (*target)[len - 1] = 0;
}

struct WordGraph *wordgraph_init(FILE *graph_file) {
  int i, j;
  struct WordGraph *g = malloc(sizeof *g);
  if (fscanf(graph_file, "%d ", &g->n_words) != 1)
    err(1, "corrupted wordgraph file");
  g->n_prefixes = 0;
  g->words = calloc(g->n_words, sizeof g->words[0]);
  g->followers_compressed = calloc(g->n_words, sizeof g->words[0]);
  for (i = 1; i < g->n_words; i++) {
    getline_trimmed(&g->words[i], graph_file);
    /* extract lowercase prefix */
    char prefix[PREFIX_LEN];
    for (j = 0; j < PREFIX_LEN; j++)
        prefix[j] = tolower(g->words[i][j]);
    /* add word to a prefix group */
    for (j = 0; j <= g->n_prefixes; ++j) {
      if (j == g->n_prefixes) {
        /* none found, need to insert */
        if (g->n_prefixes == MAX_PREFIXES)
          errx(2, "corrupted wordgraph file: too many prefixes");
        g->n_prefixes++;
        memcpy(g->prefixes[j].prefix, prefix, PREFIX_LEN);
        g->prefixes[j].words = intvec_alloc();
      }
      if (!memcmp(g->prefixes[j].prefix, prefix, PREFIX_LEN)) {
        intvec_append(g->prefixes[j].words, i);
        break;
      }
    }
  }
  /* sort prefixes (so passwords can be enumerated in order) */
  qsort(g->prefixes, g->n_prefixes, sizeof(struct Prefix), prefix_cmp);

  if (g->n_prefixes != MAX_PREFIXES)
    errx(3, "corrupted wordgraph file: not enough prefixes");
  for (i = 0; i < g->n_words; i++)
    getline_trimmed(&g->followers_compressed[i], graph_file);
  return g;
}

void wordgraph_free(struct WordGraph *g) {
  int i;
  for (i = 0; i < g->n_words; i++) {
    free(g->words[i]);
    free(g->followers_compressed[i]);
  }
  for (i = 0; i < g->n_prefixes; i++) {
    intvec_free(g->prefixes[i].words);
  }
  free(g->words);
  free(g->followers_compressed);
  free(g);
}

/* decode an adjacency list encoded as a string */
struct IntVec *decode(char *enc) {
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
  int enc_ind = 0;
  struct IntVec *dec = intvec_alloc();
  int last_num = 0;
  int zero_run = 0;
  while (enc[enc_ind] || zero_run) {
    int delta = 0;
    int delta_ind = 0;
    if (zero_run)
      zero_run--;
    else {
      unsigned char val = enc[enc_ind];
      if (val >= 0x60) {
        zero_run = enc[enc_ind] & 0x1f;
        delta_ind++;
      } else {
        /* decode base-32 varint */
        do {
          val = enc[enc_ind + delta_ind];
          delta |= (val & 0x1f) << (5 * delta_ind);
          delta_ind++;
        } while (val & 0x20);
      }
    }
    enc_ind += delta_ind;
    last_num += delta + 1;
    intvec_append(dec, last_num);
  }
  return dec;
}

void wordgraph_dump(struct WordGraph *g, int a, int b) {
  int i;
  for (i = a; i < b; i++) {
    printf("#%d: %s: %.30s ", i, g->words[i], g->followers_compressed[i]);
    struct IntVec *followers = decode(g->followers_compressed[i]);
    intvec_print(followers);
    intvec_free(followers);
    printf("\n");
  }
}

static int min(int a, int b) {
  if (a <= b)
    return a;
  return b;
}

int edit_distance(const char *a, const char *b) {
  // code based off http://hetland.org/coding/python/levenshtein.py

  int n = strlen(a), m = strlen(b);

  if (n > m) {
    // ensure n <= m, to use O(min(n,m)) space
    const char *tmp_s = a;
    a = b;
    b = tmp_s;
    int tmp_i = n;
    n = m;
    m = tmp_i;
  }

  int cost[n + 1];

  int i, j;
  int ins, del, sub;
  int prevdiag; // lets us store only one row + one cell at a time

  const int insert_cost = 1;
  const int gap_cost = 1;
  const int mismatch_cost = 1;

  for (i = 0; i < n + 1; ++i)
    cost[i] = i * insert_cost;

  for (i = 1; i < m + 1; ++i) {
    prevdiag = cost[0];
    cost[0] = i * gap_cost;

    for (j = 1; j < n + 1; ++j) {
      ins = cost[j] + gap_cost;
      del = cost[j - 1] + gap_cost;
      sub = prevdiag;
      if (a[j - 1] != b[i - 1])
        sub += mismatch_cost;
      prevdiag = cost[j];
      cost[j] = min(ins, min(del, sub));
    }
  }

  return cost[n];
}

/* find the closest word to the input */
int wordgraph_find_word(struct WordGraph *g, const char *word) {
  int i, best_word = 0, best_dist = 10000;
  for (i = 1; i < g->n_words; i++) {
    int dist = edit_distance(word, g->words[i]);
    if (dist < best_dist) {
      best_dist = dist;
      best_word = i;
    }
  }
  return best_word;
}

/* pick series of prefixes that will make up the passwords */
void generate_prefixes(int *prefix_indices_out, int length) {
  int fd_crypto, i;
  if ((fd_crypto = open("/dev/urandom", O_RDONLY)) < 0)
    err(5, "unable to get secure random numbers");

  int read_len = sizeof(*prefix_indices_out) * length;
  if (read(fd_crypto, prefix_indices_out, read_len) != read_len)
    err(6, "unable to read random numbers");

  for (i = 0; i < length; i++) {
    prefix_indices_out[i] &= MAX_PREFIXES - 1;
  }

  close(fd_crypto);
}

/* generate a reasonably memorable phrase for the prefixes */
int generate_mnemonic(
  struct WordGraph *g, int *prefix_indices, int *word_indices_out, int length) {
  /* find possible words for each of the chosen prefixes */
  int i, j;

  struct IntVec *word_sets[length];
  for (i = 0; i < length; i++) {
    word_sets[i] = intvec_copy(g->prefixes[prefix_indices[i]].words);
  }

  /* working backwards, reduce possible words for each prefix to only
     those words that have a link to a word in the next set of possible words
   */
  int mismatch = 0; /* track how many links were impossible */
  struct IntVec *next_words, *new_words, *followers, *words, *intersect;
  next_words = NULL;
  for (i = length - 1; i >= 0; i--) {
    words = word_sets[i];
    new_words = intvec_alloc();
    if (next_words) {
      for (j = 0; j < words->len; j++) {
        int word = intvec_get(words, j);
        followers = decode(g->followers_compressed[word]);
        intersect = intvec_intersect(next_words, followers);
        if (intersect->len)
          intvec_append(new_words, word);
        intvec_free(intersect);
        intvec_free(followers);
      }
    }
    if (new_words->len) {
      intvec_free(word_sets[i]);
      word_sets[i] = new_words;
    } else {
      intvec_free(new_words);
      mismatch++;
    }

    next_words = word_sets[i];
  }

  /* working forwards, pick a word for each prefix */
  int last_word = word_indices_out[0];
  for (i = 0; i < length; i++) {
    followers = decode(g->followers_compressed[last_word]);
    intersect = intvec_intersect(word_sets[i], followers);
    /* Picking the first word available biases the phrase towards more
     * common words, and produces generally satisfactory results.
     * N.B.: to save space, adjacency lists don't encode probabilities */
    last_word = intvec_get(intersect->len ? intersect : word_sets[i], 0);
    word_indices_out[i] = last_word;
    intvec_free(followers);
    intvec_free(intersect);
  }

  for (i = 0; i < length; i++) {
    intvec_free(word_sets[i]);
  }

  return mismatch;
}

void write_passphrase(
    struct Prefix *prefixes, int *prefix_indices, int length, char *buf) {
  int i;
  for (i = 0; i < length; i++)
    sprintf(buf + 3 * i, "%.3s", prefixes[prefix_indices[i]].prefix);
}

void run_tests(struct WordGraph *g) {
  /* enumerate passphrases, assert that there are as many as we expect */
  int i, length = 2;
  long expected = 1L << (length * 10L);
  long actual = 0;

  int prefix_indices[length];
  char passphrase[length * 3 + 1], last_passphrase[length * 3 + 1];

  last_passphrase[0] = 0;
  bzero(prefix_indices, sizeof(prefix_indices));
  while (1) {
    write_passphrase(g->prefixes, prefix_indices, length, passphrase);
    if (strcmp(passphrase, last_passphrase) > 0) {
      actual++;
    } else {
      errx(40, "passphrase out of order! %s -> %s",
        last_passphrase, passphrase);
    }
    memcpy(last_passphrase, passphrase, sizeof(last_passphrase));

    /* next passphrase */
    for (i = length - 1; i >= 0; --i) {
      prefix_indices[i]++;
      if (prefix_indices[i] != MAX_PREFIXES)
        break;
      prefix_indices[i] = 0;
    }
    if (i == -1)
      break;
  }

  if (actual != expected)
    errx(42, "test failed! expected: %ld != actual: %ld", expected, actual);
}

FILE *open_graph_file(char *executable, const char *graph_filename) {
  // #1 try cwd
  FILE *graph_file = fopen(graph_filename, "r");
  if (!graph_file) {
    // #2: try executable directory
    char *binary_dir = dirname(executable);
    char path[256];
    snprintf(path, 255, "%s/%s", binary_dir, graph_filename);
    graph_file = fopen(path, "r");
  }
#ifdef DATADIR
#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
  if (!graph_file) {
    // #3: datadir
    char path[256];
    snprintf(path, 255, STR(DATADIR) "/%s", graph_filename);
    graph_file = fopen(path, "r");
  }
#endif
  return graph_file;
}

int main(int argc, char *argv[]) {
  struct WordGraph *g ;
  const char *graph_filename = "wordlist_bigrams.txt";
  FILE *graph_file = open_graph_file(argv[0], graph_filename);

  if (!graph_file)
    err(1, "unable to find %s", graph_filename);

  g = wordgraph_init(graph_file);
  //wordgraph_dump(g, 0, 3000);


  long length = 0;
  long count = 0;
  int start_word = 0;
  int i, c;

  while ((c = getopt(argc, argv, "ht")) != -1) {
    switch (c) {
      case 't':
        run_tests(g);
        return 0;
      case 'h':
      default:
        errx(1, "usage: [-h]/[-t] [length] [count] [hook word]");
    }
  }

  for (i = 1; i < argc; i++) {
    errno = 0;
    if (length == 0) {
      length = strtol(argv[i], NULL, 10);
      if (!errno && length > 0)
        continue;
      length = 0;
    } else if (count == 0) {
      count = strtol(argv[i], NULL, 10);
      if (!errno && count > 0)
        continue;
      count = 0;
    }
    start_word = wordgraph_find_word(g, argv[i]);
  }

  if (!length)
    length = 5;

  if (!count)
    count = 32;

  printf("Generating %ld passwords with %ld bits of entropy\n", count,
         length * 10);

  if (start_word)
    printf("    hook: %s\n", g->words[start_word]);

  int pass_len = length * 3;
  printf("%-*s    %s\n", pass_len, "Password", "Mnemonic");
  for (i = 0; i < pass_len; i++)
    putchar('-');
  printf("    ");
  for (i = 0; i < length * 4; i++)
    putchar('-');
  printf("\n");

  while (count--) {
    int prefix_indices[length];
    int word_indices[length];
    char passphrase[3 * length + 1];
    word_indices[0] = start_word;
    generate_prefixes(prefix_indices, length);
    generate_mnemonic(g, prefix_indices, word_indices, length);

    write_passphrase(g->prefixes, prefix_indices, length, passphrase);
    printf("%s   ", passphrase);

    if (start_word)
      printf(" %s", g->words[start_word]);

    for (i = 0; i < length; i++)
      printf(" %s", g->words[word_indices[i]]);

    printf("\n");
  }

  wordgraph_free(g);

  return 0;
}

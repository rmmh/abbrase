#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
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

struct WordGraph {
  int n_words;
  int n_prefixes;
  char **words;
  char **followers_compressed;
  struct {
    char prefix[PREFIX_LEN];
    struct IntVec *words;
  } prefixes[MAX_PREFIXES];
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

struct WordGraph *wordgraph_init(const char *filename) {
  int i, j;
  FILE *graph_file = fopen(filename, "r");
  if (!graph_file)
    err(1, "unable to open %s", filename);
  struct WordGraph *g = malloc(sizeof *g);
  if (fscanf(graph_file, "%d ", &g->n_words) != 1)
    err(1, "corrupted wordgraph file");
  g->n_prefixes = 0;
  g->words = calloc(g->n_words, sizeof g->words[0]);
  g->followers_compressed = calloc(g->n_words, sizeof g->words[0]);
  for (i = 1; i < g->n_words; i++) {
    getline_trimmed(&g->words[i], graph_file);
    /* add word to a prefix group */
    for (j = 0; j <= g->n_prefixes; ++j) {
      if (j == g->n_prefixes) {
        /* none found, need to insert */
        if (g->n_prefixes == MAX_PREFIXES)
          errx(2, "corrupted wordgraph file: too many prefixes");
        g->n_prefixes++;
        memcpy(g->prefixes[j].prefix, g->words[i], PREFIX_LEN);
        g->prefixes[j].words = intvec_alloc();
      }
      if (!memcmp(g->words[i], g->prefixes[j].prefix, PREFIX_LEN)) {
        intvec_append(g->prefixes[j].words, i);
        break;
      }
    }
  }
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

/* decode an int sequence encoded into a printable string */
struct IntVec *decode(char *enc) {
  // Cf. decode in digest.py
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

int main() {
  struct WordGraph *g = wordgraph_init("wordlist_bigrams.txt");

  int count = 32;
  int length = 5;
  int i, j;
  // wordgraph_dump(g, 1, 3000)
  int fd_crypto;
  if ((fd_crypto = open("/dev/urandom", O_RDONLY)) < 0)
    err(5, "unable to get secure random numbers");

  printf("Generating %d passwords with %d bits of entropy\n", count,
         length * 10);
  int pass_len = length * 3;
  printf("%-*s    %s\n", pass_len, "Password", "Mnemonic");
  for (i = 0; i < pass_len; i++)
    putchar('-');
  printf("    ");
  for (i = 0; i < length * 4; i++)
    putchar('-');
  printf("\n");

  while (count--) {
    /* pick series of prefixes that will make up the passwords */
    int prefixes_chosen[length];
    if (read(fd_crypto, prefixes_chosen, sizeof(prefixes_chosen)) !=
        (size_t)sizeof(prefixes_chosen))
      err(6, "unable to read random numbers");
    /* find possible words for each of the chosen prefixes */
    struct IntVec *word_sets[length];
    for (i = 0; i < length; i++) {
      prefixes_chosen[i] &= MAX_PREFIXES - 1;
      printf("%.3s", g->prefixes[prefixes_chosen[i]].prefix);
      word_sets[i] = intvec_copy(g->prefixes[prefixes_chosen[i]].words);
    }

    printf("    ");

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
    int last_word = 0;
    for (i = 0; i < length; i++) {
      followers = decode(g->followers_compressed[last_word]);
      intersect = intvec_intersect(word_sets[i], followers);
      last_word = intvec_get(intersect->len ? intersect : word_sets[i], 0);
      printf("%s ", g->words[last_word]);
      intvec_free(followers);
      intvec_free(intersect);
    }

    for (i = 0; i < length; i++) {
      intvec_free(word_sets[i]);
    }

    printf("\n");
  }

  wordgraph_free(g);

  return 0;
}

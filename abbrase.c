#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

struct WordGraph {
  int n_words;
  char **words;
  char **followers_compressed;
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
  int i;
  FILE *graph_file = fopen(filename, "r");
  if (!graph_file)
    err(1, "unable to open %s", filename);
  struct WordGraph *g = malloc(sizeof *g);
  if (fscanf(graph_file, "%d", &g->n_words) != 1)
    err(1, "corrupted wordgraph file");
  g->words = calloc(g->n_words, sizeof g->words[0]);
  g->followers_compressed = calloc(g->n_words, sizeof g->words[0]);
  for (i = 0; i < g->n_words; i++)
    getline_trimmed(&g->words[i], graph_file);
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

  // wordgraph_dump(g, 1, 3000)

  wordgraph_free(g);

  return 0;
}

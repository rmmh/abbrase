#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <unistd.h>

const uint32_t trigrams[] = {
    0,         33518332,  35951138,  35951394,  44335650,  58685182,  2396706,
    35967778,  35684898,  1896600,   2130466,   53794,     35684898,  35684898,
    2130466,   33387102,  36475682,  2097152,   2130722,   45349674,  44335906,
    1929216,   2130466,   295714,    0,         2130466,   33314,     0,
    0,         36492855,  37657387,  49133567,  794641,    1086051,   2945699,
    557091,    6058136,   32770,     34079267,  41794303,  36565543,  108714943,
    0,         37589799,  2097152,   48234495,  37468971,  104657775, 18706589,
    33314,     547447,    33315,     8958583,   67142178,  0,         45963932,
    33591842,  32770,     2097664,   48133115,  0,         0,         32768,
    6150399,   2097184,   0,         35684898,  544,       32800,     62780055,
    0,         0,         2130466,   3179049,   299042,    68973276,  32,
    0,         0,         1638401,   0,         0,         8222941,   0,
    2396962,   0,         10318455,  0,         0,         45937527,  73265407,
    0,         46003063,  2130466,   0,         0,         117306367, 0,
    2097152,   35684898,  1,         2945635,   1962620,   0,         0,
    0,         1048589,   0,         0,         117273309, 299010,    2097154,
    33853986,  31323135,  2130434,   8736,      32800,     75362558,  2129920,
    0,         33587746,  33314,     34,        100528825, 262658,    0,
    35684898,  1094441,   256,       913454,    546,       33314,     0,
    541217,    0,         0,         83720413,  2396710,   37526314,  36491959,
    98401117,  37524067,  2945699,   2130466,   6060248,   2129954,   528929,
    41529983,  2748967,   75364347,  4026520,   4035363,   2097152,   50200575,
    45853499,  45912943,  6119440,   33314,     824119,    3244843,   565797,
    67109408,  0,         16546444,  0,         0,         0,         3961403,
    36475427,  0,         0,         85741822,  0,         0,         35684898,
    0,         0,         28103306,  0,         0,         2130466,   1,
    34079265,  68972728,  0,         0,         0,         513,       0,
    0,         113078941, 32768,     0,         0,         43840307,  0,
    33853986,  10015271,  73199806,  0,         0,         35684898,  34,
    565795,    8180407,   0,         0,         2130466,   1048833,   256,
    1896994,   0,         2,         0,         286725,    0,         0,
    117275389, 32770,     0,         32768,     31322747,  2097152,   0,
    32768,     73267455,  0,         0,         33587744,  34,        33312,
    16644799,  512,       0,         2130466,   1048577,   34100067,  1863900,
    0,         32802,     0,         1650705,   0,         0,         1929373,
    2888231,   37526315,  44618419,  16535769,  36737632,  11334563,  512,
    0,         0,         546,       41466623,  2748967,   66846719,  4026521,
    4043555,   2097152,   45988543,  4177919,   104657771, 8192,      4227618,
    512,       1049123,   0,         67142178,  0,         44114572,  0,
    0,         0,         10227752,  0,         0,         0,         1065092,
    0,         0,         0,         0,         0,         36442764,  0,
    0,         0,         0,         0,         1860240,   0,         0,
    0,         0,         0,         0,         35393685,  33282,     0,
    32770,     43831349,  2097664,   0,         32770,     1929276,   0,
    0,         35684896,  2,         33314,     2146304,   33312,     0,
    32770,     1061123,   32768,     69632,     0,         32770,     0,
    4099,      0,         0,         134052541, 33314,     2130722,   823905,
    66974719,  2884129,   546,       32768,     73398527,  0,         524835,
    36299647,  557603,    32,        50328319,  1082146,   0,         33587712,
    1114915,   103580451, 18742014,  546,       2,         0,         77423531,
    0,         0,         121470909, 2920995,   0,         0,         10285247,
    2129920,   0,         0,         85750011,  0,         0,         33554976,
    35684898,  547,       16644767,  37524259,  0,         32768,     1049345,
    0,         1863896,   0,         0,         0,         1572865,   0,
    0,         16612093,  2392610,   36999970,  44954479,  67105535,  2396706,
    37540711,  2130466,   75233791,  2129952,   34361891,  33587746,  32802,
    35684898,  16642813,  299522,    2097152,   2130466,   11662315,  44888867,
    1962750,   2130466,   33538,     32,        8193,      514,       0,
    18708696,  7919143,   3447594,   44602043,  6046229,   3703395,   36500387,
    33314,     1855576,   32,        533027,    41466495,  34202215,  50331647,
    73235197,  37597987,  2097152,   46004991,  37331755,  37589807,  1921212,
    2130466,   1700663,   33554977,  533027,    33312,     0,         50295485,
    294912,    32768,     2,         10342651,  0,         0,         35426851,
    85850362,  0,         0,         35684898,  34,        32800,     33422008,
    35951138,  0,         2130466,   36733731,  2654755,   1929436,   0,
    2,         0,         262657,    0,         0,         0,         0,
    0,         0,         0,         0,         0,         0,         0,
    0,         0,         0,         0,         0,         0,         0,
    0,         0,         0,         0,         33314,     0,         0,
    0,         0,         0,         0,         117276669, 2920995,   2396962,
    2921251,   67108863,  2658851,   35951138,  33587746,  75233535,  2097186,
    528931,    34112051,  36213539,  2663011,   66977791,  2921251,   2097152,
    35685154,  3261227,   111997935, 1667838,   33314,     295202,    0,
    9040419,   0,         0,         50166493,  2129954,   2396962,   546,
    67073023,  2130466,   2392064,   45412911,  90042622,  2097152,   36176419,
    35684898,  2654755,   2130466,   12448511,  36475683,  2097152,   2129952,
    45216623,  44954471,  881406,    32,        33314,     0,         553481,
    0,         0,         66943709,  295458,    299266,    32768,     29225215,
    2130434,   262146,    44954231,  73398527,  2,         0,         33587746,
    32802,     544,       29227775,  299522,    0,         35684898,  3154729,
    36475427,  1962622,   0,         295458,    0,         340481,    545,
    0,         6050460,   4044607,   3443498,   34079411,  1855607,   4160,
    2642851,   0,         73224250,  0,         32,        7994107,   2744935,
    16776191,  3408384,   37589807,  0,         41810687,  37313327,  79557631,
    8192,      544,       0,         2097185,   0,         67108896,  0,
    4027037,   0,         0,         0,         52196019,  0,         0,
    0,         73191678,  0,         0,         0,         0,         0,
    45357576,  0,         0,         0,         0,         0,         4096,
    512,       0,         0,         1,         0,         0,         56392412,
    294946,    0,         266784,    1856054,   2097152,   0,         33314,
    6123768,   0,         512,       524833,    2,         524833,    10809344,
    0,         0,         33314,     66337,     33024,     0,         0,
    0,         0,         1,         0,         0,         1056904,   0,
    2396706,   0,         811032,    0,         0,         33282,     1634362,
    0,         0,         0,         0,         0,         278528,    2396706,
    0,         0,         0,         2392610,   262146,    0,         0,
    0,         1,         0,         0,         8681480,   32802,     37154,
    262658,    1839123,   2097152,   294914,    32768,     540704,    0,
    0,         37410,     123430,    1081898,   2375824,   1622818,   0,
    33282,     1057315,   800,       8,         0,         32802,     0,
    0,         32,        0,         1851393,   0,         0,         0,
    806931,    0,         0,         0,         86048,     0,         0,
    544,       0,         0,         49152,     0,         0,         0,
    0,         0,         0,         0,         0,         0,         1,
    33559075};

#define BIG_WORDS 16
typedef struct {
  uint32_t val[BIG_WORDS];
} big;

void big_iadd(big *dst, big *src) {
  int carry = 0;
  for (int i = 0; i < BIG_WORDS; i++) {
    uint64_t tmp =
        (uint64_t)dst->val[i] + (uint64_t)src->val[i] + +(uint64_t)carry;
    dst->val[i] = (uint32_t)tmp;
    carry = (tmp >> 32) & 1;
  }
  if (carry) errx(5, "addition overflowed");
}

void big_isub(big *dst, big *src) {
  int borrow = 0;
  for (int i = 0; i < BIG_WORDS; i++) {
    int64_t tmp = (int64_t)dst->val[i] - (int64_t)src->val[i] - borrow;
    if (tmp < 0) {
      borrow = 1;
      tmp += 1L << 32;
    } else {
      borrow = 0;
    }
    dst->val[i] = tmp;
  }
  if (borrow) errx(5, "subraction underflowed");
}

bool big_lte(big *a, big *b) {
  for (int i = BIG_WORDS - 1; i >= 0; i--) {
    if (a->val[i] != b->val[i]) {
      return a->val[i] <= b->val[i];
    }
  }
  return true;  // equal
}

void big_irand(big *dst, big *below) {
  while (1) {
    if (getrandom(dst, sizeof *dst, 0) < 0) err(6, "unable to get randomness");
    for (int i = BIG_WORDS - 1; i >= 0; i--) {
      if (below->val[i] == 0) {
        dst->val[i] = 0;
      } else {
        dst->val[i] &= (1L << (32L - __builtin_clz(below->val[i]))) - 1;
        break;
      }
    }
    if (big_lte(dst, below)) return;
  }
}

int big_idiv(big *dst, int divisor) {
  int remainder = 0;
  for (int i = BIG_WORDS - 1; i >= 0; i--) {
    int64_t tmp = (int64_t)dst->val[i] + ((int64_t)(remainder) << 32);
    dst->val[i] = tmp / divisor;
    remainder = tmp % divisor;
  }
  return remainder;
}

void big_print(big a) {
  int chunks[BIG_WORDS * 4];
  int c = 0;
  big zero = {};
  while (!big_lte(&a, &zero)) chunks[c++] = big_idiv(&a, 1000);
  for (int i = c - 1; i >= 0; i--)
    printf("%0*d%s", i == c - 1 ? 1 : 3, chunks[i], i > 0 ? "," : "");
}

big prepare_path_counts(big paths[][27 * 27], int length) {
  for (int i = 0; i < 27 * 27; i++)
    paths[length - 1][i] = (big){{trigrams[i] & 1}};

  for (int level = length - 2; level > 0; level--) {
    for (int i = 27; i < 27 * 27; i++) {
      big count = {};
      for (int j = 1; j < 27; j++) {
        if (trigrams[i] & (1 << j)) {
          big_iadd(&count, &paths[level + 1][(i % 27) * 27 + j]);
        }
      }
      paths[level][i] = count;
    }
  }

  big total_paths = {};
  for (int i = 0; i < 27; i++) {
    big count = {};
    for (int j = 1; j < 27; j++) {
      if (trigrams[i] & (1 << j)) {
        big_iadd(&count, &paths[1][(i % 27) * 27 + j]);
      }
    }
    paths[0][i] = count;
    big_iadd(&total_paths, &count);
  }

  return total_paths;
}

void generate(char *out, int length, big paths[][27 * 27], big chosen) {
  int grams[length];
  for (int i = 0; i < 27 * 27; i++) {
    if (big_lte(&paths[0][i], &chosen)) {
      big_isub(&chosen, &paths[0][i]);
    } else {
      grams[0] = i;
      break;
    }
  }
  for (int level = 1; level < length; level++) {
    int base = grams[level - 1] % 27 * 27;
    for (int i = 0; i < 27; i++) {
      if (trigrams[grams[level - 1]] & (1 << i)) {
        int n = base + i;
        if (big_lte(&paths[level][n], &chosen)) {
          big_isub(&chosen, &paths[level][n]);
        } else {
          grams[level] = n;
          break;
        }
      }
    }
  }
  for (int i = 0; i < length; i++) {
    out[i] = (grams[i] % 27) + '`';
  }
  out[length] = 0;
}

void usage(char *name) {
  errx(1, "usage: %s [-halv] [length] [count]\n", name);
}

int main(int argc, char **argv) {
  int c;
  bool all = false;
  bool columns = true;
  bool verbose = false;
  long length = 8;
  long count = 0;

  while ((c = getopt(argc, argv, "halv")) != -1) {
    switch (c) {
      case 'h':
        usage(argv[0]);
        break;
      case 'a':
        all = true;
        break;
      case 'l':
        columns = false;
        break;
      case 'v':
        verbose = true;
        break;
      case '?':
        return 1;
    }
  }

  char *tmp;
  if (optind < argc) {
    length = strtol(argv[optind], &tmp, 10);
    if (errno || *tmp != 0) errx(2, "invalid length %s", argv[optind]);
    optind++;
  }

  if (length > BIG_WORDS * 9)
    errx(3, "length %ld too large, max is %d", length, BIG_WORDS * 9);
  if (length < 3) errx(3, "length %ld too small, min is 3", length);

  if (optind < argc) {
    count = strtol(argv[optind], &tmp, 10);
    if (errno || *tmp != 0) errx(2, "invalid count %s", argv[optind]);
    optind++;
  }

  if (argv[optind] != NULL) usage(argv[0]);

  big paths[length][27 * 27];

  big total = prepare_path_counts(paths, length);
  int bits = 0;
  for (int i = BIG_WORDS - 1; i >= 0; i--) {
    if (total.val[i]) {
      bits = 32 * i + (31 - __builtin_clz(total.val[i]));
      break;
    }
  }

  int words_per_line = 1;
  if (columns) {
    words_per_line = 1 + (80 - length) / (2 + length);
  }
  if (count == 0) count = words_per_line * 8;
  int words = 0;
  char word[length + 1];
  big chosen = {};
  if (all) {
    if (verbose) {
      printf("Generating all ");
      big_print(total);
      printf(" %ld-letter words with %d bits of entropy\n", length, bits);
    }
    big one = {{1}};
    while (1) {
      generate(word, length, paths, chosen);
      fputs(word, stdout);
      if (++words == words_per_line) {
        words = 0;
        putchar('\n');
      } else {
        fputs("  ", stdout);
      }
      big_iadd(&chosen, &one);
      if (big_lte(&total, &chosen)) break;
    }
  } else {
    if (verbose) {
      printf("Generating %ld %ld-letter words with %d bits of entropy (", count,
             length, bits);
      big_print(total);
      printf(" possible)\n");
    }
    for (int i = 0; i < count; i++) {
      big_irand(&chosen, &total);
      generate(word, length, paths, chosen);
      fputs(word, stdout);
      if (++words == words_per_line) {
        words = 0;
        putchar('\n');
      } else if (i + 1 < count) {
        fputs("  ", stdout);
      }
    }
  }
  if (words) putchar('\n');
}

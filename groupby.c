/* group series of tab-separated values by their first column,
outputting the first field and the total of a configurable count field */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    size_t bufsize = 256;
    char* buf = malloc(bufsize);
    char last[512];
    long long total = 0;
    int count_field;
    if (argc != 2)
        errx(1, "usage: %s <count_field>", argv[0]);
    if ((count_field = atoi(argv[1])) < 2)
        errx(1, "count_field must be at least 2");

    while (!feof(stdin)) {
        int field_index = 0;
        getline(&buf, &bufsize, stdin);

        char *tab = buf;
        while (++field_index != count_field && *tab) {
            while (*tab && *tab != '\t')
                tab++;
            *tab++ = 0;
        }
        if (field_index != count_field)
            continue;

        if (strcmp(buf, last)) {
            if (total)
                printf("%s\t%lld\n", last, total);
            strncpy(last, buf, sizeof(last)-1);
            total = 0;
        }

        total += atoll(tab);
    }

    if (total)
        printf("%s\t%lld\n", last, total);
}

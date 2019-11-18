#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bench.c"
#include "bloom.h"
#include "tst.h"

#define TableSize 5000000 /* size of bloom filter */
#define HashNumber 2      /* number of hash functions */

/** constants insert, delete, max word(s) & stack nodes */
enum { INS, DEL, WRDMAX = 256, STKMAX = 512, LMAX = 1024 };

#define REF INS
#define CPY DEL

#define BENCH_TEST_FILE "bench_cpy.txt"

long poolsize = 2000000 * WRDMAX;

/* simple trim '\n' from end of buffer filled by fgets */
static void rmcrlf(char *s)
{
    size_t len = strlen(s);
    if (len && s[len - 1] == '\n')
        s[--len] = 0;
}

#define IN_FILE "cities.txt"

int main(int argc, char **argv)
{
    char word[WRDMAX] = "";
    char *sgl[LMAX] = {NULL};
    tst_node *root = NULL, *res = NULL;
    int idx = 0, sidx = 0;
    // int rtn = 0;
    double t1, t2;

    FILE *fp = fopen(IN_FILE, "r");

    if (!fp) { /* prompt, open, validate file for reading */
        fprintf(stderr, "error: file open failed '%s'.\n", IN_FILE);
        return 1;
    }

    t1 = tvgetf();

    bloom_t bloom = bloom_create(TableSize);

    char buf[WRDMAX];
    while (fgets(buf, WORDMAX, fp)) {
        char *token = ",";
        char *s = strtok(buf, token);
        char *p = s;
        if (!tst_ins_del(&root, p, INS, CPY)) {
            fprintf(stderr, "error: memory exhausted, tst_insert.\n");
            fclose(fp);
            return 1;
        }
        bloom_add(bloom, p);
        idx++;
    }

    t2 = tvgetf();

    fclose(fp);
    printf("ternary_tree, loaded %d words in %.6f sec\n", idx, t2 - t1);

    if (argc == 2 && strcmp(argv[1], "--bench") == 0) {
        int stat = bench_test(root, BENCH_TEST_FILE, LMAX);
        tst_free_all(root);
        return stat;
    }

    FILE *output;
    output = fopen("cpy.txt", "a");
    if (output != NULL) {
        fprintf(output, "%.6f\n", t2 - t1);
        fclose(output);
    } else
        printf("open file error\n");

    for (;;) {
        printf(
            "\nCommands:\n"
            " a  add word to the tree\n"
            " f  find word in tree\n"
            " s  search words matching prefix\n"
            " d  delete word from the tree\n"
            " q  quit, freeing all data\n\n"
            "choice: ");

        if (argc > 1 && strcmp(argv[1], "--bench") == 0)  // a for auto
            strcpy(word, argv[2]);
        else
            fgets(word, sizeof word, stdin);

        switch (*word) {
        case 'a':
            printf("enter word to add: ");
            if (argc > 1 && strcmp(argv[1], "--bench") == 0)
                strcpy(word, argv[3]);
            else if (!fgets(word, sizeof word, stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(word);
            t1 = tvgetf();
            if (bloom_test(bloom, word)) /* if detected by filter, skip */
                res = NULL;
            else { /* update via tree traversal and bloom filter */
                bloom_add(bloom, word);
                res = tst_ins_del(&root, word, INS, CPY);
            }
            t2 = tvgetf();
            if (res) {
                idx++;
                printf("  %s - inserted in %.12f sec. (%d words in tree)\n",
                       (char *) res, t2 - t1, idx);
            } else
                printf("  %s - already exists in list.\n", (char *) res);

            if (argc > 1 && strcmp(argv[1], "--bench") == 0)  // a for auto
                goto quit;
            break;
        case 'f':
            printf("find word in tree: ");
            if (!fgets(word, sizeof word, stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(word);
            t1 = tvgetf();

            if (bloom_test(bloom, word)) {
                t2 = tvgetf();
                printf("  Bloomfilter found %s in %.6f sec.\n", word, t2 - t1);
                printf(
                    "  Probability of false positives:%lf\n",
                    pow(1 - exp(-(double) HashNumber /
                                (double) ((double) TableSize / (double) idx)),
                        HashNumber));
                t1 = tvgetf();
                res = tst_search(root, word);
                t2 = tvgetf();
                if (res)
                    printf("  ----------\n  Tree found %s in %.6f sec.\n",
                           (char *) res, t2 - t1);
                else
                    printf("  ----------\n  %s not found by tree.\n", word);
            } else
                printf("  %s not found by bloom filter.\n", word);
            break;
        case 's':
            printf("find words matching prefix (at least 1 char): ");

            if (argc > 1 && strcmp(argv[1], "--bench") == 0)
                strcpy(word, argv[3]);
            else if (!fgets(word, sizeof word, stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(word);
            t1 = tvgetf();
            res = tst_search_prefix(root, word, sgl, &sidx, LMAX);
            t2 = tvgetf();
            if (res) {
                printf("  %s - searched prefix in %.6f sec\n\n", word, t2 - t1);
                for (int i = 0; i < sidx; i++)
                    printf("suggest[%d] : %s\n", i, sgl[i]);
            } else
                printf("  %s - not found\n", word);

            if (argc > 1 && strcmp(argv[1], "--bench") == 0)  // a for auto
                goto quit;
            break;
        case 'd':
            printf("enter word to del: ");
            if (!fgets(word, sizeof word, stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(word);
            printf("  deleting %s\n", word);
            t1 = tvgetf();
            res = tst_ins_del(&root, word, DEL, CPY);
            t2 = tvgetf();
            if (res)
                printf("  delete failed.\n");
            else {
                printf("  deleted %s in %.6f sec\n", word, t2 - t1);
                idx--;
            }
            break;
        case 'q':
            goto quit;
        default:
            fprintf(stderr, "error: invalid selection.\n");
            break;
        }
    }

quit:
    tst_free_all(root);
    bloom_free(bloom);
    return 0;
}

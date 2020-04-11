#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wordCount.h"

/*
 * wordPair combines two words into a single string
 * with a space between the two words and returns the
 * string back to main to be inserted into the hash table
 */
char *wordPair(char *s1, char *s2) {
    int SIZE = (strlen(s1) + strlen(s2) + 1);
    char string[SIZE];

    strcpy(string, s1);
    strcat(string, " ");
    strcat(string, s2);

    return strdup(string);
}

/*
 * wordFrequency allocates a new node that is then
 * used as a 1d array to store a pointer to each of the
 * nodes stored in the hash table. The array is then sorted
 * with the qsort function in descending order.
 */
node *wordFrequency(hashTable *h) {
    node *newList = (node *) malloc(sizeof (node) * (h->maxTableSize + h->collisions));
    int jj = 0;

    for (int ii = 0; ii < h->maxTableSize; ii++) {
        node *current = h->table[ii];
        if (current != NULL) {
            newList[jj++] = *current;
            while (current->next != NULL) {
                current = current->next;
                newList[jj++] = *current;
            }
        }
    }

    qsort(newList, (size_t) jj, sizeof(node), comparator);

    return newList;
}

/*
 * comparator is used by qsort to sort the array
 * into descending order
 */
int comparator(const void *a, const void *b) {
    return (*(node *)b).count - (*(node *)a).count;
}

/*
 * printWords takes the sorted array and the specified count
 * and prints the word pairs and the number of times they
 * occured to stdout
 */
void printWords(node *list, int count) {
    node *current;

    for (int ii = 0; ii < count; ii++) {
        current = &list[ii];
        fprintf(stdout, "%10d %s\n", current->count, current->key);
    }

    free(list);
}
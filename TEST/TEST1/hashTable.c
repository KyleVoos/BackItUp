#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crc64.h"
#include "hashTable.h"
#include "wordCount.h"

#define CRC64_REV_POLY      0x95AC9329AC4BC9B5ULL
#define CRC64_INITIALIZER   0xFFFFFFFFFFFFFFFFULL
#define CRC64_TABLE_SIZE    256

// newTable allocates memory for the hash table
hashTable *newTable (int tableSize) {
    hashTable *table = (hashTable *) malloc(sizeof (hashTable));
    table->table = (node**) malloc(sizeof(node *) * tableSize);

    if (tableSize != 0) {
        table->maxTableSize = tableSize;
        table->currentTableSize = 0;
        table->collisions = 0;
    }

    for(int ii = 0; ii < tableSize; ii++) table->table[ii] = NULL;

    return table;
}
// newNode allocates memory for a new node
node *newNode(char *key, int count) {
    node *n = (node *) malloc(sizeof (node));
    n->key = strdup(key);
    n->count = count;
    n->next = NULL;

    return n;
}

/*
 * getIndex calculates the correct index to store
 * a pointer to a node in the hash table
 */
unsigned long long getIndex (unsigned long long hash, hashTable *h) {
    return (hash % h->maxTableSize);
}

hashTable *insert(hashTable *h, char *key, int frequency) {
    double loadFactor = (double) h->currentTableSize/h->maxTableSize; // hash table load factor based on the current and max hash table size
    double performanceMeasure = (double) h->collisions/h->currentTableSize; // performance measure based on collisions for larger hash tables
    unsigned long long index = getIndex(crc64(key), h);
    node *current = h->table[index];

    while (current != NULL) {
        if (strcmp(key, current->key) == 0) {
            current->count++;
            return h;
        }
        if (current->next == NULL) {
            node *n = newNode(key, 1);
            current->next = n;
            h->collisions++;
            return h;
        }
        current = current->next;
    }
    current = newNode(key, frequency);
    h->table[index] = current;
    h->currentTableSize++;

    /* rehash everything stored in the hash table based on the load factor or performance measure.
     * The constants that the load and performance measure are compared to were chosen to give a balance between
     * run time and reducing the number of collisions.
     */
    if (loadFactor >= 0.7 || performanceMeasure >= .35) return rehash(h);

    return h;
}

/*
 * if the load factor or the performance measure for the current hash table reaches
 * the constant they are compared against rehash creates a new hash table that is
 * three times larger then uses rehashInsert to calculate a new index for each node
 * which also stores the pointers to each node in the new hash table
 */
hashTable *rehash(hashTable *h) {
    hashTable *newHashTable = newTable(h->maxTableSize * 3);
    node *next = NULL;

    for (int ii = 0; ii < h->maxTableSize; ii++) {
        node *current = h->table[ii];
        if (current != NULL) {
            if (current->next == NULL) rehashInsert(newHashTable, current);
            else {
                next = current->next;
                rehashInsert(newHashTable, current);
                while (next->next != NULL) {
                    current = next;
                    if (current->next != NULL) {
                        next = current->next;
                    }
                    newHashTable = rehashInsert(newHashTable, current);
                }
                newHashTable = rehashInsert(newHashTable, next);
            }
        }
    }

    deleteTable(h);

    return newHashTable;
}

/*
 * rehashInsert is used by the rehash method to
 * find the a new index for each node stored in
 * the old hash table and inserts a pointer to
 * the node at the correct index in the new hash
 * table
 */
hashTable *rehashInsert(hashTable *h, node *n) {
    unsigned long long index = getIndex(crc64(n->key), h);
    node *current = h->table[index];

    if (current == NULL) {
        h->currentTableSize++;
        n->next = NULL;
        h->table[index] = n;
    }
    else {
        while (current->next != NULL) current = current->next;
        n->next = NULL;
        current->next = n;
        h->collisions++;
    }

    return h;
}

// deleteNode deletes a single node
void deleteNode(node *n) {
    free(n->key);
    n->key = NULL;
    n->next = NULL;
    free(n);
}

// deleteTable deletes the hash table
void deleteTable(hashTable *h) {
    free(h->table);
    h->table = NULL;
    free(h);
}

/*
 * cleanup deletes all the nodes that are stored
 * in the hash table
 */
void cleanup(hashTable *h) {

    for (int ii = 0; ii < (h->maxTableSize); ii++) {
        node *current = h->table[ii];
        if (current != NULL) {
            while (current->next != NULL) {
                node *previous = current;
                current = current->next;
                deleteNode(previous);
            }
            deleteNode(current);
        }
    }
}

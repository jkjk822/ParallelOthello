#ifndef STRUCTS_H_
#define STRUCTS_H_

typedef struct state {
    unsigned long long* board;
    int x;
    int y;
    double alpha;
    double beta;
    double val;
    struct state* next;
} state_t;

typedef struct pair {
    double score;
    int id;
} pair_t;

#endif

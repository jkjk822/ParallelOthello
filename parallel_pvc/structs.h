#ifndef STRUCTS_H_
#define STRUCTS_H_

struct state{
    unsigned long long* board;
    int x;
    int y;
    double alpha;
    double beta;
    double val;
    struct state* next;
};

#endif

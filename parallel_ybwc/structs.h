#ifndef STRUCTS_H_
#define STRUCTS_H_

struct state{
    unsigned long long* board;
    int x;
    int y;
    double val;
    struct state* next;
    state() : board(NULL), x(-1), y(-1), val(0), next(NULL) {}
};

#endif

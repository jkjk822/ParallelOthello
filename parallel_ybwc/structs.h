#ifndef STRUCTS_H_
#define STRUCTS_H_

struct state{
    unsigned long long* board;
    int x;
    int y;
    double val;
    struct state* next;
    state() : board(NULL), x(-1), y(-1), val(0), next(NULL) {
    	board = (unsigned long long*) malloc(sizeof(unsigned long long)*2);
    }
    ~state(){
    	free(board);
    	if(next)
    		delete next;
    }
};

#endif

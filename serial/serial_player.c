/*
 * More ideas at http://www.radagast.se/othello/howto.html
 * Look into transition tables and multi-prob cuts
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <string.h>
#include "structs.h"
#define WHITE 0
#define BLACK 1
#define FALSE 0
#define TRUE 1
#define R0 0 //0 rotation
#define R90 1 //90 degrees right rotation
#define R45 2 //45 degrees right rotation
#define L45 3 //45 degrees left rotation


clock_t gameClock;
clock_t frameClock;
int color;
int guessedDepth = 0;
int depthlimit, timelimit1, timelimit2;
int turn;
int totalStates = 0;
int times[] = {10, 10,10,10,10,20,50,100,1000,10000,80000, 200000, 2000000, 2000000};
pair_t* globalBest;

unsigned char mask[8] = {0xffu,0xfeu,0xfcu,0xf8u,0xf0u,0xe0u,0xc0u,0x80u}; //mask out, respectively, no bit, far right bit, far right 2 bits, etc.
unsigned char moveTable[256][256][2]; //stores all moves (by row) based on [white row config][black row config][color to move]
unsigned long long maskTable[8][8][4]; //stores all shift masks for any given move location
unsigned long long gameState[2];


/*
 * Counts bits iteratively
 */
//credit to Gunnar Andersson's wzebra for the idea of this function
unsigned int iter_count(unsigned long long board){
	unsigned int n = 0;
	for (; board != 0; n++, board &= (board - 1) );
	return n;
}

/*
 * Counts bits with bit fiddling
 */
//credit to Gunnar Andersson's wzebra for the idea of this function
unsigned int bit_count(unsigned long long board){
	board = board - ((board >> 1) & 0x5555555555555555u);
	board = (board & 0x3333333333333333u) + ((board >> 2) & 0x3333333333333333u);
	board = (board + (board >> 4)) & 0x0F0F0F0F0F0F0F0Fu;
	return ((board) * 0x0101010101010101u) >> 56;
}

/*
 * Helper function create a new state struct
 */
state_t* new_state() {
    state_t *s = NULL;
    s = malloc(sizeof(state_t));
    s->next = NULL;

    // Zero out the board
    s->board = malloc(sizeof(unsigned long long)*2);
    s->board[WHITE] = 0;
    s->board[BLACK] = 0;

    s->x = -1;
    s->y = -1;
    s->alpha = 0;
    s->beta = 0;
    s->val = 0;
    return s;
}

/*
 * Initializes the board for a new game
 */
void new_game(){
	gameState[WHITE] = 0x0000001008000000;
	gameState[BLACK] = 0x0000000810000000;
}

/*
 * Convert col, row into a long long move
 */
unsigned long long get_move(int col, int row){
	return (long long)0x1u<<(63-(row*8+col));
}

/*
 * Find how much a long long move is shifted by
 */
unsigned get_shift(unsigned long long move){
	int n = 0;
	for(;move;n++, move>>=1);
	return n-1;
}

/*
 * Compute white moves (for moveTable) based on row config w(hite) and b(lack)
 */
unsigned char compute_white_moves(unsigned char w, unsigned char b){
	if(!(w^b)) //no pieces in this area or pieces occupy the same space
		return 0;

	unsigned char moves = ~(w|b); //can't play in occupied slots

	//must play across black from white
	unsigned char lmoves = 0;
	unsigned char t = moves & b<<1;
	unsigned char t1 = 0;
	for(unsigned char i = 1; i<8; i++){ //look right
		t >>= 1;
		t1 = t&b;
		if(t ^ t1)
			lmoves |= ((t ^ t1) & w)<<i;
		t = t1;
	}
	t = moves & b>>1;
	for(unsigned char i = 1; i<8; i++){ //look left
		t <<= 1;
		t1 = t&b;
		if(t ^ t1)
			lmoves |= ((t ^ t1) & w)>>i;
		t = t1;
	}
	return (moves & lmoves);
}

/*
 * Compute black moves (for moveTable) based on row config i(white) and j(black)
 * (done by pretending colors are reversed and finding white moves)
 */
unsigned char compute_black_moves(unsigned char i, unsigned char j){
	return compute_white_moves(j, i);
}

/*
 * Fill moveTable (pre-computation)
 */
void compute_all_moves(unsigned char moves[256][256][2]){
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			//Compute all legal white moves given i and j
			moves[i][j][WHITE] = compute_white_moves(i,j);

			//Compute all legal black moves given i and j
			moves[i][j][BLACK] = compute_black_moves(i,j);
		}
	}
}

/*
 * Calculate all masks for child generation (pre-computation)
 */
void calculate_masks(unsigned long long shiftMasks[8][8][4]){
	for(int i = 0; i<8; i++){
		for(int j = 0; j<8; j++){
			int dif = j-i<0?j-i+8:j-i;
			int sum = j+i>7?j+i-8:j+i;
			int sign = j-i<0?0:1; //diagonal partial mask
			int off = j+i>7?0:1; //diagonal partial mask

			shiftMasks[i][j][R0] = (unsigned long long)0xffu<<((7-j)*8); //R0 mask
			shiftMasks[i][j][R90] = (unsigned long long)0xffu<<((7-i)*8); //R90 mask
			if(sign)
				shiftMasks[i][j][R45] = (unsigned long long)mask[dif]<<((7-dif)*8); //R45 mask
			else
				shiftMasks[i][j][R45] = (unsigned long long)(~mask[dif]&0xffu)<<((7-dif)*8); //R45 mask other part diagonal
			if(off)
				shiftMasks[i][j][L45] = (unsigned long long)mask[7-sum]<<((7-sum)*8); //L45 mask
			else
				shiftMasks[i][j][L45] = (unsigned long long)(~mask[7-sum]&0xffu)<<((7-sum)*8); //L45 mask other part diagonal
		}
	}
}

/*
 * Rotate board by 90 degrees to the right
 */
unsigned long long r90(unsigned long long board){
	unsigned long long rBoard = 0;
	unsigned long long w = 0x1u; //needed to make b be 64 bits
	for (unsigned char i = 0; i < 8; i++) {
		for (unsigned char j = 0; j < 8; j++) {
			int x = 63-(j*8+i); //convert i,j to 1D
			unsigned long long b = (board & (w<<x)) != 0; //get bit at i,j
			int y = 63-(7-j+i*8); //convert 7-j, i to 1D
			rBoard |= (b<<y); //set 7-j, i to bit b
		}
	}
	return rBoard;
}

/*
 * Rotate board by 90 degrees to the left
 */
unsigned long long l90(unsigned long long board){
	unsigned long long rBoard = 0;
	unsigned long long w = 0x1u; //needed to make b be 64 bits

	for (unsigned char i = 0; i < 8; i++) {
		for (unsigned char j = 0; j < 8; j++) {
			int x = 63-(j*8+i); //convert i,j to 1D
			unsigned long long b = (board & (w<<x)) != 0; //get bit at i,j
			int y = 63-((7-i)*8+j); //convert j, 7-i to 1D
			rBoard |= (b<<y); //set j, 7-i to bit b
		}
	}
	return rBoard;
}

/*
 * Rotate board by 45 degrees to the right
 */
unsigned long long r45(unsigned long long board){
	unsigned long long rBoard = 0;
	unsigned long long w = 0x1u; //needed to make b be 64 bits

	for (unsigned char i = 0; i < 8; i++) {
		for (unsigned char j = 0; j < 8; j++) {
			int x = 63-(j*8+i); //convert i,j to 1D
			unsigned long long b = (board & (w<<x)) != 0; //get bit at i,j
			int z = j-i;
			if(z < 0)
				z += 8;
			int y = 63-(z*8+i);
			rBoard |= (b<<y);
		}
	}
	return rBoard;
}

/*
 * Rotate board by 45 degrees to the left
 */
unsigned long long l45(unsigned long long board){
	unsigned long long rBoard = 0;
	unsigned long long w = 0x1u; //needed to make b be 64 bits

	for (unsigned char i = 0; i < 8; i++) {
		for (unsigned char j = 0; j < 8; j++) {
			int x = 63-(j*8+i); //convert i,j to 1D
			unsigned long long b = (board & (w<<x)) != 0; //get bit at i,j
			int z = j+i;
			if(z > 7)
				z -= 8;
			int y = 63-(z*8+i);
			rBoard |= (b<<y);
		}
	}
	return rBoard;
}

/*
 * Computes all other rotations based on boards in board[WHITE][R0] and board[BLACK][R0]
 * and then stores them
 */
void compute_rotations(unsigned long long board[2][4]){
	board[WHITE][R90] = r90(board[WHITE][R0]);
	board[WHITE][R45] = r45(board[WHITE][R0]);
	board[WHITE][L45] = l45(board[WHITE][R0]);

	board[BLACK][R90] = r90(board[BLACK][R0]);
	board[BLACK][R45] = r45(board[BLACK][R0]);
	board[BLACK][L45] = l45(board[BLACK][R0]);
}

/*
 * Generate moves using moveTable, current board, and color to move
 */
unsigned long long _generate_moves(unsigned long long board[2][4], int color){

	unsigned long long boardR0 = 0;
	unsigned long long boardR90 = 0;
	unsigned long long boardR45 = 0;
	unsigned long long boardL45 = 0;

	for(unsigned char i = 0; i<8; i++){ //must cast to long long because fuck C (all untyped numbers are int by default
		boardR0 |= (long long)moveTable[(board[WHITE][R0]>>(8*(7-i)))&mask[0]][(board[BLACK][R0]>>(8*(7-i)))&mask[0]][color] << (8*(7-i));
		boardR90 |= (long long)moveTable[(board[WHITE][R90]>>(8*(7-i)))&mask[0]][(board[BLACK][R90]>>(8*(7-i)))&mask[0]][color] << (8*(7-i));
		boardR45 |= ((long long)moveTable[(board[WHITE][R45]>>(8*(7-i)))&mask[i]][(board[BLACK][R45]>>(8*(7-i)))&mask[i]][color]&mask[i]) << (8*(7-i));
		boardL45 |= ((long long)moveTable[(board[WHITE][L45]>>(8*(7-i)))&mask[7-i]][(board[BLACK][L45]>>(8*(7-i)))&mask[7-i]][color]&mask[7-i]) << (8*(7-i));
		boardR45 |= ((long long)moveTable[(board[WHITE][R45]>>(8*(7-i)))&(~mask[i]&mask[0])][(board[BLACK][R45]>>(8*(7-i)))&(~mask[i]&mask[0])][color]&(~mask[i]&mask[0])) << (8*(7-i));
		boardL45 |= ((long long)moveTable[(board[WHITE][L45]>>(8*(7-i)))&(~mask[7-i]&mask[0])][(board[BLACK][L45]>>(8*(7-i)))&(~mask[7-i]&mask[0])][color]&(~mask[7-i]&mask[0])) << (8*(7-i));
	}
	return boardR0|l90(boardR90)|l45(boardR45)|r45(boardL45);

}

/*
 * Generate moves using moveTable, current board, and color to move
 */
unsigned long long generate_moves(unsigned long long currBoard[2], int color){
	unsigned long long board[2][4];
	board[WHITE][R0] = currBoard[WHITE];
	board[BLACK][R0] = currBoard[BLACK];
	compute_rotations(board);

	return _generate_moves(board, color);
}

/*
 * Calculate value of a board position
 */
double heuristics(unsigned long long board[2], int color){
	double corner = 801.724;
	double closeCorner = 382.026;
	double piece = 10;
	double mobile = 78.922;

	//Find piece diff
	double playerPieces = bit_count(board[color]);
	double opponentPieces = bit_count(board[abs(color-1)]);
	double pieceDiff = 0;

	if(playerPieces+opponentPieces != 0) {
		pieceDiff = 100*(playerPieces-opponentPieces)/(playerPieces+opponentPieces);
	}

	//Find mobility diff
	unsigned long long rBoard[2][4];
	rBoard[WHITE][R0] = board[WHITE];
	rBoard[BLACK][R0] = board[BLACK];

	compute_rotations(rBoard); //pre-compute rotations
	double playerMobil = bit_count(_generate_moves(rBoard, color));
	double opponentMobil = bit_count(_generate_moves(rBoard, abs(color-1)));
	double mobilDiff = 0;

	if(playerMobil+opponentMobil != 0) {
		mobilDiff = 100*(playerMobil-opponentMobil)/(playerMobil+opponentMobil);
	}

	//Find corner diff
	double playerCorner = iter_count(board[color]&0x8100000000000081u);
	double opponentCorner = iter_count(board[abs(color-1)]&0x8100000000000081u);

	double cornerDiff = 0;
	if ((playerCorner+opponentCorner) != 0) {
		cornerDiff = 100*(playerCorner-opponentCorner)/(playerCorner+opponentCorner);
	}

	//Find close corner diff
	//TODO: see if this calculation method is correct
	double playerCloseCorner = bit_count(board[color]&0x42c300000000c342u);
	double opponentCloseCorner = bit_count(board[abs(color-1)]&0x42c300000000c342u);

	double closeCornerDiff = 0;
	if ((playerCloseCorner+opponentCloseCorner) != 0) {
		closeCornerDiff = -12.5*(playerCloseCorner-opponentCloseCorner);
	}

	return (piece*pieceDiff + mobile*mobilDiff + corner*cornerDiff + closeCorner*closeCornerDiff);
}

/*
 * Flip all pieces in row defined by w(hite) and b(lack), given move
 */
unsigned long long flip(unsigned long long w, unsigned long long b, unsigned long long move){

	unsigned long long flipped = 0;

	unsigned long long t = move;
	unsigned long long flip = 0;

	if((move>>1&b)||(move<<1&b)){
		//RIGHT
		for(unsigned char i = 0; i<8; i++){ //look right
			t>>=1;
			t&=b;
			if(!t){
				flip = (move>>(i+1))&w;
				break;
			}
		}
		t = move;
		if(flip)
			for(unsigned char i = 0; i<8; i++){ //flip right
				t>>=1;
				t&=b;
				flipped |= t;
				if(!t)
					break;
			}
		//LEFT
		t = move;
		flip = 0;
		for(unsigned char i = 0; i<8; i++){ //look left
				t<<=1;
				t&=b;
				if(!t){
					flip = w&(move<<(i+1));
					break;
				}
			}
		t = move;
		if(flip)
			for(unsigned char i = 0; i<8; i++){ //flip left
				t<<=1;
				t&=b;
				flipped |= t;
				if(!t)
					break;
			}
	}
	return flipped;
}

/*
 * Generate a child given current board, next move, color to move, col of move, and row of move
 */
unsigned long long* generate_child(unsigned long long board[2][4], unsigned long long move, int color, int x, int y){
	unsigned long long* newBoard;
	newBoard = malloc(sizeof(unsigned long long)*2);

	unsigned long long flipped;
	flipped =
			flip(board[color][R0]&maskTable[x][y][R0], board[abs(color-1)][R0]&maskTable[x][y][R0],move)
			|l90(flip(board[color][R90]&maskTable[x][y][R90], board[abs(color-1)][R90]&maskTable[x][y][R90],r90(move)))
			|l45(flip(board[color][R45]&maskTable[x][y][R45], board[abs(color-1)][R45]&maskTable[x][y][R45],r45(move)))
			|r45(flip(board[color][L45]&maskTable[x][y][L45], board[abs(color-1)][L45]&maskTable[x][y][L45],l45(move)));

	newBoard[color] = flipped|move|board[color][0]; //add flipped pieces and this move to players old board
	newBoard[abs(color-1)] = board[abs(color-1)][0]&~flipped; //remove flipped pieces from opponents old board
	return newBoard;
}

/*
 * Update board given current board, next move, and color to move
 */
unsigned long long* update(unsigned long long currBoard[2], unsigned long long move, int color, int x, int y){
	unsigned long long board[2][4];
	board[WHITE][R0] = currBoard[WHITE];
	board[BLACK][R0] = currBoard[BLACK];
	compute_rotations(board);
	return generate_child(board, move, color, x, y);
}

/*
 * Generate all children and store in linked list given by head, with current board, board of possible moves, and color to move
 */
void generate_children(state_t* head, unsigned long long currBoard[2] , unsigned long long moves, int color){

	state_t* previous = head;
	unsigned long long board[2][4];
	board[WHITE][R0] = currBoard[WHITE];
	board[BLACK][R0] = currBoard[BLACK];
	compute_rotations(board);
	while(moves){
		unsigned long long currMove = moves&(~moves+1);
		int temp = get_shift(currMove);
		previous->x = 7-(temp%8);
		previous->y = 7-(temp/8);
		previous->board = generate_child(board, currMove, color, previous->x, previous->y);
		moves&=moves-1;

		state_t* currentState = new_state();
		previous->next = currentState;
		previous = currentState;
		totalStates++;
	}

	state_t* cur = head;
	while (cur->next != NULL) {
	if (cur->next->x == -1) {
        	cur->next = NULL;
		break;
	}
   	cur = cur->next;
   }
   cur = head;
	while (cur != NULL) {
		cur = cur->next;
	}


}

/***********************START Fuck functions***********************/

//Prints msg as error
void error(char * msg){
    fprintf(stderr,"%s\n", msg);
    exit(-1);
}
//Tests if time is up
int timeUp() {
    if (timelimit1 == 0) {
        return FALSE;
    }
    clock_t time = clock() - frameClock;
    int msec = time * 1000 / CLOCKS_PER_SEC;
    return (timelimit1 - msec) < 200;
}
//Tests if the game has ended
int game_over(unsigned long long board[2]){

	return !(generate_moves(board, WHITE)|generate_moves(board, BLACK));
}

/************************END Fuck functions************************/


void sort_children(state_t** node, int player){

    state_t* current = *node;
    while (current != NULL) {
        current->val = heuristics(current->board, player);
        current = current->next;
    }


    current = *node;
    double bestScore = -DBL_MAX;
    state_t* bestNode = NULL;
    while (current != NULL) {
        if (current->val > bestScore) {
            bestScore = current->val;
            bestNode = current;
        }
        current = current->next;
    }

    if (*node == bestNode) {
        return;
    }

    current = *node;
    while (current != NULL) {//TODO: remove print
    	//printf("Current val %.2f\n", current->val);
        if (current->next == bestNode) {
            current->next = bestNode->next;
            bestNode->next = *node;
            break;
        }

        current=current->next;
    }

     *node = bestNode;

}

void free_children(state_t* children) {
    state_t* node = children;
    while (node != NULL) {
        state_t* temp = node;
        node = node->next;
        free(temp);
    }
    children = NULL;
}

double minimax(state_t *node,state_t* bestState, int depth, int currentPlayer,double alpha, double beta, int id) {


    if (timeUp()) {
        return -1;
    }


    double bestResult = -DBL_MAX;
    state_t* gb = new_state();
    if (depth == 0 || game_over(node->board)) {
        return heuristics(node->board, currentPlayer);
    }

    state_t* children = new_state();

    generate_children(children, node->board, generate_moves(node->board, currentPlayer), currentPlayer);

    sort_children(&children, currentPlayer);//TODO: remove print
      //  printf("Children after:");
    //TODO: remove print
   // printChildren(children);
    state_t* current = children;//TODO: remove print
    //printf("is current null?x: %d\n", current->x);
    int p;
    if (depth == 1) {
     	p = 0;
    }else {
        p = id;
    }
    while (current != NULL) {
        if (timeUp()) {

            return -1;
        }
        //recurse on child
        alpha = -minimax(current,gb, depth - 1, abs(currentPlayer-1), -beta, -alpha, p);
        if (alpha == 1 && id == globalBest->id) {
        	bestState->board = current->board;
            bestState->x = current->x;
            bestState->y = current->y;
            return -1;
        }
        if (beta <= alpha) {
            return alpha;
        }
        if (alpha > bestResult)
        {

            globalBest->score = alpha;
            globalBest->id = p;
            bestResult = alpha;
            bestState->board = current->board;
            bestState->x = current->x;
            bestState->y = current->y;
        }

        if (depth == 1)
        p++;

       	//go to next child
        current = current->next;
    }

    free_children(children);


    return bestResult;
}


void make_move(){

    globalBest = malloc(sizeof(struct pair));
    globalBest->id = 0;
	globalBest->score = -DBL_MAX;
    frameClock = clock();
    clock_t beginClock = clock(), deltaClock;


    state_t* initialState = new_state();
    initialState->board = gameState;
   // printf("w:%016I64x\n",initialState->board[WHITE]);//TODO: remove print
   // printf("b:%016I64x\n",initialState->board[BLACK]);

    state_t* bestState = new_state();
    /* Timelimit2 is set - overall game time */
    if (timelimit2 > 0) {
            clock_t timePassed= clock() - gameClock;
            int msec = timelimit2 - (timePassed * 1000 / CLOCKS_PER_SEC);

            int depth = 0;
            for (int i = 0; i < 14; i++) {
                if (times[i] > msec) {
                    depth = i - 2;
                    break;
                }
            }

            minimax(initialState, bestState, depth, color, -DBL_MAX, DBL_MAX,0);
    }

    /* Depthlimit is set - we only search to that depth */
    else if (depthlimit > 0) {

        minimax(initialState, bestState, depthlimit, color, -DBL_MAX, DBL_MAX,0);
    }

    /* Time per move is set */
    else {
        minimax(initialState, bestState, 10, color, -DBL_MAX, DBL_MAX, 0);

/*
        for (int i = 1; i <15; i++) {
            int timeNeeded = times[i+1];
            clock_t start = clock(), diff;
            minimax(initialState, bestState, i, me, -DBL_MAX, DBL_MAX);
            diff = clock() - start;
            int msec2 = diff * 1000 / CLOCKS_PER_SEC;
            printf("Time taken %d seconds %d milliseconds for level %d searching %d states\n", msec2/1000, msec2%1000, i, totalStates);
            deltaClock = clock() - beginClock;
            int msec = deltaClock * 1000 / CLOCKS_PER_SEC;
            int dif = timelimit1 - msec;
            printf("we have %d left, and timeneeded for next level is %d\n", dif, timeNeeded);
            if (timeNeeded > dif) {
                break;
            }
        }
        */

    }


    if (bestState->x == -1) {
        printf("pass\n");
        fflush(stdout);
    } else {
        printf("%d %d\n", bestState->x, bestState->y);
        fflush(stdout);

        unsigned long long* temp = update(gameState, get_move(bestState->x, bestState->y), color, bestState->x, bestState->y);
        gameState[WHITE] = temp[WHITE];
        gameState[BLACK] = temp[BLACK];
    }
}

/*
 * Driver function
 */
int main(){

    char inbuf[256];
    char playerstring[1];
    int x,y;
    turn = 0;
    /*unsigned long long board[2];
	board[WHITE] = 0x3e3efcf8f0e00808;
	board[BLACK] = 0x000000040c1c0000;
	printf("w:%016I64x\n", update(board, get_move(5,7), BLACK, 7,5)[WHITE]);//TODO: remove print
	printf("b:%016I64x\n", update(board, get_move(5,7), BLACK, 7,5)[BLACK]);*/
	//TODO: Test more
    if (fgets(inbuf, 256, stdin) == NULL){
        error("Couldn't read from inpbuf");
    }

    if (sscanf(inbuf, "game %1s %d %d %d", playerstring, &depthlimit, &timelimit1, &timelimit2) != 4) {
        error("Bad initial input");
    }
    if (timelimit1 > 0) {
        for (unsigned char i = 0; i < 14; i++) {
            if (times[i] > timelimit1) {
                guessedDepth = i - 2;
                break;
            }
        }
    }
    if (playerstring[0] == 'B') {
       color = BLACK;
    } else{
       color = WHITE;
    }
    gameClock = clock();
    new_game();
	compute_all_moves(moveTable);
	calculate_masks(maskTable);
    if (color == BLACK) {
        make_move();
    }
    while (fgets(inbuf, 256, stdin) != NULL) {
        if (strncmp(inbuf, "pass", 4) != 0) {
            if (sscanf(inbuf, "%d %d", &x, &y) != 2) {
                return 0;
            }

            unsigned long long* temp = update(gameState, get_move(x,y),abs(color-1), x, y);
            gameState[WHITE] = temp[WHITE];
            gameState[BLACK] = temp[BLACK];
        }
        make_move();
    }
    return 0;
}

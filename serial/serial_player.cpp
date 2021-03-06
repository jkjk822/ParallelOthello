/*
 * More ideas at http://www.radagast.se/othello/howto.html
 * Look into transition tables and multi-prob cuts
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <cfloat>
#include <cstring>
#include <sys/time.h>
#include <string>
#include <iostream>
#include "structs.h"

//Constants
#define WHITE false
#define BLACK true
#define R0 0 //0 rotation
#define L90 0 //90 degrees left rotation (only used to correct R90)
#define R90 1 //90 degrees right rotation
#define R45 2 //45 degrees right rotation
#define L45 3 //45 degrees left rotation

using namespace std;

clock_t gameClock;
clock_t frameClock;
int guessedDepth = 0;
int depthlimit, timelimit1, timelimit2;
int turn;
int totalStates = 0;
int times[] = {10,10,10,10,10,20,50,100,1000,10000,80000, 200000, 2000000, 2000000};

const unsigned char mask[8] = {0xffu,0xfeu,0xfcu,0xf8u,0xf0u,0xe0u,0xc0u,0x80u}; //mask out, respectively, no bit, far right bit, far right 2 bits, etc.
unsigned char moveTable[256][256]; //stores all moves (by row) based on [white row config][black row config] with white to move
unsigned long long rotateTable[256][4]; //stores all moves (by row) based on [white row config][black row config] with white to move
unsigned long long maskTable[8][8][4]; //stores all shift masks for any given move location
unsigned long long gameState[2];

int verbose = false; //Print timings for testing


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
 * Compute moves (for moveTable) based on row config w(hite) and b(lack)
 * with white to move
 */
unsigned char compute_moves(unsigned char w, unsigned char b){
	if(!(w^b)) //no pieces in this area or pieces occupy the same space
		return 0;

	unsigned char moves = ~(w|b); //can't play in occupied slots

	//must play across black from white
	unsigned char lmoves = 0;
	unsigned char t = moves & b<<1;
	unsigned char t1 = 0;
	for(int i = 1; i<8; i++){ //look right
		t >>= 1;
		t1 = t&b;
		if(t ^ t1)
			lmoves |= ((t ^ t1) & w)<<i;
		t = t1;
	}
	t = moves & b>>1;
	for(int i = 1; i<8; i++){ //look left
		t <<= 1;
		t1 = t&b;
		if(t ^ t1)
			lmoves |= ((t ^ t1) & w)>>i;
		t = t1;
	}
	return (moves & lmoves);
}

/*
 * Fill moveTable (pre-computation)
 */
void compute_all_moves(){
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			//Compute all legal moves given i and j
			moveTable[i][j] = compute_moves(i,j);
		}
	}
}

/*
 * Calculate all masks for child generation (pre-computation)
 */
void calculate_masks(){
	for(int i = 0; i<8; i++){
		for(int j = 0; j<8; j++){
			int dif = j-i<0?j-i+8:j-i;
			int sum = j+i>7?j+i-8:j+i;
			int sign = j-i<0?0:1; //diagonal partial mask
			int off = j+i>7?0:1; //diagonal partial mask

			maskTable[i][j][R0] = (unsigned long long)0xffu<<((7-j)*8); //R0 mask
			maskTable[i][j][R90] = (unsigned long long)0xffu<<((7-i)*8); //R90 mask
			if(sign)
				maskTable[i][j][R45] = (unsigned long long)mask[dif]<<((7-dif)*8); //R45 mask
			else
				maskTable[i][j][R45] = (unsigned long long)(~mask[dif]&0xffu)<<((7-dif)*8); //R45 mask other part diagonal
			if(off)
				maskTable[i][j][L45] = (unsigned long long)mask[7-sum]<<((7-sum)*8); //L45 mask
			else
				maskTable[i][j][L45] = (unsigned long long)(~mask[7-sum]&0xffu)<<((7-sum)*8); //L45 mask other part diagonal
		}
	}

}

/*
 * Rotate board by 90 degrees to the right
 */
unsigned long long r90(unsigned long long board){
	unsigned long long rBoard = 0;
	unsigned long long w = 0x1u; //needed to make b be 64 bits
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
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

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
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

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
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

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
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
 * Fill rotateTable (pre-computation)
 */
void calculate_rotations(){
	for (int i = 0; i < 256; i++) {
		//Compute all rotations given row i
		rotateTable[i][L90] = l90(i);
		rotateTable[i][R90] = r90(i);
		rotateTable[i][R45] = r45(i);
		rotateTable[i][L45] = l45(i);
	}

}

/*
 * Rotate board by r
 * 0->left 90; 1->right 90; 2->right 45; 3->left 45
 */
unsigned long long rotate(unsigned long long board, int r){
	unsigned long long new_board = 0;
	switch(r){
	case L90:
		for(int i = 0; i < 8; i++)
			new_board |= rotateTable[(board>>(i*8))&mask[0]][r]<<i;
		break;
	case R90:
		for(int i = 0; i < 8; i++)
			new_board |= rotateTable[(board>>(i*8))&mask[0]][r]>>i;
		break;
	case L45:
	case R45:
		for(int i = 0; i < 64; i+=8) //i increases by 8 each iteration
			new_board |= (rotateTable[(board>>i)&mask[0]][r]<<i)|(rotateTable[(board>>i)&mask[0]][r]>>(64-i));
		break;
	}
	return new_board;
}

/*
 * Computes all other rotations based on boards in board[WHITE][R0] and board[BLACK][R0]
 * and then stores them
 */
void compute_rotations(unsigned long long board[2][4]){

	board[WHITE][R90] = rotate(board[WHITE][R0], R90);
	board[WHITE][R45] = rotate(board[WHITE][R0], R45);
	board[WHITE][L45] = rotate(board[WHITE][R0], L45);

	board[BLACK][R90] = rotate(board[BLACK][R0], R90);
	board[BLACK][R45] = rotate(board[BLACK][R0], R45);
	board[BLACK][L45] = rotate(board[BLACK][R0], L45);
}

/*
 * Wrapper function for moveTable
 */
unsigned char moves(unsigned char w, unsigned char b, bool color){
	if(color == WHITE)
		return moveTable[w][b];
	else //treat black as white
		return moveTable[b][w]; 
}

/*
 * Generate moves using moveTable, current board, and color to move
 */
unsigned long long _generate_moves(unsigned long long board[2][4], bool color){

	unsigned long long boardR0 = 0;
	unsigned long long boardR90 = 0;
	unsigned long long boardR45 = 0;
	unsigned long long boardL45 = 0;

	for(int i = 0; i<8; i++){ //must cast to long long because all untyped numbers are int by default
		boardR0 |= (long long) moves((board[WHITE][R0]>>(8*(7-i)))&mask[0], (board[BLACK][R0]>>(8*(7-i)))&mask[0], color) << (8*(7-i));
		boardR90 |= (long long) moves((board[WHITE][R90]>>(8*(7-i)))&mask[0],(board[BLACK][R90]>>(8*(7-i)))&mask[0], color) << (8*(7-i));
		boardR45 |= ((long long) moves((board[WHITE][R45]>>(8*(7-i)))&mask[i], (board[BLACK][R45]>>(8*(7-i)))&mask[i], color) & mask[i]) << (8*(7-i));
		boardL45 |= ((long long) moves((board[WHITE][L45]>>(8*(7-i)))&mask[7-i], (board[BLACK][L45]>>(8*(7-i)))&mask[7-i], color) & mask[7-i]) << (8*(7-i));
		boardR45 |= ((long long) moves((board[WHITE][R45]>>(8*(7-i)))&(~mask[i]&mask[0]), (board[BLACK][R45]>>(8*(7-i)))&(~mask[i]&mask[0]), color) & (~mask[i]&mask[0])) << (8*(7-i));
		boardL45 |= ((long long) moves((board[WHITE][L45]>>(8*(7-i)))&(~mask[7-i]&mask[0]), (board[BLACK][L45]>>(8*(7-i)))&(~mask[7-i]&mask[0]), color) & (~mask[7-i]&mask[0])) << (8*(7-i));
	}
	return boardR0|rotate(boardR90,L90)|rotate(boardR45,L45)|rotate(boardL45,R45);

}

/*
 * Generate moves using moveTable, current board, and color to move
 */
unsigned long long generate_moves(unsigned long long currBoard[2], bool color){
	unsigned long long board[2][4];
	board[WHITE][R0] = currBoard[WHITE];
	board[BLACK][R0] = currBoard[BLACK];
	compute_rotations(board);

	return _generate_moves(board, color);
}

/*
 * Calculate value of a board position
 */
double heuristics(unsigned long long board[2], bool color){
	double corner = 801.724;
	double closeCorner = 382.026;
	double piece = 10;
	double mobile = 78.922;

	//Find piece diff
	double playerPieces = bit_count(board[color]);
	double opponentPieces = bit_count(board[!color]);
	double pieceDiff = 0;

	pieceDiff = 100*(playerPieces-opponentPieces)/(playerPieces+opponentPieces);

	//Find mobility diff
	unsigned long long rBoard[2][4];
	rBoard[WHITE][R0] = board[WHITE];
	rBoard[BLACK][R0] = board[BLACK];

	compute_rotations(rBoard); //pre-compute rotations
	double playerMobil = bit_count(_generate_moves(rBoard, color));
	double opponentMobil = bit_count(_generate_moves(rBoard, !color));
	double mobilDiff = 0;

	if(playerMobil+opponentMobil != 0) {
		mobilDiff = 100*(playerMobil-opponentMobil)/(playerMobil+opponentMobil);
	}
	else
		return pieceDiff>0?DBL_MAX/1.1:-DBL_MAX/1.1;

	//Find corner diff
	double playerCorner = iter_count(board[color]&0x8100000000000081u);
	double opponentCorner = iter_count(board[!color]&0x8100000000000081u);
	double cornerDiff = 0;

	if ((playerCorner+opponentCorner) != 0) {
		cornerDiff = 100*(playerCorner-opponentCorner)/(playerCorner+opponentCorner);
	}

	//Find close corner diff
	//TODO: see if this calculation method is correct
	double playerCloseCorner = bit_count(board[color]&0x42c300000000c342u);
	double opponentCloseCorner = bit_count(board[!color]&0x42c300000000c342u);
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
		for(int i = 0; i<8; i++){ //look right
			t>>=1;
			t&=b;
			if(!t){
				flip = (move>>(i+1))&w;
				break;
			}
		}
		t = move;
		if(flip)
			for(int i = 0; i<8; i++){ //flip right
				t>>=1;
				t&=b;
				flipped |= t;
				if(!t)
					break;
			}
		//LEFT
		t = move;
		flip = 0;
		for(int i = 0; i<8; i++){ //look left
				t<<=1;
				t&=b;
				if(!t){
					flip = w&(move<<(i+1));
					break;
				}
			}
		t = move;
		if(flip)
			for(int i = 0; i<8; i++){ //flip left
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
 * Stores result in newBoard
 */
void generate_child(unsigned long long* newBoard, unsigned long long board[2][4], unsigned long long move, bool color, int x, int y){

	unsigned long long flipped =
			flip(board[color][R0]&maskTable[x][y][R0], board[!color][R0]&maskTable[x][y][R0], move)
			|rotate(flip(board[color][R90]&maskTable[x][y][R90], board[!color][R90]&maskTable[x][y][R90], rotate(move,R90)),L90)
			|rotate(flip(board[color][R45]&maskTable[x][y][R45], board[!color][R45]&maskTable[x][y][R45], rotate(move,R45)),L45)
			|rotate(flip(board[color][L45]&maskTable[x][y][L45], board[!color][L45]&maskTable[x][y][L45], rotate(move,L45)),R45);

	newBoard[color] = flipped|move|board[color][0]; //add flipped pieces and this move to players old board
	newBoard[!color] = board[!color][0]&~flipped; //remove flipped pieces from opponents old board
}

/*
 * Update board given current board, next move, and color to move
 * Stores result in newBoard
 */
void update(unsigned long long* newBoard, unsigned long long currBoard[2], unsigned long long move, bool color, int x, int y){
	unsigned long long board[2][4];
	board[WHITE][R0] = currBoard[WHITE];
	board[BLACK][R0] = currBoard[BLACK];
	compute_rotations(board);
	generate_child(newBoard, board, move, color, x, y);
}

/*
 * Generate all children and store in linked list given by head, with current board, board of possible moves, and color to move
 */
void generate_children(state* head, unsigned long long currBoard[2], unsigned long long moves, bool color){

	unsigned long long board[2][4];
	board[WHITE][R0] = currBoard[WHITE];
	board[BLACK][R0] = currBoard[BLACK];
	compute_rotations(board);

	while(moves){
		unsigned long long currMove = moves&(~moves+1);
		int temp = get_shift(currMove);
		head->x = 7-(temp%8);
		head->y = 7-(temp/8);
		generate_child(head->board, board, currMove, color, head->x, head->y);
		moves&=moves-1;

		state* currentState = new state();
		head->next = currentState;
		if(moves)
			head = currentState;
		else
			delete currentState;
		totalStates++;
	}
	head->next = NULL;
}

/***********************START special functions***********************/

//Prints msg as error
void error(string msg){
	fprintf(stderr,"%s\n", msg);
	exit(-1);
}

//Tests if the game has ended
int game_over(unsigned long long board[2]){
	return !(generate_moves(board, WHITE)|generate_moves(board, BLACK));
}

/************************END special functions************************/


/*
* Gets the best child to the head of the list (NOT a full sort)
*/
void sort_children(state** node, bool color){

	state* current = *node;
	while (current != NULL) {
		current->val = heuristics(current->board, color);
		current = current->next;
	}

	current = *node;
	double bestScore = -DBL_MAX;
	state* bestNode = NULL;
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
	while (current != NULL) {
		if (current->next == bestNode) {
			current->next = bestNode->next;
			bestNode->next = *node;
			break;
		}

		current=current->next;
	}

	*node = bestNode;
}

/*
* Alpha-Beta negamax
* node - the initial board state
* bestState - stores best move (Only used at the top level)
* depth - how much deeper this call should dive (e.g. starts high gets decremented)
* color - color of player to move (false [White]; true [Black])
* alpha/beta - values used to prune
*/
double minimax(state *node, state* bestState, int depth, bool color, double alpha, double beta) {
	
	//return when depth limit reached, or is actual leaf node
	if (depth == 0 || game_over(node->board)) {
		return heuristics(node->board, color);
	}

	state gb = state(); //throwaway state
	state* children = new state();

	generate_children(children, node->board, generate_moves(node->board, color), color);

	sort_children(&children, color);
	state* current = children;

	while (current != NULL) {
		//recurse on child
		double result = -minimax(current, &gb, depth-1, !color, -beta, -alpha);

		if (result >= beta) { //prune
			delete children;
			return beta;
		}
		if (result > alpha) {
			alpha = result;
			bestState->board[BLACK] = current->board[BLACK];
			bestState->board[WHITE] = current->board[WHITE];
			bestState->x = current->x;
			bestState->y = current->y;
		}

		//go to next child
		current = current->next;
	}

	delete children;
	return alpha;
}

/*
* Repeatedly called in main to progress game
* Initial call to minimax. Gets best move,
* makes it, and updates board.
*/
void make_move(bool color){

	frameClock = clock();
	clock_t beginClock = clock(), deltaClock;

	state* initialState = new state();
	initialState->board[BLACK] = gameState[BLACK];
	initialState->board[WHITE] = gameState[WHITE];

	state* bestState = new state();

	/***IGNORE***/
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
			minimax(initialState, bestState, depth, color, -DBL_MAX, DBL_MAX);
	}

	/***THIS ONE WE ACTUALLY USE***/
	/* Depthlimit is set - we only search to that depth */
	else if (depthlimit > 0) {
		minimax(initialState, bestState, depthlimit, color, -DBL_MAX, DBL_MAX);
	}

	/***IGNORE***/
	/* Time per move is set */
	else {
		minimax(initialState, bestState, 10, color, -DBL_MAX, DBL_MAX);
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
			printf("we have %d left, and time needed for next level is %d\n", dif, timeNeeded);
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
		if(verbose)
			printf("M: %d %d, ", bestState->x, bestState->y);
		else
			printf("%d %d\n", bestState->x, bestState->y);
		fflush(stdout);

		gameState[WHITE] = bestState->board[WHITE];
		gameState[BLACK] = bestState->board[BLACK];
		delete bestState;
		delete initialState;
	}
}

/*
 * Driver function
 */
int main(int argc, char **argv){

	char inbuf[256];
	char playerstring;
	int x,y,c;
	bool color;
	struct timeval start, finish;

	turn = 0;
	new_game(); //Setup default board state

	//Read in initial board state if specified (overwrites new_game)
	while ((c = getopt(argc, argv, "b:w:v::")) != -1)
	switch (c) {
	case 'b':
		gameState[BLACK] = strtoll(optarg, NULL, 16);
		break;
	case 'w':
		gameState[WHITE] = strtoll(optarg, NULL, 16);
		break;
	case 'v':
		verbose = true;
		break;
	default:
		exit(1);
	}

	if (fgets(inbuf, 256, stdin) == NULL){
		error("Couldn't read from inpbuf");
	}
	if (sscanf(inbuf, "game %c %d %d %d", &playerstring, &depthlimit, &timelimit1, &timelimit2) != 4) {
		error("Bad initial input\nusage: game <color> <depthlimit> <total_timelimit> <turn_timelimit>\n" \
		   "    -color: a single char (B/W) representing this player's color. 'W' is default\n" \
		   "    -limits: only specify a single limit and enter 0 for the others");
	}
	if (timelimit1 > 0) {
		for (int i = 0; i < 14; i++) {
			if (times[i] > timelimit1) {
				guessedDepth = i - 2;
				break;
			}
		}
	}
	if (playerstring == 'B' || playerstring == 'b') {
	   color = BLACK;
	} else{
	   color = WHITE;
	}

	gameClock = clock();
	compute_all_moves();
	calculate_rotations();
	calculate_masks();

	if (color == BLACK) {
		gettimeofday(&start, 0);
		make_move(color);
		gettimeofday(&finish, 0);
		if(verbose)
			fprintf(stdout, "Time: %f seconds, ", (finish.tv_sec - start.tv_sec)
				+ (finish.tv_usec - start.tv_usec) * 0.000001);
	}

	while (fgets(inbuf, 256, stdin) != NULL) {
		if (strncmp(inbuf, "pass", 4) != 0) {
			if (sscanf(inbuf, "%d %d", &x, &y) != 2) {
				fprintf(stderr, "Invalid Input\n");
				return 0;
			}
			update(gameState, gameState, get_move(x,y),!color, x, y);
		}
		make_move(color);
	}
	return 0;
}
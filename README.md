# ParallelOthello
A parallel and distributed implementation of Othello

## Implementation

This is a brief overview, for more detailed description and results, see the writeup.

#### Serial

The serial implementation uses nega-max (alpha-beta pruning) and bitboards.

#### Parallel

The parallel implementation used a C++ thread pool (https://github.com/vit-vit/ctpl) and had two versions. The first version used primary variation splitting (PVS) and the second used the younger brothers wait concept (YBWC).

#### Distributed

The distributed implementation used Open MPI (https://www.open-mpi.org/), a message passing library for C/C++. The distributed implementation also had two versions, and the first also used PVS. The second was a custom method we thought fit easily into the distributed framework.

## Running

### With Reversi.jar
This method only works for the serial and parallel versions, *not* distributed.

	usage: tournament <interface> <black_player> <white_player> <depth_limit> <total_timelimit> <turn_timelimit>
		-interface is one of the following strings:
			gui 	Display the game graphically (using X windows and the DISPLAY environmental variable).
			text 	Print a record of the game to stdout
			none 	Nothing is printed except the final winner of the game
		Each of black_player and white_player is either the flag -human or the name of a player program directory (including the path if not in the working directory).
		If a player is human the player either uses the gui to enter her moves, or in the case of text display types her moves as pairs of integers to stdin after receiving a prompt.
		These pairs are the x y coordinate of the move and should be separated by a space.
		-depth_limit: how deep the algorithms are allowed to recurse from the current state
		-total_timelimit: a time reserve that tracks the total time each player takes for their own moves. If a player's time runs out, they lose
		-turn_timelimit: a time limit for how long a single turn can take. If a player exceedes it, the player loses.
		Time limits are not enforced for human players.

	example: ./tournament gui blackPlayer whitePlayer 4 50000 0

	At the end of the game, the tournament program prints a single line to stdout of the form:

	winner <color> <advantage> <reason>
		-color: either the letter B or W, corresponding to a black or white player win, respectively
		-advantage: an integer equal to (# winner squares - # loser squares). Note that in the case of timeout or illegal (see reason below), advantage might be negative
		-reason is one of the following strings:
			legal 	The game ended normally with no moves left for either player
			timeout The losing program exceeded the time limit on a move
			illegal The losing program made an illegal move
			
### Stand-alone
    usage: game <color> <depthlimit> <total_timelimit> <turn_timelimit>
        -color: a single char (B/W) representing this player's color. 'W' is default

### Testing Scripts
	usage: <Script Name>.sh
		- testing scripts are in each folder, used for timing tests
		- MPI versions have to be killed after each time is displayed (ctrl + c)



## Board

Positions are determined by x, then y. So the upper-right black piece below is at location 4, 3

#### Intitial board position:

	  0 1 2 3 4 5 6 7
	0
	1
	2
	3       w b
	4       b w
	5
	6
	7


### Bitboard diagonals

**Rotated boards:**
This shows a board rotated right 45 degrees. Each pair represents the location of this square in the unrotated board. Notice that each row is a diagonal, with the main diagonal on top.

	  0   1   2   3   4   5   6   7
	0 0,0 1,1 2,2 3,3 4,4 5,5 6,6 7,7
	1 0,1 1,2 2,3 3,4 4,5 5,6 6,7 7,0
	2 0,2 1,3 2,4 3,5 4,6 5,7 6,0 7,1
	3 0,3 1,4 2,5 3,6 4,7 5,0 6,1 7,2
	4 0,4 1,5 2,6 3,7 4,0 5,1 6,2 7,3
	5 0,5 1,6 2,7 3,0 4,1 5,2 6,3 7,4
	6 0,6 1,7 2,0 3,1 4,2 5,3 6,4 7,5
	7 0,7 1,0 2,1 3,2 4,3 5,4 6,5 7,6
	
	
	
## Rotation Modifications

Rotations were taking a substantial amount of time in our algorithm, so we decided to precalculate and store them in a table instead of dynamically calculating them.

### Before Rotation Precalculation

These results were generated using `gprof`
**Stars represent function dependence on rotation**
- 1-star: calls functions that use rotations
- 2-star: directly uses rotations
- 3-star: heavily uses rotations
- 4-star: entirely rotations

<!-- -->
    Each sample counts as 0.01 seconds.
      %   cumulative   self              self     total
     time   seconds   seconds    calls   s/call   s/call  name
     39.27     10.21    10.21 26040415     0.00     0.00  _generate_moves **
     35.88     19.55     9.33 15422026     0.00     0.00  compute_rotations ****
     21.92     25.25     5.70  9565254     0.00     0.00  generate_child ***
      1.40     25.61     0.37 11579102     0.00     0.00  heuristics
      0.69     25.79     0.18   960713     0.00     0.00  generate_children *
      0.46     25.91     0.12        1     0.12    26.01  minimax *
      0.37     26.01     0.10 38261016     0.00     0.00  flip
      0.00     26.01     0.00        1     0.00     0.00  _GLOBAL__sub_I_gameClock


### After Rotation Precalculation

These results were generated using `gprof`
**Stars represent function dependence on rotation**
- 1-star: calls functions that use rotations
- 2-star: directly uses rotations
- 3-star: heavily uses rotations
- 4-star: entirely rotations

<!-- -->
    Each sample counts as 0.01 seconds.
      %   cumulative   self              self     total
     time   seconds   seconds    calls   s/call   s/call  name
     65.56      2.51     2.51 26040415     0.00     0.00  _generate_moves **
      9.80      2.89     0.38  9565254     0.00     0.00  generate_child ***
      8.75      3.22     0.34 11579102     0.00     0.00  heuristics
      6.27      3.46     0.24 15422026     0.00     0.00  compute_rotations ****
      5.22      3.66     0.20   960713     0.00     0.00  generate_children *
      2.22      3.75     0.09 38261016     0.00     0.00  flip
      1.31      3.80     0.05        1     0.05     3.82  minimax *
      0.52      3.82     0.02   543351     0.00     0.00  state::~state()
      0.26      3.83     0.01                             sort_children
      0.00      3.83     0.00        1     0.00     0.00  _GLOBAL__sub_I_gameClock
	  
	  
	  
You can see having rotation precalcuation and lookup instead of dynamic calculation reduced execution time by nearly 7-fold. Notice how much the "self seconds" column is reduced for functions that depend on rotations, while remaining virtually unchanged for functions that involve no rotations at all. Most the algorithm's time is now spent generating moves, instead of computing rotations (see % time).

### Resources

- https://github.com/vit-vit/ctpl
- http://iacoma.cs.uiuc.edu/~greskamp/pdfs/412.pdf
- http://www.iis.sinica.edu.tw/~tshsu/tcg/2013/slides/slide11.pdf
- http://ww2.cs.fsu.edu/~guidry/parallel_chess.pdf
- http://www.netlib.org/utk/lsi/pcwLSI/text/node350.html
- https://books.google.com/books?id=Fkq2BpzRCVgC&pg=PA276&lpg=PA276&dq=young+brother+wait+algorithm&source=bl&ots=1BGGadWRn6&sig=J2RM3_K1SHrVRvvmNmV27sRFTFk&hl=en&sa=X&ved=0ahUKEwje6v3yxtTTAhXD54MKHWk9BqsQ6AEILTAB#v=onepage&q=young%20brother%20wait%20algorithm&f=false

# ParallelOthello
A parallel implementation of Othello

https://github.com/vit-vit/ctpl
http://stackoverflow.com/questions/15752659/thread-pooling-in-c11


## Parallelization

PVC http://iacoma.cs.uiuc.edu/~greskamp/pdfs/412.pdf

Young brother (Could try DTS but finNAH) http://www.iis.sinica.edu.tw/~tshsu/tcg/2013/slides/slide11.pdf

maybe good http://ww2.cs.fsu.edu/~guidry/parallel_chess.pdf

Other
 - http://www.netlib.org/utk/lsi/pcwLSI/text/node350.html
 - https://books.google.com/books?id=Fkq2BpzRCVgC&pg=PA276&lpg=PA276&dq=young+brother+wait+algorithm&source=bl&ots=1BGGadWRn6&sig=J2RM3_K1SHrVRvvmNmV27sRFTFk&hl=en&sa=X&ved=0ahUKEwje6v3yxtTTAhXD54MKHWk9BqsQ6AEILTAB#v=onepage&q=young%20brother%20wait%20algorithm&f=false


## Running

### With Reversi.jar

    Use: tournament interface player1Folder player2Folder maxDepth timeout1 timeout2
        -interface: gui, text, or none


### Stand-alone
    usage: game <color> <depthlimit> <total_timelimit> <turn_timelimit>
        -color: a single char (B/W) representing this player's color. 'W' is default
        -limits: only specify a single limit and enter 0 for the others

### Testing Scripts
	Use: <Script Name>.sh
		- testing scripts are in each folder, used for timing tests
		- MPI versions have to be killed after each time is displayed (ctrl + c)

After this, enter the position you would like to move when prompted as an x y pair, separated by a space. The moves made by the player will be printed in response.



## Board

Positions are determined by x, then y. So the upper-right black piece below is at location 4, 3


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

**R45:**
This shows a board rotated right 45 degrees. Each pair represents the location of this bit in the unrotated board. Notice that each row is a diagonal, with the main diagonal on top.

	  0   1   2   3   4   5   6   7
	0 0,0 1,1 2,2 3,3 4,4 5,5 6,6 7,7
	1 0,1 1,2 2,3 3,4 4,5 5,6 6,7 7,0
	2 0,2 1,3 2,4 3,5 4,6 5,7 6,0 7,1
	3 0,3 1,4 2,5 3,6 4,7 5,0 6,1 7,2
	4 0,4 1,5 2,6 3,7 4,0 5,1 6,2 7,3
	5 0,5 1,6 2,7 3,0 4,1 5,2 6,3 7,4
	6 0,6 1,7 2,0 3,1 4,2 5,3 6,4 7,5
	7 0,7 1,0 2,1 3,2 4,3 5,4 6,5 7,6
	
	
	
# Rotations

Rotations were precalculated and stored in a table instead of dynamically calculating

## Before Rotation Precalculation
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


## After Rotation Precalculation
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

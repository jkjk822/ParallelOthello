# ParallelOthello
A parallel implementation of Othello



## Running

### With Reversi.jar

    Use: tournament interface player1Folder player2Folder maxDepth timeout1 timeout2
        -interface: gui, text, or none


### Stand-alone
    usage: game <color> <depthlimit> <total_timelimit> <turn_timelimit>
        -color: a single char (B/W) representing this player's color. 'W' is default
        -limits: only specify a single limit and enter 0 for the others

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
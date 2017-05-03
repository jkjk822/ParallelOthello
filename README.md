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
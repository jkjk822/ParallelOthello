Taken from https://github.com/scarafoni/Reversi242

Reversi242
==========
Welcome to the readme for Reversi RG (really good) edition. 

manifest:
=========
	README.md: this file.	
	Reversi.jar: the main executable for the game
	SampleClient.java: a sample program that plays the othello game

Instructions for running:
=========================
	to start, type the folowing
	tournament GUI BLACKPLAYER WHITEPLAYER DEPTHLIMIT TIMELIMIT TIMELIMIT2
	
	GUI is one of the following strings:
	gui 	Display the game graphically (using X windows and the DISPLAY environmental variable).
	text 	Print a record of the game to stdout
	none 	Nothing is printed except the final winner of the game

	Each of BLACKPLAYER and WHITEPLAYER is either the flag -human or the name of a player program directory (including the path if not in the working directory). If a player is human the player either uses the gui to enter her moves, or in the case of text display types her moves as pairs of integers to stdin after receiving a prompt. TIMELIMITs are not enforced for human players. For program players, the tournament program runs them and interacts with them via stdin and stdout as described above.

	example: ./tournament gui blackPlayer whitePlayer 4 50000 0
	
	At the end of the game, the tournament program prints a single line to stdout of the form:

	winner COLOR ADVANTAGE REASON

	where COLOR is either the letter B or W, ADVANTAGE is an integer equal to (# winner squares - # loser squares), and REASON is one of the following strings:
	legal 	The game was played properly by both players
	timeout 	The losing program exceeded the time limit on a move
	illegal 	The losing program made an illegal move

Note that in the came of timeout or illegal, ADVANTAGE might be negative.	
		where:

		the sample client file only takes on argument, a string, either "black" or "white" specifying which color the player is. 
		It is very rudimentary, and as such has no support for depth limit, and you'll have to adjust the source code to modify how long it takes to make a move. 
		Black is player 1, white is player 2

What can I do to make my own othello-player?
============================================
	-the game will send a string to your program via stdio which contains the game board. 
	-Your program then prints out a row and column as a single string to stdio (standar print) and the programwill receive it.
	-if you're program wanted to place a piece on spot 2, 3, it would output "2 3"

/*
Reversi

Reversi (Othello) is a game based on a grid with eight rows and eight columns, played between you and the computer, by adding pieces with two sides: black and white.
At the beginning of the game there are 4 pieces in the grid, the player with the black pieces is the first one to place his piece on the board.
Each player must place a piece in a position that there exists at least one straight (horizontal, vertical, or diagonal) line between the new piece and another piece of the same color, with one or more contiguous opposite pieces between them. 

Usage:  java Reversi

10-12-2006 version 0.1: initial release
26-12-2006 version 0.15: added support for applet

Requirement: Java 1.5 or later

future features:
- undo
- save/load board on file, logging of moves
- autoplay
- sound

This software is released under the GNU GENERAL PUBLIC LICENSE, see attached file gpl.txt
*/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*; 
import javax.swing.event.*;
import javax.swing.text.html.*;
import java.io.*;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.FutureTask;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.ExecutionException;

class GPanel extends JPanel implements MouseListener {


	public boolean FLAG = false;
	public int time1;
	public int time2;
	public int depth;
	public int blackTimeLeft;
	public int whiteTimeLeft;
	public String DISPLAY;
	public boolean whH = false;
	public boolean blH = false;
	public String BLACK;
	public String WHITE;
	public boolean iswhite = true;
	public String gameOverString = "GAME_OVER";
	public String currentMove;
	public boolean flag = false; //It will be set if both black and white could not make a legal move, even though the board is not full.
	public Process blackPlayer, whitePlayer;

	BufferedWriter blackIn;
	BufferedReader blackOut;
	BufferedWriter whiteIn;
	BufferedReader whiteOut;

	public ReversiBoard board;
	int gameLevel;
	ImageIcon button_black, button_white;
	JLabel score_black, score_white;
	String gameTheme;
	Move hint=null;
	boolean inputEnabled, active;
	public GPanel (ReversiBoard board, JLabel score_black, JLabel score_white, String theme, int level, String disp, String blackProgram,String whiteProgram, int t1, int t2, int de) {
		super();
		this.board = board;
		this.score_black = score_black;
		this.score_white = score_white;

		Runtime runtime = Runtime.getRuntime();
		/*
		try{
			blackPlayer = runtime.exec(blackProgram);
			whitePlayer = runtime.exec(whiteProgram);
			this.blackOut = new BufferedReader(new InputStreamReader(blackPlayer.getInputStream()));
			this.blackIn = new BufferedWriter(new OutputStreamWriter(blackPlayer.getOutputStream()));
			this.whiteOut = new BufferedReader(new InputStreamReader(whitePlayer.getInputStream()));
			this.whiteIn = new BufferedWriter(new OutputStreamWriter(whitePlayer.getOutputStream()));

		}catch(IOException e){System.exit(1);}
		//System.out.println("done initing readers");
		*/
		time1 = t1;
		depth = de;
		DISPLAY = disp;
		blackTimeLeft =t2*1000;
		whiteTimeLeft =t2*1000;
		time2 = t2;
		if((blackProgram.equals("-human") || whiteProgram.equals("-human")) && !disp.equals("gui")) {
			System.out.println("Need gui for human players");
			System.exit(0);
		}

		if (blackProgram.equals("-human")) {
			blH = true;
		}
		else {
			try {
				blackPlayer = runtime.exec(blackProgram+"/play");
				this.blackOut = new BufferedReader(new InputStreamReader(blackPlayer.getInputStream()));
				this.blackIn = new BufferedWriter(new OutputStreamWriter(blackPlayer.getOutputStream()));
				//blackIn.write("game B "+depth+" "+time1+" "+time2 +"\n");
				blackIn.write("game B "+depth+" "+time1+" "+time2 +"\n");
				blackIn.flush();
			}catch(Exception e){
					if(!DISPLAY.equals("none"))
						System.out.println("black's io isn't working");}
		}
		if (whiteProgram.equals("-human")) {
			whH = true;
		}
		else {
			try {
				whitePlayer = runtime.exec(whiteProgram+"/play");
				this.whiteOut = new BufferedReader(new InputStreamReader(whitePlayer.getInputStream()));
				this.whiteIn = new BufferedWriter(new OutputStreamWriter(whitePlayer.getOutputStream()));
				//whiteIn.write("game W "+depth+" "+time1+" "+time2+"\n");
				whiteIn.write("game W "+depth+" "+time1+" "+time2+"\n");
				whiteIn.flush();
			}catch(Exception e){
				if(!DISPLAY.equals("none"))
					System.out.println("white's io isn't working");}
		}
		
		gameLevel = level;
		setTheme(theme);
		
		addMouseListener(this);
	}

	public void setTheme(String gameTheme)  {
		//System.out.println("GPanel: setTheme");
		hint = null;
		this.gameTheme = gameTheme;
		if (gameTheme.equals("Classic")) {
			button_black = new ImageIcon(Reversi.class.getResource("button_black.jpg"));
			button_white = new ImageIcon(Reversi.class.getResource("button_white.jpg"));
			setBackground(Color.green);
		}	else if (gameTheme.equals("Electric")) {
				button_black = new ImageIcon(Reversi.class.getResource("button_blu.jpg"));
				button_white = new ImageIcon(Reversi.class.getResource("button_red.jpg"));
				setBackground(Color.white);
			}	else {
						gameTheme = "Flat"; // default theme "Flat"
						setBackground(Color.green); 
				}
		repaint();
	}

	public void setLevel(int level) {
		//System.out.println("GPanel: setLevel");
		if ((level > 1) && (level < 7)) gameLevel = level;
	}

	public void drawPanel(Graphics g) {
	//	System.out.println("GPanel: drawPanel");
//	    int currentWidth = getWidth();
//		int currentHeight = getHeight();
		for (int i = 1 ; i < 8 ; i++) {
			g.drawLine(i * Reversi.Square_L, 0, i * Reversi.Square_L, Reversi.Height);
		}
		g.drawLine(Reversi.Width, 0, Reversi.Width, Reversi.Height);
		for (int i = 1 ; i < 8 ; i++) {
			g.drawLine(0, i * Reversi.Square_L, Reversi.Width, i * Reversi.Square_L);
		}
		g.drawLine(0, Reversi.Height, Reversi.Width, Reversi.Height);
		 for (int i = 0 ; i < 8 ; i++)
			for (int j = 0 ; j < 8 ; j++)
				switch (board.get(i,j)) {
					case white:  
						if (gameTheme.equals("Flat"))
						{	g.setColor(Color.white);
							g.fillOval(1 + i * Reversi.Square_L, 1 + j * Reversi.Square_L, Reversi.Square_L-1, Reversi.Square_L-1);
						}
						else g.drawImage(button_white.getImage(), 1 + i * Reversi.Square_L, 1 + j * Reversi.Square_L, this); 
						break;
					case black:  
						if (gameTheme.equals("Flat"))
						{	g.setColor(Color.black);
							g.fillOval(1 + i * Reversi.Square_L, 1 + j * Reversi.Square_L, Reversi.Square_L-1, Reversi.Square_L-1);
						}
						else g.drawImage(button_black.getImage(), 1 + i * Reversi.Square_L, 1 + j * Reversi.Square_L, this); 
						break;
				}
		if (hint != null) {
			g.setColor(Color.darkGray);
			g.drawOval(hint.i * Reversi.Square_L+3, hint.j * Reversi.Square_L+3, Reversi.Square_L-6, Reversi.Square_L-6);
		}
	}	
	
	public void paintComponent(Graphics g) {
		//System.out.println("GPanel: paintComponent");
		super.paintComponent(g);
		drawPanel(g);

	}

	public Dimension getPreferredSize() {
		//System.out.println("Reversi: getPreferredSize");
		return new Dimension(Reversi.Width,Reversi.Height);
	}

	public void showWinner() {
		//System.out.println("Reversi: showWinner");
		if(DISPLAY.equals("text"))
			System.out.println(board.textBoard());
		inputEnabled = false;
		active = false;
		if (board.counter[0] > board.counter[1]) {
			//JOptionPane.showMessageDialog(this, "You win!","Reversi",JOptionPane.INFORMATION_MESSAGE);
			System.out.println("winner B " +  (board.counter[0] - board.counter[1]) + " legal");
		}
		else if (board.counter[0] < board.counter[1]) {
		   	//JOptionPane.showMessageDialog(this, "I win!","Reversi",JOptionPane.INFORMATION_MESSAGE);
			System.out.println("winner W " +  (board.counter[1] - board.counter[0]) + " legal");
		}
		else {
			System.out.println("winner T 0 legal");
			//JOptionPane.showMessageDialog(this, "Drawn!","Reversi",JOptionPane.INFORMATION_MESSAGE);
		}
		if(blackPlayer != null)
			blackPlayer.destroy();
		if(whitePlayer != null)
		whitePlayer.destroy();
		System.exit(0);	
	}

	public void setHint(Move hint) {
		this.hint = hint;
	}

	public void clear() {
		//System.out.println("GPanel: clear");
		board.clear();
		score_black.setText(Integer.toString(board.getCounter(TKind.black)));
		score_white.setText(Integer.toString(board.getCounter(TKind.white)));
		//inputEnabled = true;
		//active = true;
	}


	public void computerMove() {
		paintImmediately(0, 0, Reversi.Width, Reversi.Height);
		repaint();
		Integer i = 0;
		//System.out.println("in computerMove");
		boolean validInput;
		Integer j = 0;
		try {
				String input = null;
				BufferedWriter currentIn;
				BufferedReader currentOut;
				BufferedWriter otherIn;
				BufferedReader otherOut;
				String currentPlayer;
				if(iswhite) {
					currentPlayer = "white";
					currentIn = whiteIn;
					currentOut = whiteOut;
					otherIn = blackIn;
					otherOut = blackOut;
				}
				else {
					currentPlayer = "black";
					currentIn = blackIn;
					currentOut = blackOut;
					otherIn = whiteIn;
					otherOut = whiteOut;
				}


				//if(!board.userCanMove(TKind.black)&& !board.userCanMove(TKind.white))
					//showWinner();	
				try {
					if(!DISPLAY.equals("none"))
						System.out.println("about to read input");
			if ( (iswhite && !whH) || (!iswhite && !blH)) {
                		input = currentOut.readLine();
			}
			else {
				input = this.currentMove;
			}
					
				}catch(Exception io) {
					if(!DISPLAY.equals("none"))
						System.out.println("couldn't read input");
					//System.exit(1);
					illegalMove();
				}
					if(!DISPLAY.equals("none"))
						System.out.println("read input: "+input);
				if(input == null) {illegalMove();}
					
				//System.out.println(board.textBoard());

				//process numbers
				if(!input.equals("pass")) {
				String[] inputSplit = input.split(" ");
				//System.out.println(inputSplit[0]+" "+inputSplit[1]);
					try {
						i = Integer.parseInt(inputSplit[0]);
						j = Integer.parseInt(inputSplit[1]);

						if ((!iswhite && (i < 8) && (j < 8) && (board.get(i,j) == TKind.nil) && (board.move(new Move(i,j),TKind.black) != 0)) || 
							(iswhite && (i < 8) && (j < 8) && (board.get(i,j) == TKind.nil) && (board.move(new Move(i,j),TKind.white) != 0))) 
							{
					if(!DISPLAY.equals("none"))
                        		  System.out.println(input);
							if(otherIn != null) {
								try {
								otherIn.write(input + "\n");
								otherIn.flush();
								}
								catch(IOException ioe) {
									if(!DISPLAY.equals("none"))
										ioe.printStackTrace();
									iswhite = !iswhite;
									illegalMove();

								}
								}
									
								validInput = true;
								//flag = false;
							}
						else 
							illegalMove();
					}catch(NumberFormatException e) {
						if(!DISPLAY.equals("none"))
							System.out.println("poorly formatted input");
						illegalMove();
						//validInput = false;
					}
			}
			else
			{
				//if(!board.userCanMove(TKind.black)&& !board.userCanMove(TKind.white))
				if(iswhite && board.userCanMove(TKind.white)) {
					if(!DISPLAY.equals("none"))
							System.out.println("White has moves!");
					illegalMove();
				}
				if(!iswhite && board.userCanMove(TKind.black)) {
					if(!DISPLAY.equals("none"))
							System.out.println("Black has moves!");
					illegalMove();
				}
				//if((iswhite && !blH) || (!iswhite && !whH)){
				if(otherIn != null) {
					if(!DISPLAY.equals("none"))
						System.out.println(input);
					try {
						otherIn.write(input + "\n");
						otherIn.flush();
						}
						catch(IOException ioe) {
							if(!DISPLAY.equals("none"))
								ioe.printStackTrace();
							iswhite = !iswhite;
							illegalMove();

						}
					validInput = true;
				}
				//if(flag)
					//showWinner();
				//flag = true;
			}
		}catch(Exception e) {
			if(!DISPLAY.equals("none"))
				e.printStackTrace();
			if(blackPlayer != null) {
				blackPlayer.destroy();
			}
			if(whitePlayer != null) {
				whitePlayer.destroy();
			}
			illegalMove();
			System.exit(1); 
		}
		score_black.setText(Integer.toString(board.getCounter(TKind.black)));
		score_white.setText(Integer.toString(board.getCounter(TKind.white)));
		//repaint();

		if (board.gameEnd()) 
			showWinner();

		flag = false;
		if (iswhite && !board.userCanMove(TKind.black)) {
			if(!DISPLAY.equals("none"))			
				System.out.println("black cant move");
			//showWinner();
			if(blH)
				flag = true;
		}
		if (!iswhite && !board.userCanMove(TKind.white)) {
			if(!DISPLAY.equals("none"))
				System.out.println("white cant move");
			if(whH)
				flag = true;
			//showWinner();
		}
		//run();

		/*if (board.gameEnd()) 
			showWinner();
		else if (!board.userCanMove(TKind.black))
				System.out.println("You Pass");
		else if (board.userCanMove(TKind.black)) 
			System.out.println("I Pass");
		else showWinner();*/
	}

	public void mouseClicked(MouseEvent e) {
		//System.out.println("GPanel: mouseClicked");
		// generato quando il mouse viene premuto e subito rilasciato (click)
		if (inputEnabled) {
			hint = null;
			int i = e.getX() / Reversi.Square_L;
			int j = e.getY() / Reversi.Square_L;
			if ((!iswhite && (i < 8) && (j < 8) && (board.get(i,j) == TKind.nil) && (board.move(new Move(i,j),TKind.black) != 0)) || (iswhite && (i < 8) && (j < 8) && (board.get(i,j) == TKind.nil) && (board.move(new Move(i,j),TKind.white) != 0))) {
				score_black.setText(Integer.toString(board.getCounter(TKind.black)));
				score_white.setText(Integer.toString(board.getCounter(TKind.white)));
				//repaint();
				moved = true;
				inputEnabled = false;						
				javax.swing.SwingUtilities.invokeLater(new Runnable() {
					public void run() {	
						
						Cursor savedCursor = getCursor();
						setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));				
						
						setCursor(savedCursor);			
					}
				});
				this.currentMove = i + " " + j;
				try {
			    if (iswhite && !blH) {
				  blackIn.write(this.currentMove + "\n");
				  blackIn.flush();
				}
				if (!iswhite && !whH) {
				  whiteIn.write(this.currentMove + "\n");
				  whiteIn.flush();
				}
                }
				catch (IOException ex) {
				  System.out.println("human side output to pipe error\n");
				}
				inputEnabled = false;

				if (board.gameEnd()) {
					showWinner();			
				}
				if(blH && whH)
					flag = false;
				if (iswhite && !board.userCanMove(TKind.black)) {
					System.out.println("black cant move");
					if(blH)
						flag = true;
					//showWinner();
				}
				if (!iswhite && !board.userCanMove(TKind.white)) {
					System.out.println("white cant move");
					if(whH)
						flag = true;
					//showWinner();
				}
				run();
			}
			else JOptionPane.showMessageDialog(this, "Illegal move","Reversi",JOptionPane.ERROR_MESSAGE);
			
		}
	}

	public void mouseEntered(MouseEvent e) {
// generato quando il mouse entra nella finestra
	}


	public void mouseExited(MouseEvent e) {
// generato quando il mouse esce dalla finestra
	}


	public void mousePressed(MouseEvent e) {
// generato nell'istante in cui il mouse viene premuto
	}


	public void mouseReleased(MouseEvent e) {
// generato quando il mouse viene rilasciato, anche a seguito di click
	}


	public boolean moved = false;



	public void timeup() {
		if(iswhite) {
			System.out.println("winner B " + (board.counter[0] - board.counter[1]) + " timeout");	
		}
		else {
			System.out.println("winner W " +  (board.counter[1] - board.counter[0]) + " timeout");	
		}
		if(!DISPLAY.equals("none"))
			System.out.println(gameOverString);
		//whiteSocket.sendBoard(gameOverString);
		//blackSocket.sendBoard(gameOverString);
		if(blackPlayer != null)
			blackPlayer.destroy();
		if(whitePlayer != null)
			whitePlayer.destroy();
		DISPLAY = "none";
		System.exit(0);
		
	}
	public void illegalMove() {
		if(!FLAG) {
			if(iswhite) {
				System.out.println("winner B " +  (board.counter[0] - board.counter[1]) + " Illegal");
			}
			else {
				System.out.println("winner W " + (board.counter[1] - board.counter[0]) + " Illegal");
			}
			if(!DISPLAY.equals("none"))
				System.out.println(gameOverString);
			//whiteSocket.sendBoard(gameOverString);
			//blackSocket.sendBoard(gameOverString);
			if(blackPlayer != null)
				blackPlayer.destroy();
			if(whitePlayer != null)
			whitePlayer.destroy();
			
		}
		System.exit(0);
	}


	public void run() {
		inputEnabled = false;
		repaint();

		//System.out.println("GPanel: run");

		//while(!board.gameEnd()) {
		if(!board.userCanMove(TKind.black)&& !board.userCanMove(TKind.white))
			showWinner();
		if((whH || blH) && flag)
		{
			try {
				if (whH && !blH) {
					blackIn.write("pass\n");
					blackIn.flush();
				}
				if (blH && !whH) {
					whiteIn.write("pass\n");
					whiteIn.flush();
				}
			}
			catch (IOException ex) {
				System.out.println("human side output to pipe error\n");
			}
		}
		else
			iswhite = !iswhite;
		int currentTimeout = time1;
		
		if(DISPLAY.equals("text")) {
			System.out.println(board.textBoard());
		}

		if(iswhite) {
			if(!DISPLAY.equals("none"))
			System.out.println("IT IS WHITE'S TURN");
			if(whH) {
				inputEnabled = true;

			}
			else {
				if(time2!=0) {
					final Runnable mstuffToDo = new Thread() {
						@Override public void run() {
							computerMove();
						}
					};
					long start_w_time = System.currentTimeMillis();
					final ExecutorService mexecutor = Executors.newSingleThreadExecutor();
					final Future mfuture = mexecutor.submit(mstuffToDo);
					mexecutor.shutdown();
					try { mfuture.get(whiteTimeLeft, TimeUnit.MILLISECONDS); }
					catch (InterruptedException ie) {
						System.out.println(ie.getMessage()); 
						if(blackPlayer != null) 
							blackPlayer.destroy();
						if(whitePlayer != null) 
							whitePlayer.destroy(); 
						System.exit(0);
					}
					catch (TimeoutException te) {mfuture.cancel(true); FLAG = true; timeup();}
					catch (ExecutionException ee) { 
						System.out.println(ee.getMessage()); 
						if(blackPlayer != null)
							blackPlayer.destroy();
						if(whitePlayer != null)
							whitePlayer.destroy(); 
						System.exit(0);
					}
					if (!mexecutor.isTerminated())
						mexecutor.shutdownNow();
					mfuture.cancel(true);

					whiteTimeLeft = whiteTimeLeft - (int)(System.currentTimeMillis() - start_w_time);
				}
				else {
					if(time1!=0) {
						final Runnable stuffToDo = new Thread() {
							@Override public void run() {computerMove();}
						};
						final ExecutorService executor = Executors.newSingleThreadExecutor();
						final Future future = executor.submit(stuffToDo);
						executor.shutdown();
						try { future.get(time1, TimeUnit.SECONDS); }
						catch (InterruptedException ie) {
							System.out.println(ie.getMessage()); 
							if(blackPlayer != null) 
								blackPlayer.destroy();
							if(whitePlayer != null) 
								whitePlayer.destroy(); 
							System.exit(0);
						}
						catch (TimeoutException te) {future.cancel(true); FLAG = true; timeup();}
						catch (ExecutionException ee) { 
							System.out.println(ee.getMessage()); 
							if(blackPlayer != null)
								blackPlayer.destroy();
							if(whitePlayer != null)
								whitePlayer.destroy(); 
							System.exit(0);
						}
						if (!executor.isTerminated())
							executor.shutdownNow();
						future.cancel(true);
					}
					else {computerMove();}
				}
				run();
			}
		}
		else {
			if(!DISPLAY.equals("none"))
				System.out.println("IT IS BLACK'S TURN");
			if(blH) {
				inputEnabled = true;
			}
			else {
				if(time2!=0) {
					final Runnable mstuffToDo = new Thread() {
						@Override public void run() {
							computerMove();
						};
					};
					long start_b_time = System.currentTimeMillis();

					final ExecutorService mexecutor = Executors.newSingleThreadExecutor();
					final Future mfuture = mexecutor.submit(mstuffToDo);
					mexecutor.shutdown();
					try { mfuture.get(whiteTimeLeft, TimeUnit.MILLISECONDS); }
					catch (InterruptedException ie) {
						System.out.println(ie.getMessage()); 
						if(blackPlayer != null) 
							blackPlayer.destroy();
						if(whitePlayer != null) 
							whitePlayer.destroy(); 
						System.exit(0);
					}
					catch (TimeoutException te) {mfuture.cancel(true); FLAG = true; timeup();}
					catch (ExecutionException ee) { 
						System.out.println(ee.getMessage()); 
						if(blackPlayer != null)
							blackPlayer.destroy();
						if(whitePlayer != null)
							whitePlayer.destroy(); 
						System.exit(0);
					}
					if (!mexecutor.isTerminated())
						mexecutor.shutdownNow();
					mfuture.cancel(true);

					blackTimeLeft = blackTimeLeft - (int)(System.currentTimeMillis() - start_b_time);
				}
				else {
					if(time1!=0) {
						final Runnable stuffToDo = new Thread() {
							@Override public void run(){ computerMove();}
						};
						final ExecutorService executor = Executors.newSingleThreadExecutor();
						final Future future = executor.submit(stuffToDo);
						executor.shutdown();
						try { future.get(time1, TimeUnit.SECONDS); }
						catch (InterruptedException ie) {
							System.out.println(ie.getMessage()); 
							if(blackPlayer != null) 
								blackPlayer.destroy();
							if(whitePlayer != null) 
								whitePlayer.destroy(); 
							System.exit(0); 
						}
						catch (TimeoutException te) {future.cancel(true); FLAG = true;  timeup();}
						catch (ExecutionException ee) {
							System.out.println(ee.getMessage()); 
							if(blackPlayer != null) 
								blackPlayer.destroy();
							if(whitePlayer != null) 
								whitePlayer.destroy(); 
							System.exit(0); 
						}
						if (!executor.isTerminated())
							executor.shutdownNow();
						future.cancel(true);
					}
					else {computerMove();}
				}
				run();
			}
		}



		//}
		//showWinner();

	}

};

public class Reversi extends JFrame implements ActionListener{

	JEditorPane editorPane;

	static final String WindowTitle = "Reversi";
	static final String ABOUTMSG = WindowTitle+"\n\n26-12-2006\njavalc6";
	static GPanel gpanel;
	static JMenuItem hint;
	static boolean helpActive = false;

	static final int Square_L = 33; // length in pixel of a square in the grid
	static final int  Width = 8 * Square_L; // Width of the game board
	static final int  Height = 8 * Square_L; // Width of the game board

	ReversiBoard board;
	static JLabel score_black, score_white;
	JMenu level, theme;

	public Reversi(String disp,String blackProgram, String whiteProgram, int depth, int time1, int time2) {
		super(WindowTitle);
		
		//System.out.println("Reversi");
		score_black=new JLabel("2"); // the game start with 2 black pieces
		score_black.setForeground(Color.blue);
		score_black.setFont(new Font("Dialog", Font.BOLD, 16));
		score_white=new JLabel("2"); // the game start with 2 white pieces
		score_white.setForeground(Color.red);
		score_white.setFont(new Font("Dialog", Font.BOLD, 16));
		board = new ReversiBoard();
		gpanel = new GPanel(board, score_black, score_white,"Classic", 3, disp,blackProgram,whiteProgram, time1, time2, depth);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setupMenuBar();
		gpanel.setMinimumSize(new Dimension(Reversi.Width,Reversi.Height));

		JPanel status = new JPanel();
		status.setLayout(new BorderLayout());
		status.add(score_black, BorderLayout.WEST);
		status.add(score_white, BorderLayout.EAST);
//		status.setMinimumSize(new Dimension(100, 30));
        JSplitPane splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT, gpanel, status);
		splitPane.setOneTouchExpandable(false);
		getContentPane().add(splitPane);

		pack();
		if(disp.equals("none"))
			setVisible(false);
		else if(disp.equals("text"))
			setVisible(false);
		else
			setVisible(true);

		setResizable(false);

		//gpanel.blackSocket.sendBoard(gpanel.board.printBoard());
		gpanel.run();
	}



// voci del menu di primo livello
// File Edit Help
//
	void setupMenuBar() {
		//System.out.println("Reversi: setupMenuBar");
		JMenuBar menuBar = new JMenuBar();
		menuBar.add(buildGameMenu());
		menuBar.add(buildHelpMenu());
		setJMenuBar(menuBar);	
	}


    public void actionPerformed(ActionEvent e) {
		//System.out.println("Reversi: actionPerformed");
        JMenuItem source = (JMenuItem)(e.getSource());
		String action = source.getText();
		if (action.equals("2")) gpanel.setLevel(2);
		else if (action.equals("3")) gpanel.setLevel(3);
			else if (action.equals("4")) gpanel.setLevel(4);
				else if (action.equals("5")) gpanel.setLevel(5);
					else if (action.equals("6")) gpanel.setLevel(6);
						else if (action.equals("Classic")) gpanel.setTheme(action);
							else if (action.equals("Electric")) gpanel.setTheme(action);
								else if (action.equals("Flat")) gpanel.setTheme(action);
	}

    protected JMenu buildGameMenu() {
		//System.out.println("Reversi: buildGameMenu");
		JMenu game = new JMenu("Game");
		JMenuItem newWin = new JMenuItem("New");
//		JMenuItem level = new JMenuItem("Level");
		level = new JMenu("Level");
		theme = new JMenu("Theme");
		JMenuItem undo = new JMenuItem("Undo");
		hint = new JMenuItem("Hint");
		undo.setEnabled(false);
		JMenuItem quit = new JMenuItem("Quit");

// build level sub-menu
		ButtonGroup group = new ButtonGroup();
		JRadioButtonMenuItem rbMenuItem = new JRadioButtonMenuItem("2");
		group.add(rbMenuItem);
		level.add(rbMenuItem);
		rbMenuItem.addActionListener(this);
		rbMenuItem = new JRadioButtonMenuItem("3", true);
		group.add(rbMenuItem);
		level.add(rbMenuItem);
		rbMenuItem.addActionListener(this);
		rbMenuItem = new JRadioButtonMenuItem("4");
		group.add(rbMenuItem);
		level.add(rbMenuItem);
		rbMenuItem.addActionListener(this);
		rbMenuItem = new JRadioButtonMenuItem("5");
		group.add(rbMenuItem);
		level.add(rbMenuItem);
		rbMenuItem.addActionListener(this);
		rbMenuItem = new JRadioButtonMenuItem("6");
		group.add(rbMenuItem);
		level.add(rbMenuItem);
		rbMenuItem.addActionListener(this);

// build theme sub-menu
		group = new ButtonGroup();
		rbMenuItem = new JRadioButtonMenuItem("Classic");
		group.add(rbMenuItem);
		theme.add(rbMenuItem);
		rbMenuItem.addActionListener(this);
		rbMenuItem = new JRadioButtonMenuItem("Electric", true);
		group.add(rbMenuItem);
		theme.add(rbMenuItem);
		rbMenuItem.addActionListener(this);
		rbMenuItem = new JRadioButtonMenuItem("Flat");
		group.add(rbMenuItem);
		theme.add(rbMenuItem);
		rbMenuItem.addActionListener(this);

// Begin "New"
		newWin.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				gpanel.clear();
				hint.setEnabled(true);
				repaint();
			}});
// End "New"

// Begin "Quit"
		quit.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
					System.exit(0);
			}});
// End "Quit"


// Begin "Hint"
		hint.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (gpanel.active)	{
					Move move = new Move();
					if (board.findMove(TKind.black,gpanel.gameLevel,move))
						gpanel.setHint(move);
						repaint();
	/*					if (board.move(move,TKind.black) != 0) {
							score_black.setText(Integer.toString(board.getCounter(TKind.black)));
							score_white.setText(Integer.toString(board.getCounter(TKind.white)));
							repaint();
							javax.swing.SwingUtilities.invokeLater(new Runnable() {
								public void run() {
									Cursor savedCursor = getCursor();
									setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
									gpanel.computerMove();
									setCursor(savedCursor);		
								}
							});
						}	
	*/
				} else hint.setEnabled(false);
			}});
// End "Hint"


		game.add(newWin);
		game.addSeparator();
		game.add(undo);
		game.add(hint);
		game.addSeparator();
		game.add(level);
		game.add(theme);
		game.addSeparator();
		game.add(quit);
		return game;
    }


	protected JMenu buildHelpMenu() {
		//System.out.println("Reversi: buildHelpMenu");
		JMenu help = new JMenu("Help");
		JMenuItem about = new JMenuItem("About "+WindowTitle+"...");
		JMenuItem openHelp = new JMenuItem("Help Topics...");

// Begin "Help"
		openHelp.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
		        createEditorPane();
			}});
// End "Help"

// Begin "About"
		about.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				ImageIcon icon = new ImageIcon(Reversi.class.getResource("reversi.jpg"));
				JOptionPane.showMessageDialog(null, ABOUTMSG, "About "+WindowTitle,JOptionPane.PLAIN_MESSAGE, icon);
			}});
// End "About"

		help.add(openHelp);
		help.add(about);

		return help;
    }

    protected void createEditorPane() {
	//System.out.println("Reversi: createEditorPane");
        if (helpActive) return;
		editorPane = new JEditorPane(); 
        editorPane.setEditable(false);
		editorPane.addHyperlinkListener(new HyperlinkListener() {
				public void hyperlinkUpdate(HyperlinkEvent e) {
				if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
					if (e instanceof HTMLFrameHyperlinkEvent) {
					((HTMLDocument)editorPane.getDocument()).processHTMLFrameHyperlinkEvent(
						(HTMLFrameHyperlinkEvent)e);
					} else {
					try {
						editorPane.setPage(e.getURL());
					} catch (java.io.IOException ioe) {
						System.out.println("IOE: " + ioe);
					}
					}
				}
				}
			});
        java.net.URL helpURL = Reversi.class.getResource("HelpFile.html");
        if (helpURL != null) {
            try {
                editorPane.setPage(helpURL);
				new HelpWindow(editorPane);
            } catch (java.io.IOException e) {
                System.err.println("Attempted to read a bad URL: " + helpURL);
            }
        } else {
            System.err.println("Couldn't find file: HelpFile.html");
        }

        return;
    }



	public class HelpWindow extends JFrame{

		public HelpWindow(JEditorPane editorPane) {
		super("Help Window");
		addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				Reversi.helpActive = false;
				setVisible(false);
			}
		});

		JScrollPane editorScrollPane = new JScrollPane(editorPane);
		editorScrollPane.setVerticalScrollBarPolicy(
							JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
		getContentPane().add(editorScrollPane);
		setSize(600,400);
		setVisible(true);
		helpActive = true;
		}
	}

    public HyperlinkListener createHyperLinkListener1() {
	return new HyperlinkListener() {
	    public void hyperlinkUpdate(HyperlinkEvent e) {
		if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
		    if (e instanceof HTMLFrameHyperlinkEvent) {
			((HTMLDocument)editorPane.getDocument()).processHTMLFrameHyperlinkEvent(
			    (HTMLFrameHyperlinkEvent)e);
		    } else {
			try {
			    editorPane.setPage(e.getURL());
			} catch (java.io.IOException ioe) {
			    System.out.println("IOE: " + ioe);
			}
		    }
		}
	    }
	};
    }




	public static void main(String[] args) {
		try {
		UIManager.setLookAndFeel(
			"com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
		} catch (Exception e) { }

		
		if (args.length != 6) {
			System.out.println("Wrong number of arguments");
			System.exit(0);
		}
		else {
			Reversi app = new Reversi(args[0],args[1],args[2],Integer.parseInt(args[3]), Integer.parseInt(args[4]), Integer.parseInt(args[5]));
		}

		
	}

}
echo ======================
#beginning game
./play -v <<< "game b 10 0 0"
echo "Depth: 10, START"
#middle game
./play -v -w 0x0002040C1408100 -b 0x0000007008040200 <<< "game b 10 0 0"
echo "Depth: 10, MIDDLE"
#Close to end game
./play -v -w 0x00101717076F0000 -b 0x0C0C084838103C3C <<< "game b 8 0 0"
echo "Depth: 8, END"

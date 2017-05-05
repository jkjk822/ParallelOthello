echo ======================
#beginning game
mpirun -np 5 ./linux/sor <<< "game b 10 0 0"
echo "Depth: 10, START"
#middle game
#mpirun -np 4 -machinefile hosts ./linux/sor -w 0x0002040C1408100 -b 0x0000007008040200 <<< "game b 10 0 0"
#echo "Depth: 10, MIDDLE"
#Close to end game
#mpirun -np 4 -machinefile hosts ./linux/sor -w 0x00101717076F0000 -b 0x0C0C084838103C3C <<< "game b 10 0 0"
#echo "Depth: 10, END"
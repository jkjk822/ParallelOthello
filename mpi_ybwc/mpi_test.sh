N=11
echo ======================
for i in 2 4 8 12 16 24 32 48
do
	for j in 1 2 3
	do
		mpirun -np $i -machinefile hosts ./linux/sor -v <<< "game b $N 0 0"
		last_pid=$!
		sleep 10
		kill -9 $last_pid
		echo "Depth: $N, START"
	done
	for j in 1 2 3
	do
		mpirun -np $i -machinefile hosts ./linux/sor -v -w 0x0002040C1408100 -b 0x0000007008040200 <<< "game b $N 0 0"
		last_pid=$!
		sleep 10
		kill -9 $last_pid
		echo "Depth: $N, MIDDLE"
	done
	for j in 1 2 3
	do
		mpirun -np $i -machinefile hosts ./linux/sor -v -w 0x00101717076F0000 -b 0x0C0C084838103C3C <<< "game b $N 0 0"
		last_pid=$!
		sleep 10
		kill -9 $last_pid
		echo "Depth: $(($N-1)), END"
	done
done
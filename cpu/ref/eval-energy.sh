for t in {3..7}
do
	e1=$(( -t ))
	e2=$(( 1+t ))
	echo $e1 $e2
	time ./sim --particles 50 --frames 1 --dt 1e$e1 --timesteps 1e$e2
	cp energy.csv energy/dt$e1
done

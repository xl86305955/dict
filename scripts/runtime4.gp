reset
set xlabel 'prefix'
set ylabel 'time(msec)'
set title 'perfomance comparison'
set term png enhanced font 'Verdana,10'
set output 'runtime4.png'
set format x "%10.0f"
set xtic 1200
set xtics rotate by 45 right

plot [:12500][:]'bench_ref.txt' using 1:2 with points  lc 2 title 'ref',\


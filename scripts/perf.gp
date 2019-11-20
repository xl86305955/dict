reset
set style fill solid
set key center top
set title 'perf stat'
set term png enhanced font 'Verdana,10'
set output 'perf_stat.png'

plot 'perf_cpy.txt' using 1:xtic(1) with histogram title 'cpy' , \
	'perf_ref.txt' using 1:xtic(1) with histogram title 'ref' 

# plot '' using 1:xtic(2) with histogram

# plot '' using 1:xtic(3) with histogram 

# plot '' using 1:xtic(4) with histogram 


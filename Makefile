TESTS = test_cpy test_ref

BENCH_FILE = bench_cpy.txt bench_ref.txt

TEST_DATA = s Tai

CFLAGS = -O0 -Wall -Werror -g

# Control the build verbosity
ifeq ("$(VERBOSE)","1")
    Q :=
    VECHO = @true
else
    Q := @
    VECHO = @printf
endif

GIT_HOOKS := .git/hooks/applied

.PHONY: all clean

all: $(GIT_HOOKS) $(TESTS)

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

OBJS_LIB = \
    tst.o bloom.o

OBJS := \
    $(OBJS_LIB) \
    test_cpy.o \
    test_ref.o

deps := $(OBJS:%.o=.%.o.d)

test_%: test_%.o $(OBJS_LIB)
	$(VECHO) "  LD\t$@\n"
	$(Q)$(CC) $(LDFLAGS)  -o $@ $^ -lm

%.o: %.c
	$(VECHO) "  CC\t$@\n"
	$(Q)$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<

test:  $(TESTS)
	rm -f cpy.txt ref.txt
	echo 3 | sudo tee /proc/sys/vm/drop_caches;
	perf stat --repeat 100 \
                -e cache-misses,cache-references,instructions,cycles \
                ./test_cpy --bench $(TEST_DATA)
	perf stat --repeat 100 \
                -e cache-misses,cache-references,instructions,cycles \
				./test_ref --bench $(TEST_DATA)

bench_file: $(TESTS)
	./test_cpy --bench > bench_cpy.txt
	./test_ref --bench > bench_ref.txt

bench: $(TESTS)
	@for test in $(TESTS); do \
	    echo -n "$$test => "; \
	    ./$$test --bench $(TEST_DATA); \
	done

plot: $(TESTS)
	echo 3 | sudo tee /proc/sys/vm/drop_caches;
	perf stat --repeat 100 \
                -e cache-misses,cache-references,instructions,cycles \
                ./test_cpy --bench $(TEST_DATA) \
		| grep 'ternary_tree, loaded 93827 words'\
		| grep -Eo '[0-9]+\.[0-9]+' > cpy_data.csv
	perf stat --repeat 100 \
                -e cache-misses,cache-references,instructions,cycles \
	perf record -o ref_perf.data -e cpu-cycles ./test_ref --bench					./test_ref --bench $(TEST_DATA)\
		| grep 'ternary_tree, loaded 93827 words'\
		| grep -Eo '[0-9]+\.[0-9]+' > ref_data.csv

plot_pt: $(BENCH_FILE)
	gnuplot scripts/runtimept.gp
	eog runtime2.png
clean:
	$(RM) $(TESTS) $(OBJS)
	$(RM) $(deps)
	$(RM) bench_cpy.txt bench_ref.txt ref.txt cpy.txt
	$(RM) *.csv
	$(RM) *.png

-include $(deps)

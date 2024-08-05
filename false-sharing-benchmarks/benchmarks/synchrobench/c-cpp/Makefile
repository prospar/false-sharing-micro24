.PHONY:	all

BENCHS = src/trees/sftree src/linkedlists/lockfree-list src/hashtables/lockfree-ht src/trees/rbtree src/skiplists/sequential src/trees/sftree-man
LBENCHS = src/trees/tree-lock src/linkedlists/lock-coupling-list src/linkedlists/lazy-list src/hashtables/lockbased-ht src/skiplists/skiplist-lock src/linkedlists/lazy-list-man src/linkedlists/lazy-list-pad src/linkedlists/lazy-list-ts-sep src/hashtables/lockbased-ht-man src/hashtables/lockbased-ht-ts-sep
LFBENCHS = src/trees/lfbstree src/linkedlists/lockfree-list src/hashtables/lockfree-ht src/skiplists/rotating src/skiplists/fraser src/skiplists/nohotspot src/skiplists/numask

# include the m5 library for parallel region stats
# https://stackoverflow.com/questions/2826029/passing-additional-variables-from-command-line-to-make
# https://stackoverflow.com/questions/67096413/defining-local-variable-in-makefile-target
#CFLAGS += -I$(GEM5_PATH)/include
#LDFLAGS += -L$(GEM5_PATH)/util/m5/build/x86/out -lm5

# Only compile C11/GNU11 algorithms with compatible compiler
GCC_GTEQ_490 := $(shell expr `gcc -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \>= 40900)
ifeq "$(GCC_GTEQ_490)" "1"
	BENCHS +=
	LBENCHS += src/linkedlists/versioned
	LFBENCHS += src/linkedlists/selfish
endif

#MAKEFLAGS+=-j4

MALLOC=TC

.PHONY:	clean all $(BENCHS) $(LBENCHS)

all:	lock spinlock lockfree estm sequential

lock: clean-build
	$(MAKE) "LOCK=MUTEX" $(LBENCHS)

spinlock: clean-build
	$(MAKE) "LOCK=SPIN" $(LBENCHS)

sequential: clean-build
	$(MAKE) "STM=SEQUENTIAL" $(BENCHS)

lockfree: clean-build
	for dir in $(LFBENCHS); do \
	$(MAKE) "STM=LOCKFREE" -C $$dir; \
	done

estm: clean-build
	$(MAKE) -C src/utils/estm-0.3.0
	$(MAKE) "STM=ESTM" $(BENCHS)

clean-build:
	rm -rf build

clean:
	$(MAKE) -C src/utils/estm-0.3.0 clean
	rm -rf build bin

$(BENCHS):
	$(MAKE) -C $@ $(TARGET)

$(LBENCHS):
	$(MAKE) -C $@ $(TARGET)

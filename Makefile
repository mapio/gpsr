CFLAGS = -ansi -O2 -DNDEBUG -Wall -Wno-format-nonliteral

target1 = gpsr
target2 = cat
target3 = cli

targets = $(target1) $(target2) $(target3)

.PHONY: all clean distclean dataclean

sources = $(wildcard *.c)
objects = $(filter-out $(targets:=.o), $(sources:.c=.o))
objectsi = $(filter-out expr.o stats.o signal.o genetic.o, $(objects)) \
           expr_i.o stats_i.o signal_i.o genetic_i.o

all: $(targets)

gpsr: $(objectsi) $(target1).o
	$(CC) $(objectsi) $(target1).o -o $(target1) -lm -lcurses

cat: $(objects) $(target2).o
	$(CC) $(objects) $(target2).o -o $(target2) -lm

cli: $(objects) $(target3).o
	$(CC) $(objects) $(target3).o -o $(target3) -lm -lreadline -lcurses

clean:
	-rm *.o *.d *~ core TAGS

dataclean:
	-rm *.pop *.sts *.run

distclean: clean dataclean
	-rm $(targets)

# deps

%_i.o: %.c %.o
	$(CC) $(CFLAGS) -DINTERACTIVE -c $< -o $@

%.d: %.c
	$(SHELL) -ec '$(CC) -MM -MG $(CPPFLAGS) $< \
	| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@'

include $(sources:.c=.d)

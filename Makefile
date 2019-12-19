CC = gcc -std=c99
CCFLAGS = -Wall -O2 -g -I./include

dependency = \
disk \
pagetable \
phytable \
pra \
sim



EXEC = memory_manager

GIT_HOOKS := .git/hooks/applied

all: $(GIT_HOOKS)
	$(CC) -o $(EXEC) $(CCFLAGS) $(EXEC).c

$(GIT_HOOKS):
	scripts/install-git-hooks

clean:
	rm -rf $(EXEC) *.o
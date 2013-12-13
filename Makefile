# Type one of either 'make client' or 
# 'make server' to compile an executable. 
#
# Type 'make clean' to clear out the 
# executables.
#

client:
	gcc -std=c99 -o solitaire_c solitaire_c.c

server:
	gcc -std=c99 -o solitaire_s solitaire_s.c carddeck.c cardstack.c

clean:
	rm -f solitaire_s solitaire_c *~

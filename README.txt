MATH 442 SS Fall 2013

HOMEWORK 5: SOLITAIRE

v2.0 solitaire_c solitaire_s

A "freecell" solitaire remote player service.
---------------------------------------------

In this folder is a collection of C files that could serve as a 
starting point for a text-based networked multiple solitaire game.
There are two main C files, solitaire_c.c and solitaire_s.c, that
can be used to compile a rudimentary client and server executable
that can play a single-player solitaire hand over a network
connection.

The code is nearly the same as the single-executable version, found
in the v1.0 folder.  The difference is that the code is split into
two portions: (1) a front-end client that receives a solitaire player's
card plays and sends each command over a network socket to a listening
server and (2) the server listening server that deals out a game,
displays the status of play, and accepts commands send over a network
socket from a single client.

To test out the programs, bring up two terminal windows on the same
machine.  In one, type the commands

   make server
   ./solitaire_s 2000

The first command will build a 'solitaire_s' executable from a bunch of
C files (see the text file 'Makefile' for the command line that script
that builds the server).  The second line starts the server, listening
on a port numbered 2000.  Next, in another terminal, type the commands

   make client
   ./solitaire_c localhost 2000

These build and run the client executable 'solitaire_c'.  The second command
tells the client to connect with the server listening on port 2000 on the
same machine.  
  
This code is obviously incomplete.  A client should normally be able to 
see the status of play being managed by the server.  Furthermore, a server
should be able to manage the hands and play of more than one client, and
the center area shared by those clients should handle an 'arena' of more
than four suit piles.


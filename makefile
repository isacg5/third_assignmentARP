files: 
	gcc src/server.c -lncurses -lbmp -lm -o bin/server
	gcc src/client.c -lncurses -o bin/client
	gcc src/processA.c -lncurses -lbmp -lm -o bin/processA
	gcc src/processB.c -lncurses -lbmp -lm -o bin/processB
	gcc src/master.c -lbmp -lm -o bin/master

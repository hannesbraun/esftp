all: bin/esftp-server bin/esftp-client

bin/esftp-server: src/server.h src/serverConfig.h src/server.c src/lobby.c src/worker.c src/worker.h src/globalsServer.c src/workerList.c src/workerList.h src/fileSize.c src/fileSize.h src/printVersion.c src/printVersion.h src/commons.h
	mkdir -p bin
	gcc -Wall -Wextra -O3 -o bin/esftp-server -pthread src/server.c src/lobby.c src/worker.c src/globalsServer.c src/workerList.c src/fileSize.c src/printVersion.c

bin/esftp-client: src/client.c src/esftpClient.h src/esftpClient.c src/recvFileStatus.c src/recvFileStatus.h src/recvExact.c src/recvExact.h src/printVersion.c src/printVersion.h src/commons.h
	mkdir -p bin
	gcc -Wall -Wextra -O3 -o bin/esftp-client src/client.c src/esftpClient.c src/recvFileStatus.c src/recvExact.c src/printVersion.c -lm

clean:
	rm -rf build
	rm -rf bin

install:
	mkdir -p /usr/local/bin
	mkdir -p /usr/local/share/man/man1
	cp bin/esftp-server /usr/local/bin/esftp-server
	cp doc/esftp-server.1 /usr/local/share/man/man1/esftp-server.1
	cp bin/esftp-client /usr/local/bin/esftp-client
	cp doc/esftp-client.1 /usr/local/share/man/man1/esftp-client.1

uninstall:
	rm -f /usr/local/bin/esftp-server
	rm -f /usr/local/share/man/man1/esftp-server.1
	rm -f /usr/local/bin/esftp-client
	rm -f /usr/local/share/man/man1/esftp-client.1

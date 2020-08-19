all: bin/esftp-server bin/esftp-client

bin/esftp-server: src/server.h src/serverConfig.h src/workerList.h src/server.c src/lobby.c src/worker.c src/globalsServer.c src/workerList.c src/fileSize.c src/fileSize.h src/printVersion.c src/printVersion.h
	mkdir -p bin
	gcc -Wall -Wextra -o bin/esftp-server -pthread src/server.c src/lobby.c src/worker.c src/globalsServer.c src/workerList.c src/fileSize.c src/printVersion.c

bin/esftp-client: src/client.h src/client.c src/esftpClient.c src/recvFileStatus.c src/recvFileStatus.h src/recvExact.c src/recvExact.h src/printVersion.c src/printVersion.h
	mkdir -p bin
	gcc -Wall -Wextra -o bin/esftp-client -lm src/client.c src/esftpClient.c src/recvFileStatus.c src/recvExact.c src/printVersion.c

clean:
	rm -rf build
	rm -rf bin

install:
	cp bin/esftp-server /usr/local/bin
	cp bin/esftp-client /usr/local/client

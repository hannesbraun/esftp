all: bin/esftp-server bin/esftp-client

bin/esftp-server: src/server.h src/serverConfig.h src/workerList.h src/server.c src/lobby.c src/worker.c src/globals.c src/workerList.c build/fileSize.o build/printVersion.o
	mkdir -p bin
	gcc -Wall -Wextra -o bin/esftp-server -pthread src/server.c src/lobby.c src/worker.c src/globals.c src/workerList.c build/fileSize.o build/printVersion.o

bin/esftp-client: src/client.h src/client.c src/esftpClient.c build/recvExact.o build/printVersion.o
	mkdir -p bin
	gcc -Wall -Wextra -o bin/esftp-client src/client.c src/esftpClient.c build/recvExact.o build/printVersion.o

build/recvExact.o: src/recvExact.c src/recvExact.h
	mkdir -p build
	gcc -Wall -Wextra -c -o build/recvExact.o src/recvExact.c

build/fileSize.o: src/fileSize.c src/fileSize.h
	mkdir -p build
	gcc -Wall -Wextra -c -o build/fileSize.o src/fileSize.c

build/printVersion.o: src/printVersion.c src/printVersion.h
	mkdir -p build
	gcc -Wall -Wextra -c -o build/printVersion.o src/printVersion.c

clean:
	rm -rf build
	rm -rf bin

install:
	cp bin/esftp-server /usr/local/bin
	cp bin/esftp-client /usr/local/client

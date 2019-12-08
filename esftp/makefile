all: bin/esftp-server bin/esftp-client

bin/esftp-server: server/server.h server/serverConfig.h server/workerList.h server/main.c server/lobby.c server/worker.c server/globals.c server/workerList.c build/fileSize.o build/printVersion.o
	mkdir -p bin
	gcc -Wall -o bin/esftp-server -pthread server/main.c server/lobby.c server/worker.c server/globals.c server/workerList.c build/fileSize.o build/printVersion.o

bin/esftp-client: client/client.h client/main.c client/esftpClient.c build/recvExact.o build/printVersion.o
	mkdir -p bin
	gcc -Wall -o bin/esftp-client client/main.c client/esftpClient.c build/recvExact.o build/printVersion.o

build/recvExact.o: util/recvExact.c util/recvExact.h
	mkdir -p build
	gcc -Wall -c -o build/recvExact.o util/recvExact.c

build/fileSize.o: util/fileSize.c util/fileSize.h
	mkdir -p build
	gcc -Wall -c -o build/fileSize.o util/fileSize.c

build/printVersion.o: util/printVersion.c util/printVersion.h
	mkdir -p build
	gcc -Wall -c -o build/printVersion.o util/printVersion.c

clean:
	rm -rf build
	rm -rf bin

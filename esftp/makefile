all: bin/esftp-server bin/esftp-client

bin/esftp-server: server.h serverConfig.h workerList.h mainServer.c lobby.c worker.c globals.c workerList.c build/fileSize.o build/printVersion.o
	mkdir -p bin
	gcc -Wall -o bin/esftp-server -pthread mainServer.c lobby.c worker.c globals.c workerList.c build/fileSize.o build/printVersion.o

bin/esftp-client: client.h mainClient.c esftpClient.c build/recvExact.o build/printVersion.o
	mkdir -p bin
	gcc -Wall -o bin/esftp-client mainClient.c esftpClient.c build/recvExact.o build/printVersion.o

build/recvExact.o: recvExact.c recvExact.h
	mkdir -p build
	gcc -Wall -c -o build/recvExact.o recvExact.c

build/fileSize.o: fileSize.c fileSize.h
	mkdir -p build
	gcc -Wall -c -o build/fileSize.o fileSize.c

build/printVersion.o: printVersion.c printVersion.h
	mkdir -p build
	gcc -Wall -c -o build/printVersion.o printVersion.c

clean:
	rm -rf build
	rm -rf bin

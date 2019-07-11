all: esftp-server esftp-client

esftp-server: server/main.c server/server.h server/serverConfig.h server/lobby.c server/worker.c util/fileSize.c util/fileSize.h
	gcc -Wall -o esftp-server -pthread server/*.c util/fileSize.c

esftp-client: client/main.c client/esftpClient.c client/client.h
	gcc -Wall -o esftp-client client/*.c util/recvExact.c

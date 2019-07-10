all: esftp_server esftp_client

esftp_server: server/main.c server/server.h server/serverConfig.h server/lobby.c server/worker.c util/fileSize.c util/fileSize.h
	gcc -Wall -o esftp_server -pthread server/*.c util/fileSize.c

esftp_client: client/main.c client/esftpClient.c client/client.h
	gcc -Wall -o esftp_client client/*.c util/recvExact.c

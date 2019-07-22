all: esftp-server esftp-client

esftp-server: server/main.c server/server.h server/serverConfig.h server/lobby.c server/worker.c util/fileSize.c util/fileSize.h util/printVersion.h util/printVersion.c
	gcc -Wall -o esftp-server -pthread server/*.c util/fileSize.c util/printVersion.c

esftp-client: client/main.c client/esftpClient.c client/client.h util/recvExact.h util/recvExact.c util/printVersion.h util/printVersion.c
	gcc -Wall -o esftp-client client/*.c util/recvExact.c util/printVersion.c

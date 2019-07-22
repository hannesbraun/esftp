# esftp (in development)

"esftp" (even simpler file transfer protocal) is a (hopefully) simple tool implementing a simple protocol to transfer a file over the network. You can start an `esftp-server` serving a file and multiple instances of `esftp-client` can receive the file.

## Building from source

To build the client and the server:
```sh
make
```

To build the server:
```sh
make esftp-server
```

To build the client:
```sh
make esftp-client
```

## Limitations
You can only send files up to a size of 9223372036854775807 bytes (about 9 exabytes).

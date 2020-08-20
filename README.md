# ESFTP

"ESFTP" (even simpler file transfer protocol) is a (hopefully) simple protocol to transfer one or multiple files and/or directories over the network.
This repository contains the [definition of the protocol itself](doc/protocol.md) as well as an implementation of this protocol.

The basic idea is that you can start an `esftp-server` serving some files and directories. Multiple instances of `esftp-client` can then receive the provided content.

**Important note:** neither the protocol nor the implementation includes any kind of encryption. Keep this in mind, everything transferred over the network will be transferred in plaintext.

## Building from source

*A POSIX-compliant system is required to build and run this software.*

Compile both the server and the client with:
```sh
make
```

Then, install the binaries and the manpages:
```sh
sudo make install
```

You can uninstall the software again with:
```sh
sudo make uninstall
```

## Basic usage

Start an ESFTP server for example with something like:
```sh
esftp-server /path/to/file.png /path/to/directory
```

To receive these items, launch an ESFTP client like this:
```sh
esftp-client 192.168.0.101
```

The items will then be placed into your current working directory.

If want to shut down the server, press `Ctrl-C` to initiate a friendly shutdown. From now on, new transfers won't be started. All existing transfers will be completed and the server will terminate afterwards. If you need to shutdown the server immediately, press `Ctrl-C` again.

## Limitations

- You can only send files up to a size of 9223372036854775807 bytes (about 9 exabytes). This is a general limitation of the protocol.
- Only ip addresses can be specified for the server in the client. Domain names and hostnames are not supported.
- Per default, a maximum of 256 top-level items can be served with the server. This value can be changed: see `MAX_ITEMS` in [src/serverConfig.h](src/serverConfig.h).

Note: a few more parameters for the server can be changed (if required) in [src/serverConfig.h](src/serverConfig.h).

## License

This ESFTP implementation is released under the GNU General Public License, version 3. For more information see [LICENSE](LICENSE "GNU General Public License, Version 3").

## Author

Hannes Braun (<hannesbraun@mail.de>)

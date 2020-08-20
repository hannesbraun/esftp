# ESFTP - The protocol

*Last change: August 19th, 2020*

ESFTP is a protocol for sending one or multiple files and/or directories over a network. ESFTP is an abbreviation for "Even simpler file transfer protocol".
The goal of this protocol is to be as simple (and efficient) as possible. It is designed for use in a kind of "home network". That's why no encryption or other security related elements are included in the protocol.

## Introduction
When one or multiple files shall be transferred, the sender starts the ESFTP server. Then a client connects to this server and receives the content of the ESFTP server, he connected to. The client doesn't have to send any information to the server.

## Protocol specification
For the connection between server and client, TCP/IP is used. The server listens on port 6719 for new clients. As soon as a client connects to the server, the server will immediately start the transfer. The client doesn't have to send a single byte.

If an integer is sent with ESFTP, little-endianness is used.

At first, a header byte will be sent. The first bit in this byte indicates the compliance with this first version of the protocol. If the bit is set, the data sent by the server is compliant to this first version of the protocol. Else, the client should terminate the connection because he won't be able to correctly receive the data.
The other bits in the header byte are unused for now and should not be set as well as ignored by the client.

After this, a sequence of so-called items will follow. An item can be a file or a directory. An item has the following structure:
- An item header byte is sent at the beginning of an item.
   - The first bit is the item type. If the bit is not set, the item is a file, else it's a directory.
   - The second bit indicates if it is the last item in this directory. This case is described further below.
   - The third bit is used for directories. If the directory is empty, the bit is set. This is also described further below.
   - The other five bits are used to encode the length of the filename or directory name. These five bits shall be interpreted as an unsigned integer. Add 1, then multiply with 128 to get the amount of bytes that will follow for the filename or directory name. The actual name might not use all these bytes, so the terminating null-byte may occur before the last byte.
- Next, the filename or directory name is sent as a null-terminated string of the length specified by the header. To avoid an unexpected behavior, characters should be ASCII-encoded.
- If the item is a file, the following two parts will also be sent. Else, this part is left out.
   - The file length (number of bytes) is sent as an 8 byte unsigned integer.
   - The file (the actual data) is sent next. The length of this block is the file length received before.

When the sequence of items is finished, the connection will be closed and the transfer is complete.

### Transferring directory structures
To transfer the directory structures, the following logic will be used: the first item received will be placed in the target directory specified by the client. This is the "most outer directory". If an item is a directory, the next item (if existing) will be placed inside this directory, else it will be placed in the same directory as the current item. To return to the parent directory, the second bit in the item header byte is used. If the bit is set, this item is the last one in this directory. The next item will be placed inside the parent directory again. If the current directory is already the most outer directory, this is the last item to send and the sequence of items is finished.
In the special case, that an empty directory is sent, the third bit in its item header byte is set. This overrides the behavior of placing the next item inside the just created directory (this item).

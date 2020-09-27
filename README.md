# TeaSip

## Build

To build TeaSip, use:

```shell
cmake -D CMAKE_CXX_FLAGS='-DBUFFER_SIZE=8 -DIPADDR=\"127.0.0.1\"' .
make
```

Note: You can set buffer size to anything other than `8` (this is just to test communication consistency).

## Server

To run server, use:

```shell
./server/tcp-server <port>
```

## Client

### Run client

To run client, use:

```shell
./client/tcp-client
```

### Client commands

 * `CONNECT <port> <name>`
 * `DISCONNECT`
 * `QUIT` (or `EXIT`)


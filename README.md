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
 * `SUBSCRIBE <topic>`
 * `UNSUBSCRIBE <topic>˙
 * `PUBLISH <topic> <data>`
 * `QUIT` (or `EXIT`)

## Known issues

 * `std::cin` and `std::cout` are in a race, and it would be best to implement a good interface, with bottom line being used for input, and the rest of the screen for server output (but it would require hacking the terminal around); investigate replacing `getLine()` with pure ˙std::cin˙ with a lock for now
 * some exceptions are not handled properly, sometimes `accept()` will fail due to `EINVAL` causing the client to abort; for now, add a try and catch logic on connect
 * when client disconnects, he is not automatically unsubscribed; it would be best to keep a separate map of file descriptors to subscribed topics; so that when we have a disconnect from a client, he gets unsubscribed automatically

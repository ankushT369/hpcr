# hpcr
## Project Overview

hpcr is a high performance chat room server implemented in C++ using Boost.Asio for asynchronous network communication and threading for handling concurrent client connections. It allows multiple clients to connect to a server and exchange messages in a chat room.

## Key Features

*   **Asynchronous Communication:** Uses Boost.Asio for non-blocking I/O, allowing the server to handle multiple clients concurrently.

## Architecture

The project consists of the following key components:

## Critical Functionalities

### Async I/O

The application uses Boost.Asio for asynchronous I/O, which allows the server to handle multiple client connections without blocking. The `async_read` and `async_write` methods in the `Session` class are used to asynchronously read data from and write data to the client socket.

### Threading

The application uses threading to handle concurrent client connections. Each client session is run in a separate thread, allowing the server to handle multiple clients simultaneously.


## LLD
Comming Soon

## Build and Run

To build and run the project, follow these steps:

1.  Clone the repository.
2.  Run `cd build`.
3.  Run `make` to build the project.
4.  Run `./chatApp <port>` to start the server, replacing `<port>` with the port number you want to use.
5.  Run `./hpcr`.

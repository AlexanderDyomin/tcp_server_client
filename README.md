# Description
This repository contains code for server and client communicating via custom protocol based on TCP. Server stores key-value pairs that can be get or modified by client.
## Protocol description
Protocol supports two commands: `get` and `set`.
Syntax:
`$get <key>` - returns current valuey of `<key>`.

`$set key=value` - set value of `<key>` on server to `<value>`.

Delimiter between different commands is symbols `\n\r\n\r`.

## Server
To build a server one needs to use CMake target `server`.

Server depends on Boost::Asio and Boost::System (as dependency of Boost::Asio).

Server is multi-threaded, supports multiple connections. Modified key-values map saves periodically on timeout, if map was modified. All operations on server are async.

## Client
To build a server one needs to use CMake target `client`.

Server depends on Boost::Asio and Boost::System (as dependency of Boost::Asio).

Client is single-threaded, sync test program, that randomly generates `get` or `set` command and sends it on server.
# Telnet Example

This is an example using telnet to access MyCLI.  It opens a socket and waits for
a new connection.  When a new telnet session comes in it places the prompt on that
socket.

## Files
| File          | Description                                                  |
| ------------- | ------------------------------------------------------------ |
| CLI_Options.h | The options for this example                                 |
| CLI_Sockets.c | The code to handle all the socket (open, read, write, close) |
| main.c        | The file with supported commands in it and main()            |



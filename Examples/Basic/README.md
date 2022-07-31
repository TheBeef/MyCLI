# Basic Example

This is an example using just the most basic IO and commands.  It shows
how to use the commands and help.

## Code Layout

The important parts of the code are as follows:

| g_CLICmds    | This is the list of commands the user can type |
| SetupIO()    | This function changes the term into raw mode   |
| ShutDownIO() | This function restores the term                |

# Compiling
The example compiles under Linux using the build.sh script.  Linux is needed
because the example uses term IO to set stdio to RAW mode.

# MyCLI
An embedded command line interface (CLI) that you can embed into your C/C++ code

## Quick start
This is a quick start guide to getting MyCLI up and running fast.

This will assume you have your own `main.c` in a directory that we will work off.

The quick start will be assume you have the following directory structure:
```
QuickStart/main.c
QuickStart/MyCLI/src/CLI.h
QuickStart/MyCLI/src/Example_CLI_Options.h
QuickStart/MyCLI/src/Full/CLI.c
```

### Step 1
Copy QuickStart/MyCLI/src/Example_CLI_Options.h to QuickStart/CLI_Options.h

### Step 2
Add the following functions to your main.c (and fill them in with working code):

| Function                                  | Description                                                 |
| ----------------------------------------- | ----------------------------------------------------------- |
| bool HAL_CLI_IsCharAvailable(void)        | Returns true when HAL_CLI_GetChar() will return a char      |
| char HAL_CLI_GetChar(void)                | Gets the next char from your input stream                   |
| void HAL_CLI_PutChar(char c)              | Sends a char out your output stream                         |
| uint32_t HAL_CLI_GetMilliSecCounter(void) | Returns the number of milliseconds.  This can just return 0 |

So for example you can set these functions up to talk on your UART.

### Step 3
Add a structure for your commands to your main.c file.

```
const struct CLICommand g_CLICmds[]=
{
    {"MyCommand","My one line help string",MyCommandFn},
};
```

### Step 4
Add your command functions (don't forget to add a prototype above `g_CLICmds`).

```
void MyCommandFn(int argc,const char **argv)
{
    printf("Hello MyCommand!\n");
}
```

### Step 5
Init the prompt in main()
```
    struct CLIHandle *Prompt;
    char LineBuff[100];
    char HistoryBuff[100];

    Prompt=CLI_GetHandle();
    CLI_InitPrompt(Prompt);
    CLI_SetLineBuffer(Prompt,LineBuff,sizeof(LineBuff));
    CLI_SetHistoryBuffer(Prompt,HistoryBuff,sizeof(HistoryBuff));
```

### Step 6
Add `CLI_RunCmdPrompt()` to your main while loop.
```
    CLI_DrawPrompt(Prompt);
    while(1)
        CLI_RunCmdPrompt(Prompt);
```

### Step 7
Compile.
`gcc -I MyCLI/src -I . MyCLI/src/Full/CLI.c main.c`

## See also
Check out `Examples/Basic` for a Linux version that talks on stdio.


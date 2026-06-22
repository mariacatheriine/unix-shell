# ush — Unix Shell
 
A Unix shell built in C, written from scratch as a learning project. Supports command execution, built-in commands, pipes, and I/O redirection.
 
## Building
 
```bash
gcc -o ush ush.c
```
 
## Running
 
```bash
./ush
```
 
You'll see a `>` prompt. Type commands and press Enter.
 
## Features
 
### Command Execution
Runs any program available on your system's PATH, with arguments.
```
> ls -la
> echo hello world
```
 
### Built-in Commands
 
| Command | Description |
|--------|-------------|
| `cd <dir>` | Change the current directory |
| `help` | List available built-in commands |
| `exit` | Exit the shell |
 
### Pipes
Chain two commands together so the output of the first becomes the input of the second.
```
> ls | grep txt
> cat names.txt | sort
```
 
### I/O Redirection
 
| Operator | Description |
|----------|-------------|
| `>` | Redirect stdout to a file, overwriting it |
| `>>` | Redirect stdout to a file, appending to it |
| `<` | Read stdin from a file |
 
```
> ls > out.txt
> echo hello >> log.txt
> sort < names.txt
```
 
## Implementation Notes

- Uses fork() and execvp() for command execution
- Pipes implemented using pipe(), dup2(), and two child processes
- I/O redirection implemented using open() and dup2() inside the child process before execvp()
- Dynamic memory allocation for input buffer and token array with automatic resizing
 
## References
 
- Stephen Brennan's Tutorial - Write a Shell in C
- Indradhanush Gupta — Writing a Unix Shell
 

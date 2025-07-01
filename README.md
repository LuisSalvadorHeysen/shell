# MyShell - A Custom Unix Shell Implementation

A lightweight Unix shell implementation written in C that supports basic shell functionality including built-in commands, command execution, input/output redirection, and batch processing.

## Features

### Built-in Commands
- **`cd [directory]`** - Change directory (defaults to HOME if no directory specified)
- **`pwd`** - Print working directory
- **`exit`** - Exit the shell

### Command Execution
- Execute external commands using `execvp()`
- Support for command arguments
- Proper process management with `fork()` and `waitpid()`

### Input/Output Redirection
- **Standard redirection (`>`)** - Overwrite output to file
- **Advanced redirection (`>+`)** - Append output to file
- Error handling for invalid redirections
- Support for spaces around redirection operators

### Multiple Commands
- Execute multiple commands on a single line using semicolon (`;`) separator
- Each command is executed sequentially

### Batch Processing
- Run shell commands from a file using `./myshell <filename>`
- Support for large command files (up to 1MB)
- Proper error handling for non-existent files

### Error Handling
- Comprehensive error checking for invalid commands
- Graceful handling of non-existent files and directories
- Input validation for command length and format

## Building and Running

### Prerequisites
- GCC compiler
- Unix-like operating system (Linux, macOS, etc.)

### Compilation
```bash
make
```

### Usage

#### Interactive Mode
```bash
./myshell
```

#### Batch Mode
```bash
./myshell <batch_file>
```

### Example Usage

```bash
# Interactive shell
$ ./myshell
myshell> ls -la
myshell> cd /tmp
myshell> pwd
/tmp
myshell> exit

# Batch processing
$ ./myshell batch-files/gooduser_basic

# Redirection examples
myshell> ls > output.txt
myshell> echo "Hello" >+ append.txt
myshell> cat file1 file2 > combined.txt

# Multiple commands
myshell> ls; pwd; echo "Done"
```

## Project Structure

```
p3shell/
├── myshell.c          # Main shell implementation
├── Makefile           # Build configuration
├── batch-files/       # Test batch files
├── expected-output/   # Expected test outputs
├── test-scripts/      # Testing utilities
└── README.md          # This file
```

## Implementation Details

### Key Components

1. **Input Processing**
   - `read_and_validate_input()` - Handles input reading and validation
   - `get_tokens()` - Tokenizes command strings
   - Support for both interactive and batch modes

2. **Command Parsing**
   - Built-in command detection (`is_cd()`, `is_pwd()`, `is_exit()`)
   - Redirection parsing (`pack_command_data()`)
   - Multiple command parsing with semicolon separation

3. **Execution Engine**
   - `run_one_command()` - Executes single commands
   - `run_with_redirect()` - Handles I/O redirection
   - `run_multiple()` - Processes multiple commands

4. **Process Management**
   - Proper fork/exec pattern for external commands
   - Signal handling and process cleanup
   - File descriptor management for redirections

### Memory Management
- Dynamic memory allocation for command tokens
- Proper cleanup of allocated resources
- Error handling for memory allocation failures

### Error Handling
- Consistent error messages using `error()` function
- Input validation for command length and format
- File operation error checking

## Testing

The project includes comprehensive test suites:

```bash
cd test-scripts
python2 grade.py
```

Tests cover:
- Basic command execution
- Built-in commands
- Redirection functionality
- Error conditions
- Batch processing
- Edge cases

## Limitations

- Maximum input size: 512 characters
- Maximum arguments: 100 per command
- No support for pipes (`|`)
- No support for background processes (`&`)
- No support for environment variables beyond basic usage

## License

This project was created as part of CS144 coursework at the University of Chicago.

## Author

Created as a learning project to understand Unix shell internals and system programming concepts. 
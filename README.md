# Stack Machine Emulator (C Project)

## Overview
This project implements an emulator for a fictional stack-based machine. The emulator performs two main tasks:

1. **Assembler**: Converts a program written in assembly language into machine code.
2. **Simulator**: Executes the generated machine code instruction by instruction.

The project is designed to simulate the internal workings of a simplified stack machine, including memory management, instruction decoding, and execution.

## Features
- Translates assembly language into machine code with syntax error detection.
- Simulates the stack machine's execution of instructions.
- Supports a variety of operations, including arithmetic, logical, and memory manipulation.
- Handles procedure calls, jumps, and input/output operations.

## Project Structure
The project includes the following key files and folders:
- `src/`: Source code files for the assembler and simulator.
- `include/`: Header files defining the machine's structures and functions.
- `docs/`:
  - `user.pdf`: User documentation for running and using the project.
  - `dev.pdf`: Developer documentation detailing implementation choices and challenges.
- `README.md`: This file, providing an overview of the project.
- `Makefile`: For building the project (optional).

## Compilation and Execution
### Prerequisites
- GCC compiler (tested with version 9.4+).
- Unix or Unix-like environment (required for testing compatibility).

### Compilation
Use the following command to compile the project:
```bash
gcc -Wall -g -o simulateur src/*.c
```

### Running the Program
To assemble and execute a program, run:
```bash
./simulateur [source_file]
```
- Replace `[source_file]` with the path to an assembly language file (e.g., `pgm.txt`).
- The program generates a machine code file `hexa.txt` and executes it.

## Instruction Set
The emulator supports the following instructions:

- **Memory Operations**:
  - `pop x`: Stores the top of the stack at address `x`.
  - `push x`: Pushes the value at address `x` onto the stack.
  - `push# i`: Pushes the constant value `i` onto the stack.
- **Control Flow**:
  - `jmp adr`: Jumps to the instruction at the relative address `adr`.
  - `jnz adr`: Jumps to `adr` if the stack's top value is non-zero.
  - `call adr`: Calls a procedure at `adr`.
  - `ret`: Returns from the current procedure.
- **Input/Output**:
  - `read x`: Reads a value into memory at address `x`.
  - `write x`: Writes the value at address `x` to the console.
- **Arithmetic/Logic**:
  - `op i`: Performs an operation specified by `i` (e.g., addition, multiplication, bitwise logic).

Refer to `docs/user.pdf` for the full instruction set and examples.

## Example
### Input Assembly File (`pgm.txt`)
```assembly
start: read 1000
push 1000
push# 0
op 0
jnz end
push 1000
op 15
pop 1000
write 1000
jmp start
end: halt
```

### Generated Machine Code (`hexa.txt`)
```plaintext
09 03e8
02 03e8
04 0000
0b 0000
06 0005
02 03e8
0b 000f
00 03e8
0a 03e8
05 fff6
63 0000
```

## Contributions and License
This project was developed as part of the L2 MIDO Architecture course at Dauphine University (2024-2025). All submissions must comply with academic integrity policies, and unauthorized sharing of code is prohibited.

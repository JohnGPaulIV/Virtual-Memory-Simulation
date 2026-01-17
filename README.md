# Virtual Memory Simulator

**Course:** CS 261 – Computer Systems I  
**Role:** Software Engineer (Student)

## Overview

This project is a comprehensive simulation of **virtual memory 
management** and **program execution**, modeled after the **Von Neumann 
architecture’s Fetch–Decode–Execute cycle**.

The simulator emulates how an operating system loads programs into memory, 
interprets executable file formats, and executes instructions within a 
virtualized memory space.

The program operates on a **custom Mini-ELF executable format**, maps 
program segments into virtual memory, disassembles binary instructions 
into **y86 assembly**, and executes them within a simulated memory 
environment.

---

## Project Breakdown

### Mini-ELF Parsing
- Reads and validates a custom Mini-ELF header format
- Extracts metadata required for memory mapping
- Establishes the foundation for virtual memory allocation

### Virtual Memory Mapping
- Interprets program headers
- Loads program segments into virtual memory
- Simulates realistic process memory layouts

### Disassembly
- Converts binary instructions into readable **y86 assembly**
- Supports disassembly of both code and data segments
- Bridges machine-level data with human-readable instructions

### Program Execution
- Executes y86 instructions using the fetch–decode–execute cycle
- Operates entirely within simulated virtual memory
- Includes optional trace mode for step-by-step execution

---

## File Structure
```bash
.
├── inputs/
│ └── *.o # Test Mini-ELF object files
├── src/
│ ├── *.c
│ └── *.h
├── Makefile
└── README.md
```

### Inputs Folder

The `inputs/` directory contains **precompiled `.o` files** used as test 
programs for the simulator.

Because this project uses a **custom Mini-ELF header format**, standard 
system executables are not compatible.  
These input files are specifically designed to work with the simulator and 
support:

- Header inspection
- Memory mapping
- Disassembly
- Program execution
- Trace mode execution

---

## Building the Program

Before running the simulator, compile the program using:

```bash
make
```
while in the parent directory.

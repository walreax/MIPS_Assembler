# CLAIMS

## Submission Overview

This document outlines the claims regarding the submission for the CS2102 Project 2024, which involves developing a two-pass assembler and an emulator for an extended SIMPLE instruction set. The project includes the implementation of an assembler that processes assembly language, generates object files, and produces listing files, as well as an emulator capable of executing the generated machine code.

## Claims

### General Submission Claims

- **Correct File Structure**: All source files are correctly named and include my name and roll number at the top, along with a declaration of authorship.
- **Compilation**: The code compiles without errors using `g++ -std=c++11 -pedantic`
- **Code Quality**: The code is consistently formatted with sensible program structure. Variable, function, and type names are descriptive, and appropriate comments are included throughout.

### Assembler Claims (65%) - [asm.cpp](asm.cpp)

- **Error Diagnostics**: The assembler successfully diagnoses common assembly errors, including duplicate labels and missing labels.
- **Listing File Generation**: A listing file is produced that includes a memory dump format.
- **Advanced Listing Features**: The listing file also provides human-readable mnemonics and operands corresponding to each instruction.
- **Instruction Table Implementation**: An internal table of instruction names and expected operands is implemented to facilitate easy reference during assembly.
- **Test Program Assembly**: The assembler successfully assembles provided test programs without errors.
- **Additional Test Program**: Additional test programs have been created to demonstrate both successful assembly and error detection, with explanations included as comments in each test file. [MEAN OF NUMBERS IN ARRAY](mean.asm)
- **SET Instruction Implementation**: The optional SET instruction has been implemented and demonstrated within the test programs.
- **Bubble Sort Program**: A bubble sort program written in SIMPLE assembly has been completed and successfully assembled. [BUBBLESORT](bubblesort.asm)

### Emulator Claims (25%) - [emu.cpp](emu.cpp)

- **Object File Loading**: The emulator is capable of loading object files generated by the assembler.
- **Error Detection Capabilities**: It detects errant programs and handles execution errors appropriately.
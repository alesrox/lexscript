# My own Programming Language
Lexscript: A new high-level and strongly-typed language on development.

## Compiler
### Compiler Usage
The compiler, written in Python, takes a .lx file and generates the corresponding bytecode for the virtual machine.
```bash
python compiler/main.py <lex file> <output filename>
```
Example
```bash
python compiler/main.py test/test3.lx output
```

### Compiler Flags
The compiler supports several flags for debugging and output customization:

* -l: Only print the output of the lexer stage.
* -p: Only print the output of the parser stage.
* -d: Export a human-readable version of the bytecode to output.txt.
* -b: Export the raw bytecode to output.txt for direct use with the virtual machine.

# Virtual Machine
### Compile the Virtual Machine
The Virtual Machine (./vml) is compiled for MacOS systems with ARM chips, but you can compile it for your operating system using the makefile:
```bash
make
```

### Run the Virtual Machine
After compiling, you can execute the virtual machine with a binary file as input:
<binary file>
```bash
./vml <binary file>
```
Example:
```bash
./vml output
```

## Next Step
- BigInt and BigFloat implementation.
- For each statement.
- Import statement.
- Dictionaries or json-like-objects (like Python dicts).

## Future features to add
- System control functions
- Multi-file scripts (like import statement from python or java, or #include from C/C++/C#)
- Trash collector for the Virtual Machine
- Something else in order to improve the language
